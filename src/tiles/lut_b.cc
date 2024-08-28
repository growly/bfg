#include "lut_b.h"

#include <map>
#include <set>
#include <unordered_map>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "../atoms/sky130_buf.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_mux.h"
#include "../atoms/sky130_tap.h"
#include "../checkerboard_guide.h"
#include "../equivalent_nets.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../geometry/port.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../routing_layer_info.h"
#include "../routing_via_info.h"
#include "../row_guide.h"

namespace bfg {
namespace tiles {

const std::pair<size_t, LutB::LayoutConfig> LutB::kLayoutConfigurations[] = {
  {4, LutB::LayoutConfig {
    .left = LutB::BankArrangement {
      .memory_rows = {0, 1, 2, 3, 4, 5, 6, 7},
      .buffer_rows = {7, 7, 7},
      .clk_buf_rows = {4},
      .horizontal_alignment = geometry::Compass::LEFT
    },
    .right = LutB::BankArrangement {
      .memory_rows = {7, 6, 5, 4, 3, 2, 1, 0},
      .buffer_rows = {0},
      .active_mux2_rows = {0},
      .clk_buf_rows = {3},
      .horizontal_alignment = geometry::Compass::RIGHT
    },
    .mux_area_padding = 2500,
    .mux_area_rows = 2,
    .mux_area_columns = 2,
  }},
  //{5, LayoutConfig {
  //    .num_banks = 2,
  //    .bank_rows = 8,
  //    .bank_columns = 2,
  //    .mux_area_rows = 4,
  //    .mux_area_columns = 2,
  //    .mux_area_padding = 2500,
  //    .rotate_first_row = true
  //}},
  //{6, LayoutConfig {
  //    .num_banks = 2,
  //    .bank_rows = 8,
  //    .bank_columns = 4,
  //    .mux_area_rows = 4,
  //    .mux_area_columns = 4,
  //    .mux_area_padding = 2500,
  //    .rotate_first_row = true
  //}}
};

const LutB::LayoutConfig *LutB::GetLayoutConfiguration(size_t lut_size) {
  size_t num_configurations =
      sizeof(kLayoutConfigurations) / sizeof(kLayoutConfigurations[0]);
  for (size_t i = 0; i < num_configurations; ++i ){
    if (kLayoutConfigurations[i].first == lut_size) {
      return &kLayoutConfigurations[i].second;
    }
  }
  LOG(FATAL) << "No layout configuration for LUT size: " << lut_size;
  return nullptr;
}

bfg::Cell *LutB::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Cell> lut_cell(new bfg::Cell("lut"));
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  memories_.clear();
  clk_buf_order_.clear();
  active_mux2s_.clear();
  mux_order_.clear();
  buf_order_.clear();
  banks_.clear();
  
  const LutB::LayoutConfig layout_config =
      *LutB::GetLayoutConfiguration(lut_size_); 
  constexpr int64_t kMuxSize = 8;

  int64_t buf_y_pos = 0;

  bfg::atoms::Sky130Tap::Parameters tap_params = {
    .height_nm = 2720,
    .width_nm = 460
  };
  bfg::atoms::Sky130Tap tap_generator(tap_params, design_db_);
  bfg::Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      "lut_tap_template");

  std::vector<geometry::Instance*> memories;

  std::vector<std::reference_wrapper<const BankArrangement>> arrangements = {
    layout_config.left, layout_config.right};

