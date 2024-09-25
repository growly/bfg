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
#include "../geometry/group.h"
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
      .horizontal_alignment = geometry::Compass::LEFT,
      .strap_alignment = geometry::Compass::RIGHT
    },
    .right = LutB::BankArrangement {
      .memory_rows = {7, 6, 5, 4, 3, 2, 1, 0},
      .buffer_rows = {0},
      .active_mux2_rows = {0},
      .clk_buf_rows = {3},
      .horizontal_alignment = geometry::Compass::RIGHT,
      .strap_alignment = geometry::Compass::LEFT
    },
    .mux_area_horizontal_padding = 2500,
    .mux_area_vertical_min_padding = 1250,
    .mux_area_rows = 2,
    .mux_area_columns = 2,
  }},
  //{5, LayoutConfig {
  //    .num_banks = 2,
  //    .bank_rows = 8,
  //    .bank_columns = 2,
  //    .mux_area_rows = 4,
  //    .mux_area_columns = 2,
  //    .mux_area_horizontal_padding = 2500,
  //    .rotate_first_row = true
  //}},
  //{6, LayoutConfig {
  //    .num_banks = 2,
  //    .bank_rows = 8,
  //    .bank_columns = 4,
  //    .mux_area_rows = 4,
  //    .mux_area_columns = 4,
  //    .mux_area_horizontal_padding = 2500,
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

      memories_.push_back(installed);
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
  int64_t x_pos =
      left_bank_bottom_row_right_x + layout_config.mux_area_horizontal_padding;
  // This staggers the mux area below the memories on the left:
  //int64_t y_pos = -mux_height / 2;
  int64_t y_pos = memories_.front()->Height() / 2;

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
  mux_grid.set_horizontal_overlap(-500);
  mux_grid.set_vertical_overlap(-2500);
  const std::vector<geometry::Instance*> &mux_order = mux_grid.InstantiateAll();
  mux_order_.insert(mux_order_.begin(), mux_order.begin(), mux_order.end());

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
      layout_config.mux_area_horizontal_padding;

  int64_t y_pitch = db.Rules("met1.drawing").min_pitch;
  // To maintain the relative alignment of the RoutingGrid to the cells, we
  // restrict the vertical space between the left and right banks to a multiple
  // of the vertical pitch. The minimum value this should take is the min
  // vertical spacing to the mux.
  int64_t y_diff = banks_[0].Origin().y() - (
      mux_grid.GetBoundingBox()->lower_left().y() -
      layout_config.mux_area_vertical_min_padding);
  y_pos = banks_[0].Origin().y() - std::ceil(
      static_cast<double>(y_diff) / static_cast<double>(y_pitch));

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
  RoutingGrid routing_grid(design_db_->physical_db());

  ConfigureRoutingGrid(&routing_grid, layout);

  // Debug only.
  //routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout.get());

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

  // The scan chain is connected in the order memories are assigned by the
  // BankArrangement.
  std::map<geometry::Instance*, std::string> memory_output_net_names;

  AddClockAndPowerStraps(&routing_grid, layout);

  RouteScanChain(&routing_grid, layout, &memory_output_net_names);
  //RouteRemainder(&routing_grid, layout);
  //RouteClockBuffers(&routing_grid, layout);
  RouteMuxInputs(&routing_grid, layout, &memory_output_net_names);

  // Debug only.
  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);
  //routing_grid.ExportEdgesAsRectangles("areaid.frameRect", true, layout.get());

  std::unique_ptr<bfg::Layout> grid_layout;
  grid_layout.reset(routing_grid.GenerateLayout());
  layout->AddLayout(*grid_layout, "routing");
}

void LutB::ConfigureRoutingGrid(
    RoutingGrid *routing_grid, Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();
  LOG(INFO) << "Pre-routing bounds: " << pre_route_bounds;
  bfg::RoutingLayerInfo met1_layer_info =
      db.GetRoutingLayerInfoOrDie("met1.drawing");
  met1_layer_info.direction = bfg::RoutingTrackDirection::kTrackHorizontal;
  met1_layer_info.area = pre_route_bounds;
  // TODO(aryap): Need an easier way of lining this up!
  met1_layer_info.offset = 70;

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
  routing_grid->AddRoutingViaInfo(
      met1_layer_info.layer, met2_layer_info.layer, routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("li.drawing", "met1.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid->AddRoutingViaInfo(
      met1_layer_info.layer, db.GetLayer("li.drawing"), routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("met2.drawing", "met3.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid->AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), met2_layer_info.layer, routing_via_info)
      .IgnoreError();

  //routing_grid.AddRoutingLayerInfo(li_layer_info);
  routing_grid->AddRoutingLayerInfo(met1_layer_info).IgnoreError();
  routing_grid->AddRoutingLayerInfo(met2_layer_info).IgnoreError();

  routing_grid->ConnectLayers(met1_layer_info.layer, met2_layer_info.layer)
      .IgnoreError();

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met1.drawing"), &shapes);
    routing_grid->AddBlockages(shapes);
  }
  {
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met2.drawing"), &shapes);
    routing_grid->AddBlockages(shapes);
  }

}

void LutB::RouteClockBuffers(RoutingGrid *routing_grid,
                             Layout *layout) const {
  // Connect clock buffers to straps.
  // Connect "X" from clock buf to CLK;
  // connect "P" from clock buf to CLKI.
  std::vector<PortKeyCollection> clk_connections;
  for (size_t bank = 0; bank < banks_.size(); ++bank) {
    PortKeyCollection &clk_collection = clk_connections.emplace_back();
    clk_collection.port_keys.push_back({
        .instance = clk_buf_order_[bank],
        .port_name = "X"
    });
    clk_collection.net_name = absl::StrCat("clk_", bank);

    PortKeyCollection &clk_i_collection = clk_connections.emplace_back();
    clk_i_collection.port_keys.push_back({
        .instance = clk_buf_order_[bank],
        .port_name = "P"
    });
    clk_i_collection.net_name = absl::StrCat("clk_i_", bank);
  }

  for (auto &clk_connection : clk_connections) {
    PortKey &source_spec = clk_connection.port_keys[0];
    geometry::Port *source_port =
        source_spec.instance->GetFirstPortNamed(source_spec.port_name);
    const std::string &target_net = *clk_connection.net_name;

    // Note that source_port->net() will include the source_port's instance
    // name, which is important for disambiguating the port in the context of
    // the instantiating cell.
    EquivalentNets net_aliases({target_net, source_port->net()});

    geometry::ShapeCollection non_net_connectables;
    layout->CopyConnectableShapesNotOnNets(net_aliases, &non_net_connectables);

    auto result = routing_grid->AddRouteToNet(
        *source_port, target_net, net_aliases, non_net_connectables);
  }

  PortKeyCollection clk_inputs;
  for (geometry::Instance *clk_buf : clk_buf_order_) {
    clk_inputs.port_keys.push_back({
        .instance = clk_buf,
        .port_name = "A"
    });
  }
  AddMultiPointRoute(clk_inputs, routing_grid, layout).IgnoreError();
}

void LutB::RouteScanChain(
    RoutingGrid *routing_grid,
    Layout *layout,
    std::map<geometry::Instance*, std::string> *memory_output_net_names)
    const {
  for (size_t i = 0; i < memories_.size() - 1; ++i) {
    geometry::Instance *source = memories_[i];
    geometry::Instance *sink = memories_[i + 1];

    LOG(INFO) << "Adding scan routes for pair "
              << source->name() << ", " << sink->name();

    std::set<geometry::Port*> ports;
    source->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();
    ports.clear();

    sink->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();

    EquivalentNets net_names = EquivalentNets({end->net(), start->net()});
    (*memory_output_net_names)[source] = net_names.primary();

    geometry::ShapeCollection non_net_connectables;
    layout->CopyConnectableShapesNotOnNets(
        net_names,
        &non_net_connectables);
    routing_grid->AddRouteBetween(*start, *end, non_net_connectables, net_names)
                .IgnoreError();
  }
}

void LutB::RouteMuxInputs(
    RoutingGrid *routing_grid,
    Layout *layout,
    std::map<geometry::Instance*, std::string> *memory_output_net_names)
    const {
  // Connect flip-flops to mux.

  // TODO(aryap): We know that the mux connections roughly map to the nearest
  // flip flops in groups of 4; we should automate finding the order within
  // those groups that yield best routes.

  // The mux input order is, from top to bottom:
  // input_5  --+---------
  // input_4  --|
  // input_6  --|
  // input_7  --|  8:1 mux
  // input_3  --|
  // input_2  --|
  // input_0  --|
  // input_1  --+---------
  std::vector<AutoMemoryMuxConnection> auto_mem_connections = {
    //{banks_[0].instances()[7][0], mux_order_[1], "input_5"},
    // {banks_[0].instances()[5][0], mux_order_[1], "input_6"},
    //{banks_[0].instances()[6][0], mux_order_[1], "input_4"},
    //{banks_[0].instances()[4][0], mux_order_[1], "input_7"},

    // {banks_[0].instances()[1][0], mux_order_[0], "input_0"},
    // {banks_[0].instances()[3][0], mux_order_[0], "input_3"},
    //{banks_[0].instances()[2][0], mux_order_[0], "input_2"},
    //{banks_[0].instances()[0][0], mux_order_[0], "input_1"},

    {banks_[1].instances()[5][0], mux_order_[1], "input_0"},
    //{banks_[1].instances()[7][0], mux_order_[1], "input_3"},
    //{banks_[1].instances()[6][0], mux_order_[1], "input_2"},
    //{banks_[1].instances()[4][0], mux_order_[1], "input_1"},

    //{banks_[1].instances()[3][0], mux_order_[0], "input_5"},
    //{banks_[1].instances()[2][0], mux_order_[0], "input_4"},
    //{banks_[1].instances()[1][0], mux_order_[0], "input_6"},
    //{banks_[1].instances()[0][0], mux_order_[0], "input_7"},
  };

  for (auto &auto_connection : auto_mem_connections) {
    geometry::Instance *memory = auto_connection.source_memory;
    geometry::Instance *mux = auto_connection.target_mux;
    const std::string &input_name = auto_connection.mux_port_name;

    // Heuristically determine which mux port to use based on which which is
    // closest to the memory output, even if we're routing to the memory output
    // net instead of the port specifically.
    std::set<geometry::Port*> memory_ports;
    memory->GetInstancePorts("Q", &memory_ports);
    geometry::Port *memory_output = *memory_ports.begin();

    std::set<geometry::Port*> mux_ports_on_net;
    mux->GetInstancePorts(input_name, &mux_ports_on_net);

    geometry::Port *mux_port = mux->GetNearestPortNamed(*memory_output,
                                                        input_name);
    if (!mux_port) {
      continue;
    }
    LOG_IF(FATAL, mux_ports_on_net.find(mux_port) == mux_ports_on_net.end())
        << "Nearest port named " << input_name
        << " did not appear in list of all ports for same name";

    // TODO(aryap): Why can't AddMultiPointRoute just replace this? Speed?
    while (mux_port) {
      EquivalentNets net_names = EquivalentNets(
          {memory_output->net(), mux_port->net()});
      geometry::ShapeCollection non_net_connectables;
      layout->CopyConnectableShapesNotOnNets(net_names, &non_net_connectables);
      LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                << " avoiding " << non_net_connectables.Describe();

      bool path_found = false;
      auto named_output_it = memory_output_net_names->find(memory);
      if (named_output_it == memory_output_net_names->end()) {
        (*memory_output_net_names)[memory] = net_names.primary();
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to " << memory->name();
        path_found = routing_grid->AddRouteBetween(
            *mux_port, *memory_output, non_net_connectables, net_names).ok();
      } else {
        // FIXME(aryap): I am stupid. The set of names given to the router to
        // determine which shapes are connectable is different to the target
        // set; in fact we must make sure that the net has a distinct name from
        // either start/end port so that routed wires can be differentiated from
        // start/end obstacles and ports!
        const std::string &target_net = named_output_it->second;
        net_names.set_primary(target_net);
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to net " << target_net;
        path_found = routing_grid->AddRouteToNet(
            *mux_port, target_net, net_names, non_net_connectables).ok();
      }
      if (path_found) {
        break;
      }
      mux_ports_on_net.erase(mux_port);
      mux_port = mux_ports_on_net.empty() ? nullptr : *mux_ports_on_net.begin();
    }
  }
}

void LutB::RouteRemainder(RoutingGrid *routing_grid, Layout *layout) const {
  // Connect the input buffers on the selector lines.
  // TODO(aryap): These feel like first-class members of the RoutingGrid API
  // soon. "RouteGroup"?
  std::vector<PortKeyCollection> auto_connections = {{
      .port_keys = {{buf_order_[0], "P"}, {mux_order_[0], "S0_B"},
                    {mux_order_[1], "S0_B"}},
    }, {
      .port_keys = {{buf_order_[0], "X"}, {mux_order_[0], "S0"},
                    {mux_order_[1], "S0"}},
    }, {
      .port_keys = {{buf_order_[1], "P"}, {mux_order_[0], "S1_B"},
                    {mux_order_[1], "S1_B"}},
    }, {
      .port_keys = {{buf_order_[1], "X"}, {mux_order_[0], "S1"},
                    {mux_order_[1], "S1"}},
    }, {
      .port_keys = {{buf_order_[2], "P"}, {mux_order_[0], "S2_B"},
                    {mux_order_[1], "S2_B"}},
    }, {
      .port_keys = {{buf_order_[2], "X"}, {mux_order_[0], "S2"},
                    {mux_order_[1], "S2"}},
    }, {
      .port_keys = {{buf_order_[3], "X"}, {active_mux2s_[0], "S"}},
    }, {
      .port_keys = {{mux_order_[0], "Z"}, {active_mux2s_[0], "A0"}},
    }, {
      .port_keys = {{mux_order_[1], "Z"}, {active_mux2s_[0], "A1"}},
    }, {
      .port_keys = {{active_mux2s_[0], "X"}, {buf_order_[3], "A"}},
    }
  };

  for (const PortKeyCollection &collection : auto_connections) {
    AddMultiPointRoute(collection, routing_grid, layout).IgnoreError();
  }
}

// TODO(aryap): This clearly needs to be factored out of this class somehow.
absl::Status LutB::AddMultiPointRoute(const PortKeyCollection &collection,
                                      RoutingGrid *routing_grid,
                                      Layout *layout) const {
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

  return routing_grid->AddMultiPointRoute(*layout,
                                          route_targets,
                                          collection.net_name);
}

geometry::Group LutB::AddVerticalSpineWithFingers(
    const std::string &spine_layer_name,
    const std::string &via_layer_name,
    const std::string &finger_layer_name,
    const std::string &net,
    const std::vector<geometry::Point> &connections,
    int64_t spine_x,
    int64_t spine_width,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &spine_rules = db.Rules(spine_layer_name);
  const auto &finger_rules = db.Rules(finger_layer_name);
  const auto &via_rules = db.Rules(via_layer_name);
  const auto &spine_via_rules = db.Rules(spine_layer_name, via_layer_name);
  const auto &finger_via_rules = db.Rules(finger_layer_name, via_layer_name);

  geometry::Group created_shapes;

  // Sort points by y (the key) and remove duplicates by keeping either the
  // closest or the furthest point from spine_x.
  std::map<int64_t, geometry::Point> points;
  for (const geometry::Point &point : connections) {
    auto it = points.find(point.y());
    if (it != points.end()) {
      geometry::Point on_spine = {spine_x, point.y()};
      const geometry::Point &existing = it->second;
      // Keeps closest point:
      if (point.L1DistanceTo(on_spine) < existing.L1DistanceTo(on_spine)) {
        points[point.y()] = point;
      }
    } else {
      points[point.y()] = point;
    }
  }

  if (points.size() < 2) {
    return created_shapes;
  }

  // Draw spine.
  int64_t y_min = points.begin()->second.y();
  int64_t y_max = points.rbegin()->second.y();

  geometry::PolyLine spine_line({{spine_x, y_min}, {spine_x, y_max}});
  spine_line.SetWidth(
      std::max(spine_rules.min_width, spine_width));
  spine_line.set_min_separation(spine_rules.min_separation);
  spine_line.set_net(net);

  uint64_t via_side = std::max(via_rules.via_width, via_rules.via_height);
  uint64_t spine_bulge_width = 2 * spine_via_rules.via_overhang_wide + via_side;
  uint64_t spine_bulge_length = 2 * spine_via_rules.via_overhang + via_side;
  uint64_t finger_bulge_width =
      2 * finger_via_rules.via_overhang_wide + via_side;
  uint64_t finger_bulge_length = 2 * finger_via_rules.via_overhang + via_side;

  for (const auto &entry : points) {
    const geometry::Point &point = entry.second;
    if (point.x() == spine_x) {
      spine_line.InsertBulge(point, spine_bulge_width, spine_bulge_length);
      layout->MakeVia(via_layer_name, point, net);
      continue;
    }
    geometry::Point spine_via = {spine_x, point.y()};
    // Have to draw a finger!
    geometry::PolyLine finger({point, spine_via});

    finger.SetWidth(finger_rules.min_width);
    finger.set_min_separation(finger_rules.min_separation);
    finger.InsertBulge(spine_via, finger_bulge_width, finger_bulge_length);
    finger.set_net(net);
    layout->SetActiveLayerByName(finger_layer_name);
    geometry::Polygon *finger_polygon = layout->AddPolyLine(finger);
    created_shapes.Add(finger_polygon);

    layout->RestoreLastActiveLayer();

    geometry::Rectangle *via = layout->MakeVia(via_layer_name, spine_via, net);
    created_shapes.Add(via);

    spine_line.InsertBulge(spine_via, spine_bulge_width, spine_bulge_length);

    // TODO: do we worry about the via from the finger to the connection pin
    // here?
    // finger.InsertBulge(point, finger_bulge_width, finger_bulge_length);
  }

  layout->SetActiveLayerByName(spine_layer_name);
  geometry::Polygon *spine_metal_pour = layout->AddPolyLine(spine_line);
  created_shapes.Add(spine_metal_pour);
  layout->RestoreLastActiveLayer();

  return created_shapes;
}

// Align ports by x position and connect them.
void LutB::AddClockAndPowerStraps(
    RoutingGrid *routing_grid, Layout *layout) const {
  static const std::array<std::string, 4> kPortNames =
      {"VPWR", "VGND", "CLK", "CLKI"};
  static const std::array<std::string, 4> kNets =
      {"vpwr", "vgnd", "clk", "clk_i"};

  // FIXME(aryap): We are leaking technology-specific concerns into what was
  // previously somewhat agnostic; but was it ever really agnostic? There could
  // just be a strap configuration sction in the parameters:
  // TODO(aryap): What if we has a class SyntheticRules that created common
  // derivative rules from the base rule structs? Maybe users can define them
  // with std::functions (functors) in a standard form...
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &spine_via_rules = db.Rules("met2.drawing", "via1.drawing");
  const auto &spine_rules = db.Rules("met2.drawing");
  const auto &via_rules = db.Rules("via1.drawing");
  uint64_t via_side = std::max(via_rules.via_width, via_rules.via_height);
  uint64_t spine_bulge_width = 2 * spine_via_rules.via_overhang_wide + via_side;
  int64_t strap_pitch = spine_bulge_width + spine_rules.min_separation;

  const LutB::LayoutConfig layout_config =
      *LutB::GetLayoutConfiguration(lut_size_);
  std::vector<geometry::Compass> strap_alignment_per_bank = {
      layout_config.left.strap_alignment,
      layout_config.right.strap_alignment};

  for (size_t bank = 0; bank < banks_.size(); ++bank) {
    std::optional<int64_t> last_spine_x;
    for (size_t i = 0; i < kPortNames.size(); ++i) {
      const std::string &port_name = kPortNames[i];
      std::vector<geometry::Point> connections;
      for (const auto &row : banks_.at(bank).instances()) {
        for (geometry::Instance *instance : row) {
          std::set<geometry::Port*> ports;
          instance->GetInstancePorts(port_name, &ports);
          for (geometry::Port *port : ports) {
            connections.push_back(port->centre());
          }
        }
      }

      // Sort connections so that the left-most (lowest-x) is at the front.
      // Thus pick the left-most port.
      std::sort(
          connections.begin(), connections.end(),
          geometry::Point::CompareX);

      int64_t spine_x = 0;
      if (strap_alignment_per_bank[bank] == geometry::Compass::LEFT) {
        spine_x = connections.front().x();
        if (last_spine_x) {
          spine_x = std::min(spine_x, *last_spine_x - strap_pitch);
        }
      } else if (strap_alignment_per_bank[bank] == geometry::Compass::RIGHT) {
        spine_x = connections.back().x();
        if (last_spine_x) {
          spine_x = std::max(spine_x, *last_spine_x + strap_pitch);
        }
      }
      last_spine_x = spine_x;

      std::string net = absl::StrCat(kNets[i], "_", bank);

      geometry::Group new_shapes =
          AddVerticalSpineWithFingers("met2.drawing",
                                      "via1.drawing",
                                      "met1.drawing",
                                      net,
                                      connections,
                                      spine_x,
                                      spine_bulge_width,
                                      layout);
      routing_grid->AddBlockages(new_shapes);
    }
  }
}

}  // namespace atoms
}  // namespace bfg