  for (size_t p = 0; p < arrangements.size(); ++p) {
    const BankArrangement &bank_arrangement = arrangements[p].get();
    banks_.push_back(MemoryBank(layout.get(),
                               design_db_,
                               tap_cell,
                               true,      // Rotate alternate rows.
                               true,      // Rotate the first row.
                               bank_arrangement.horizontal_alignment));
    MemoryBank &bank = banks_.back();

    // We now want to assign things to rows and have the memory bank create the
    // row if they don't exist.

    size_t num_memories = 0;
    for (size_t i = 0; i < bank_arrangement.memory_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.memory_rows[i];

      std::string instance_name = absl::StrFormat(
          "lut_dfxtp_%d_%d", p, num_memories);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      bfg::atoms::Sky130Dfxtp::Parameters params;
      bfg::atoms::Sky130Dfxtp generator(params, design_db_);
      bfg::Cell *cell = generator.GenerateIntoDatabase(cell_name);

      geometry::Instance *installed =
          bank.InstantiateRight(assigned_row, instance_name, cell->layout());

      memories.push_back(installed);
      ++num_memories;
    }

    //  layout->SavePoint(absl::StrCat("row_", j, "_lr"),
    //                    geometry::Point(row_width, y_pos));
    //}
  }

  LOG_IF(FATAL, banks_.size() < 1) << "Expected at least 1 bank by this point.";

  banks_[0].MoveTo(geometry::Point(0, 0));

  bfg::atoms::Sky130Mux::Parameters mux_params;
  mux_params.extend_inputs_top = true;
  mux_params.extend_inputs_bottom = false;

  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *base_mux_cell = mux.GenerateIntoDatabase("sky130_mux");

  // A second version of the mux has its inputs on the bottom instead of the
  // top:
  bfg::atoms::Sky130Mux::Parameters alt_mux_params;
  alt_mux_params.extend_inputs_top = false;
  alt_mux_params.extend_inputs_bottom = true;

  bfg::Cell *alt_mux_cell = bfg::atoms::Sky130Mux(
      alt_mux_params, design_db_).GenerateIntoDatabase("alt_sky130_mux");

  // Muxes are positioned like so:
  //
  // | 4-LUT | 5-LUT | 6-LUT
  //                 
  // |       |   x   |   x x
  // |       | x     | x     x
  // |   x   |   x   |   x x
  // | x     | x     | x     x
  //
  // The number of columns is defined in the LayoutConfig struct in
  // kLayoutConfigurations. Here we must compute the position based on where
  // they are in this chain.
  int64_t mux_height = base_mux_cell->layout()->GetBoundingBox().Height();
  int64_t mux_width = base_mux_cell->layout()->GetBoundingBox().Width();
  int64_t left_bank_bottom_row_right_x = banks_[0].Row(0).Width();
  int64_t x_pos = left_bank_bottom_row_right_x + layout_config.mux_area_padding;
  // This staggers the mux area below the memories on the left:
  //int64_t y_pos = -mux_height / 2;
  int64_t y_pos = memories.front()->Height() / 2;

  std::vector<bfg::Cell*> mux_templates = {base_mux_cell, alt_mux_cell};

  CheckerboardGuide mux_grid(geometry::Point(x_pos, y_pos),
                             "mux",
                             layout_config.mux_area_rows,
                             layout_config.mux_area_columns,
                             layout.get(),
                             nullptr,
                             design_db_);
  mux_grid.set_template_cells(&mux_templates);
  // FIXME(aryap): This is a function of track pitch, really, not some number I
  // eyeballed.
  mux_grid.set_horizontal_overlap(-300);
  mux_grid.set_vertical_overlap(-1500);
  const std::vector<geometry::Instance*> &mux_order = mux_grid.InstantiateAll();

  std::unordered_map<std::string, geometry::Instance *const>
      all_instances_by_name;
  layout->GetInstancesByName(&all_instances_by_name);

  for (size_t p = 0; p < arrangements.size(); ++p) {
    const BankArrangement &bank_arrangement = arrangements[p].get();
    MemoryBank &bank = banks_[p];

    for (size_t i = 0; i < bank_arrangement.buffer_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.buffer_rows[i];
      size_t buf_count = buf_order_.size();

      std::string instance_name = absl::StrFormat("buf_%d", buf_count);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130Buf::Parameters buf_params = {
        .width_nm = 1380,
        .height_nm = 2720,
        .nfet_0_width_nm = 520,
        .nfet_1_width_nm = 520,
        .pfet_0_width_nm = 790,
        .pfet_1_width_nm = 790
      };
      atoms::Sky130Buf buf_generator(buf_params, design_db_);
      bfg::Cell *buf_cell = buf_generator.GenerateIntoDatabase(cell_name);
      buf_cell->layout()->ResetY();
      geometry::Instance *installed = bank.InstantiateInside(
          assigned_row, instance_name, buf_cell->layout());
      buf_order_.push_back(installed);
    }

    for (size_t i = 0; i < bank_arrangement.clk_buf_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.clk_buf_rows[i];
      size_t buf_count = clk_buf_order_.size();

      std::string instance_name = absl::StrFormat("clk_buf_%d", buf_count);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130Buf::Parameters buf_params = {
        .width_nm = 1380,
        .height_nm = 2720,
        .nfet_0_width_nm = 520,
        .nfet_1_width_nm = 520,
        .pfet_0_width_nm = 790,
        .pfet_1_width_nm = 790
      };
      atoms::Sky130Buf buf_generator(buf_params, design_db_);
      bfg::Cell *buf_cell = buf_generator.GenerateIntoDatabase(cell_name);
      buf_cell->layout()->ResetY();
      geometry::Instance *installed = bank.InstantiateInside(
          assigned_row, instance_name, buf_cell->layout());
      clk_buf_order_.push_back(installed);
    }

    for (size_t i = 0; i < bank_arrangement.active_mux2_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.active_mux2_rows[i];

      std::string instance_name = absl::StrFormat("hd_mux2_1_%d", i);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130HdMux21 active_mux2_generator({}, design_db_);
      bfg::Cell *active_mux2_cell = active_mux2_generator.GenerateIntoDatabase(
          cell_name);
      active_mux2_cell->layout()->ResetY();
      geometry::Instance *instance = bank.InstantiateInside(
          assigned_row, instance_name, active_mux2_cell->layout());
      active_mux2s_.push_back(instance);
    }
  }

  // Now that all instances have been assigned to the banks and their
  // dimensions are known, move them into place around the muxes. Well, move
  // the right bank because the first bank is fixed.
  int64_t right_bank_top_row_left_x = banks_[1].rows().back().LowerLeft().x();
  int64_t right_bank_bottom_row_top_y =
      banks_[1].rows().front().UpperLeft().y();

  x_pos = mux_grid.GetBoundingBox()->upper_right().x() +
      layout_config.mux_area_padding;
  y_pos = mux_grid.GetBoundingBox()->lower_left().y() -
      layout_config.mux_area_padding;

  banks_[1].AlignPointTo(
      {right_bank_top_row_left_x, right_bank_bottom_row_top_y},
      {x_pos, y_pos});

  Route(layout.get());

  // //// FIXME(aryap): remove
  // ///DEBUG
  // //lut_cell->SetLayout(layout.release());
  // //lut_cell->SetCircuit(circuit.release());
  // //bfg::Cell *pre = lut_cell.release();
  // //pre->set_name(name);
  // //design_db_->ConsumeCell(pre);
  // //return pre;
  lut_cell->SetLayout(layout.release());
  lut_cell->SetCircuit(circuit.release());
  bfg::Cell *cell = lut_cell.release();
  cell->set_name(name);
  design_db_->ConsumeCell(cell);
  return cell;
}

void LutB::Route(Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  const LutB::LayoutConfig layout_config =
      *LutB::GetLayoutConfiguration(lut_size_); 

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();
  LOG(INFO) << "Pre-routing bounds: " << pre_route_bounds;

  RoutingGrid routing_grid(db);

  bfg::RoutingLayerInfo met1_layer_info =
      db.GetRoutingLayerInfoOrDie("met1.drawing");
  met1_layer_info.direction = bfg::RoutingTrackDirection::kTrackHorizontal;
  met1_layer_info.area = pre_route_bounds;
  // TODO(aryap): Need an easier way of lining this up!
  //met1_layer_info.offset = 95;

  bfg::RoutingLayerInfo met2_layer_info =
      db.GetRoutingLayerInfoOrDie("met2.drawing");
  met2_layer_info.direction = bfg::RoutingTrackDirection::kTrackVertical;
  met2_layer_info.area = pre_route_bounds;
  met2_layer_info.offset = 50;

  // TODO(aryap): Store connectivity information (which layers connect through
  // which vias) in the PhysicalPropertiesDatabase's via_layers_.
  bfg::RoutingViaInfo routing_via_info =
      db.GetRoutingViaInfoOrDie("met1.drawing", "met2.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer, met2_layer_info.layer, routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("li.drawing", "met1.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer, db.GetLayer("li.drawing"), routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("met2.drawing", "met3.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), met2_layer_info.layer, routing_via_info)
      .IgnoreError();

  //routing_grid.AddRoutingLayerInfo(li_layer_info);
  routing_grid.AddRoutingLayerInfo(met1_layer_info).IgnoreError();
  routing_grid.AddRoutingLayerInfo(met2_layer_info).IgnoreError();

  routing_grid.ConnectLayers(met1_layer_info.layer, met2_layer_info.layer)
      .IgnoreError();

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->CopyShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
    routing_grid.AddBlockages(shapes);
  }
  {
    geometry::ShapeCollection shapes;
    layout->CopyShapesOnLayer(db.GetLayer("met2.drawing"), &shapes);
    routing_grid.AddBlockages(shapes);
  }

  // Debug only.
  //routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout.get());

  std::unique_ptr<bfg::Layout> grid_layout;

  // // The scan chain is connected in the order memories are assigned by the
  // // BankArrangement.
  // std::map<geometry::Instance*, std::string> memory_output_net_names;
  // for (size_t i = 0; i < memories.size() - 1; ++i) {
  //   geometry::Instance *source = memories[i];
  //   geometry::Instance *sink = memories[i + 1];

  //   LOG(INFO) << "Adding scan routes for pair "
  //             << source->name() << ", " << sink->name();

  //   std::set<geometry::Port*> ports;
  //   source->GetInstancePorts("Q", &ports);
  //   geometry::Port *start = *ports.begin();
  //   ports.clear();

  //   sink->GetInstancePorts("D", &ports);
  //   geometry::Port *end = *ports.begin();

  //   EquivalentNets net_names = EquivalentNets({end->net(), start->net()});
  //   memory_output_net_names[source] = net_names.primary();

  //   geometry::ShapeCollection non_net_connectables;
  //   layout->CopyConnectableShapesNotOnNets(
  //       net_names,
  //       &non_net_connectables);
  //   routing_grid.AddRouteBetween(*start, *end, non_net_connectables, net_names)
  //               .IgnoreError();
  // }

  // TODO(aryap): I want to solve the general problem of connecting to a port
  // on an instance which is comprised of many, possibly connected, shapes on
  // many, possibly connected, layers. The tricky thing is that connecting on
  // one layer might create DRC violations on an adjacent layer (e.g. if you
  // connect on met2 but jump up from met1 just before, and there's a met1
  // shape near, you get a problem).
  //
  // A related and important consideration is that all shapes with the same
  // port name label should be considered connected, even if they are not port
  // objects. Or should they be port objects?
  //
  // Is it desirable in general to treat any shape given a net name as
  // belonging to, and connectable by, that net?
  //
  // And what namespaces do these net names occupy? Their parent instance?
  // Unless exported by being labelled a port with the same name?
  //
  // FIXME(aryap): Immediately:
  //  - add power straps
  //  - complete routing ffs

  AddClockAndPowerStraps(layout);

  // Connect the input buffers on the selector lines.
  // TODO(aryap): These feel like first-class members of the RoutingGrid API
  // soon. "RouteGroup"?
  struct PortKey {
    geometry::Instance *instance;
    std::string port_name;
  };
  struct PortKeyCollection {
    std::vector<PortKey> port_keys;
    std::optional<std::string> net_name;
  };
  std::vector<PortKeyCollection> auto_connections; // = {{
  //    .port_keys = {{buf_order_[0], "P"}, {mux_order_[0], "S0_B"},
  //                  {mux_order_[1], "S0_B"}},
  //  }, {
  //    .port_keys = {{buf_order_[0], "X"}, {mux_order_[0], "S0"},
  //                  {mux_order_[1], "S0"}},
  //  }, {
  //    .port_keys = {{buf_order_[1], "P"}, {mux_order_[0], "S1_B"},
  //                  {mux_order_[1], "S1_B"}},
  //  }, {
  //    .port_keys = {{buf_order_[1], "X"}, {mux_order_[0], "S1"},
  //                  {mux_order_[1], "S1"}},
  //  }, {
  //    .port_keys = {{buf_order_[2], "P"}, {mux_order_[0], "S2_B"},
  //                  {mux_order_[1], "S2_B"}},
  //  }, {
  //    .port_keys = {{buf_order_[2], "X"}, {mux_order_[0], "S2"},
  //                  {mux_order_[1], "S2"}},
  //  }, {
  //    .port_keys = {{buf_order_[3], "X"}, {active_mux2s_[0], "S"}},
  //  }, {
  //    .port_keys = {{mux_order_[0], "Z"}, {active_mux2s_[0], "A0"}},
  //  }, {
  //    .port_keys = {{mux_order_[1], "Z"}, {active_mux2s_[0], "A1"}},
  //  }, {
  //    .port_keys = {{active_mux2s_[0], "X"}, {buf_order_[3], "A"}},
  //  }
  //};

  // // Add automatic connections for memory clock and inverted clock inputs.
  // for (size_t bank = 0; bank < banks_.size(); ++bank) {
  //   PortKeyCollection clks;
  //   clks.net_name = absl::StrCat("clk_", bank);
  //   std::vector<PortKey> &clk_connections = clks.port_keys;

  //   PortKeyCollection inverted_clks;
  //   inverted_clks.net_name = absl::StrCat("clk_i_", bank);
  //   std::vector<PortKey> &clk_i_connections = inverted_clks.port_keys;

  //   for (auto &row : banks_[bank].instances()) {
  //     for (geometry::Instance *instance : row) {
  //       if (instance->HasPort("CLK")) {
  //         clk_connections.push_back({
  //             .instance = instance,
  //             .port_name = "CLK",
  //         });
  //       }
  //       if (instance->HasPort("CLKI")) {
  //         clk_i_connections.push_back({
  //             .instance = instance,
  //             .port_name = "CLKI",
  //         });
  //       }
  //     }
  //   }
  //   clk_connections.push_back({
  //       .instance = clk_buf_order[bank],
  //       .port_name = "X",
  //   });

  //   clk_i_connections.push_back({
  //       .instance = clk_buf_order[bank],
  //       .port_name = "P",
  //   });

  //   auto_connections.push_back(clks);
  //   auto_connections.push_back(inverted_clks);
  // }

  for (const PortKeyCollection &collection : auto_connections) {
    std::vector<std::vector<geometry::Port*>> route_targets;
    for (auto &port_key : collection.port_keys) {
      std::vector<geometry::Port*> &port_list =
          route_targets.emplace_back();
      geometry::Instance *instance = port_key.instance;

      std::set<geometry::Port*> matching_ports;
      instance->GetInstancePorts(port_key.port_name, &matching_ports);
      if (matching_ports.empty()) {
        LOG(WARNING) << "No port found named \"" << port_key.port_name
                     << "\" on instance \"" << instance->name();
        continue;
      }
      port_list.insert(
          port_list.end(), matching_ports.begin(), matching_ports.end());
    }

    std::string net = collection.net_name ? *collection.net_name : "default";

    LOG(INFO) << "Connecting (net: " << net << ") all of: " << absl::StrJoin(
        collection.port_keys, ", ",
        [](std::string *out, const PortKey &port_key) {
          absl::StrAppend(
              out, port_key.instance->name(), "/", port_key.port_name);
        });

    auto result = routing_grid.AddMultiPointRoute(*layout,
                                                  route_targets,
                                                  collection.net_name);
  }


  // Connect memory outputs to the muxes in order:
  // - Connect the closest output with

  // Connect flip-flops to mux.
  struct AutoMemoryMuxConnection {
    geometry::Instance *source_memory;
    geometry::Instance *target_mux;
    std::string mux_port_name;
  };


  // // TODO(aryap): We know that the mux connections roughly map to the nearest
  // // flip flops in groups of 4; we should automate finding the order within
  // // those groups that yield best routes.

  // // The mux input order is, from top to bottom:
  // // input_5  --+---------
  // // input_4  --|
  // // input_6  --|
  // // input_7  --|  8:1 mux
  // // input_3  --|
  // // input_2  --|
  // // input_0  --|
  // // input_1  --+---------
  // std::vector<AutoMemoryMuxConnection> auto_mem_connections = {
  //   {banks_[0].instances()[7][0], mux_order_[1], "input_5"},
  //   {banks_[0].instances()[6][0], mux_order_[1], "input_4"},
  //   {banks_[0].instances()[5][0], mux_order_[1], "input_6"},
  //   {banks_[0].instances()[4][0], mux_order_[1], "input_7"},

  //   {banks_[0].instances()[3][0], mux_order_[0], "input_3"},
  //   {banks_[0].instances()[2][0], mux_order_[0], "input_2"},
  //   {banks_[0].instances()[1][0], mux_order_[0], "input_0"},
  //   {banks_[0].instances()[0][0], mux_order_[0], "input_1"},

  //   {banks_[1].instances()[7][0], mux_order_[1], "input_3"},
  //   {banks_[1].instances()[6][0], mux_order_[1], "input_2"},
  //   {banks_[1].instances()[5][0], mux_order_[1], "input_0"},
  //   {banks_[1].instances()[4][0], mux_order_[1], "input_1"},

  //   {banks_[1].instances()[3][0], mux_order_[0], "input_5"},
  //   {banks_[1].instances()[2][0], mux_order_[0], "input_4"},
  //   {banks_[1].instances()[1][0], mux_order_[0], "input_6"},
  //   {banks_[1].instances()[0][0], mux_order_[0], "input_7"},
  // };

  // for (auto &auto_connection : auto_mem_connections) {
  //   geometry::Instance *memory = auto_connection.source_memory;
  //   geometry::Instance *mux = auto_connection.target_mux;
  //   const std::string &input_name = auto_connection.mux_port_name;

  //   // Heuristically determine which mux port to use based on which which is
  //   // closest to the memory output, even if we're routing to the memory output
  //   // net instead of the port specifically.
  //   std::set<geometry::Port*> memory_ports;
  //   memory->GetInstancePorts("Q", &memory_ports);
  //   geometry::Port *memory_output = *memory_ports.begin();

  //   std::set<geometry::Port*> mux_ports_on_net;
  //   mux->GetInstancePorts(input_name, &mux_ports_on_net);

  //   geometry::Port *mux_port = mux->GetNearestPortNamed(*memory_output,
  //                                                       input_name);
  //   if (!mux_port) {
  //     continue;
  //   }
  //   LOG_IF(FATAL, mux_ports_on_net.find(mux_port) == mux_ports_on_net.end())
  //       << "Nearest port named " << input_name
  //       << " did not appear in list of all ports for same name";

  //   while (mux_port) {
  //     EquivalentNets net_names = EquivalentNets(
  //         {memory_output->net(), mux_port->net()});
  //     geometry::ShapeCollection non_net_connectables;
  //     layout->CopyConnectableShapesNotOnNets(net_names, &non_net_connectables);
  //     LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
  //               << " avoiding " << non_net_connectables.Describe();

  //     bool path_found = false;
  //     auto named_output_it = memory_output_net_names.find(memory);
  //     if (named_output_it == memory_output_net_names.end()) {
  //       memory_output_net_names[memory] = net_names.primary();
  //       LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
  //                 << " to " << memory->name();
  //       path_found = routing_grid.AddRouteBetween(
  //           *mux_port, *memory_output, non_net_connectables, net_names).ok();
  //     } else {
  //       // FIXME(aryap): I am stupid. The set of names given to the router to
  //       // determine which shapes are connectable is different to the target
  //       // set; in fact we must make sure that the net has a distinct name from
  //       // either start/end port so that routed wires can be differentiated from
  //       // start/end obstacles and ports!
  //       const std::string &target_net = named_output_it->second;
  //       net_names.set_primary(target_net);
  //       LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
  //                 << " to net " << target_net;
  //       path_found = routing_grid.AddRouteToNet(
  //           *mux_port, target_net, net_names, non_net_connectables).ok();
  //     }
  //     if (path_found) {
  //       break;
  //     }
  //     mux_ports_on_net.erase(mux_port);
  //     mux_port = mux_ports_on_net.empty() ? nullptr : *mux_ports_on_net.begin();
  //   }
  // }

  // Debug only.
  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);
  //routing_grid.ExportEdgesAsRectangles("areaid.frameRect", true, layout.get());

  grid_layout.reset(routing_grid.GenerateLayout());
  layout->AddLayout(*grid_layout, "routing");
}

// Align ports by x position and connect them.
void LutB::AddClockAndPowerStraps(Layout *layout) const {
  // Bucket ports according to
  //  - net
  //    - x position
  //      - set of y positions for ports at x on net
  std::unordered_map<std::string, std::map<int64_t, std::set<int64_t>>> buckets;

  std::array<std::string, 2> nets = {"CLK", "CLKI"};

  for (size_t bank = 0; bank < banks_.size(); ++bank) {
    for (const auto &row : banks_.at(bank).instances()) {
      for (geometry::Instance *instance : row) {
        for (const std::string &net : nets) {
          std::set<geometry::Port*> ports;
          instance->GetInstancePorts(net, &ports);
          for (geometry::Port *port : ports) {
            const geometry::Point centre = port->centre();
            buckets[net][centre.x()].insert(centre.y());
          }
        }
      }
    }
  }

  // FIXME(aryap): We are leaking technology-specific concerns into what was
  // previously somewhat agnostic; but was it ever really agnostic? There could
  // just be a strap configuration sction in the parameters:
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &met2_rules = db.Rules("met2.drawing");
  const auto &via1_rules = db.Rules("via1.drawing");
  const auto &met2_via1_rules = db.Rules("met2.drawing", "via1.drawing");

  layout->SetActiveLayerByName("met2.drawing");
  for (const auto &entry : buckets) {
    const std::string &net = entry.first;

    for (const auto &inner : entry.second) {
      int64_t x = inner.first;

      std::set<int64_t> y_positions(inner.second.begin(), inner.second.end());
      if (y_positions.size() < 2) {
        continue;
      }

      int64_t y_min = *y_positions.begin();
      int64_t y_max = *y_positions.rbegin();
      y_positions.erase(y_min);
      y_positions.erase(y_max);

      geometry::PolyLine line({{x, y_min}, {x, y_max}});
      line.SetWidth(met2_rules.min_width);
      line.set_min_separation(met2_rules.min_separation);
      geometry::Polygon *metal_pour = layout->AddPolyLine(line);
    }
  }
  layout->RestoreLastActiveLayer();
}

}  // namespace atoms
}  // namespace bfg
