#include "lut_b.h"

#include <map>
#include <optional>
#include <unordered_map>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "../atoms/sky130_buf.h"
#include "../atoms/sky130_decap.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_mux.h"
#include "../atoms/sky130_tap.h"
#include "../checkerboard_guide.h"
#include "../equivalent_nets.h"
#include "../geometry/compass.h"
#include "../geometry/group.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"
#include "../geometry/port.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../routing_layer_info.h"
#include "../routing_via_info.h"
#include "../row_guide.h"
#include "../utility.h"
#include "proto/parameters/lut_b.pb.h"

namespace bfg {
namespace tiles {

void LutB::Parameters::ToProto(proto::parameters::LutB *pb) const {
  pb->set_lut_size(lut_size);
}

void LutB::Parameters::FromProto(const proto::parameters::LutB &pb) {
  if (pb.has_lut_size()) {
    lut_size = pb.lut_size();
  }
}

const std::pair<size_t, LutB::LayoutConfig> LutB::kLayoutConfigurations[] = {
  {4, LutB::LayoutConfig {
    .left = LutB::BankArrangement {
      .memory_rows = {0, 1, 2, 3, 4, 5, 6, 7},
      .buffer_rows = {7, 7, 7},
      .clk_buf_rows = {4},
      .horizontal_alignment = geometry::Compass::LEFT,
      .strap_alignment = geometry::Compass::RIGHT,
      .alternate_rotation = false
    },
    .right = LutB::BankArrangement {
      .memory_rows = {8, 8, 7, 6, 5, 4, 3, 2},
      .buffer_rows = {0},
      .active_mux2_rows = {0},
      .clk_buf_rows = {3},
      .horizontal_alignment = geometry::Compass::RIGHT,
      .strap_alignment = geometry::Compass::LEFT,
      .alternate_rotation = true
    },
    .mux_area_horizontal_padding = 0,   // Determined at runtime.
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

Cell *LutB::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> lut_cell(new Cell(name));
  std::unique_ptr<Layout> layout(new Layout(db));
  std::unique_ptr<Circuit> circuit(new Circuit());

  memories_.clear();
  clk_buf_order_.clear();
  active_mux2s_.clear();
  mux_order_.clear();
  buf_order_.clear();
  banks_.clear();

  const LutB::LayoutConfig layout_config =
      *LutB::GetLayoutConfiguration(parameters_.lut_size);
  constexpr int64_t kMuxSize = 8;

  // Circuit setup.
  // ---------------------------------------------------------------------------

  // Selector signals S0, S1, S2, ... S(K - 1)
  for (size_t i = 0; i < parameters_.lut_size; ++i) {
    circuit->AddPort(circuit->AddSignal(absl::StrCat("S", i)));
  }
  // Output.
  circuit->AddPort(circuit->AddSignal("Z"));
  // Scan in and out.
  circuit->AddPort(circuit->AddSignal("CONFIG_IN"));
  circuit->AddPort(circuit->AddSignal("CONFIG_OUT"));

  // Clock, power, ground in.
  circuit->AddPort(circuit->AddSignal("CLK"));
  // TODO(aryap): For now there's actually one of these per bank.
  circuit->AddPort(circuit->AddSignal("VPWR_0"));
  circuit->AddPort(circuit->AddSignal("VPWR_1"));
  circuit->AddPort(circuit->AddSignal("VGND_0"));
  circuit->AddPort(circuit->AddSignal("VGND_1"));

  // Layout.
  // ---------------------------------------------------------------------------
  int64_t buf_y_pos = 0;

  atoms::Sky130Tap::Parameters tap_params = {
    .height_nm = 2720,
    .width_nm = 460
  };
  atoms::Sky130Tap tap_generator(tap_params, design_db_);
  Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      "lut_tap_template");

  std::vector<std::reference_wrapper<const BankArrangement>> arrangements = {
    layout_config.left, layout_config.right};

  for (size_t p = 0; p < arrangements.size(); ++p) {
    const BankArrangement &bank_arrangement = arrangements[p].get();
    banks_.push_back(MemoryBank(layout.get(),
                                circuit.get(),
                                design_db_,
                                tap_cell,
                                true,  // Rotate alternate rows.
                                // Rotate the first row.
                                !bank_arrangement.alternate_rotation,
                                bank_arrangement.horizontal_alignment));
    MemoryBank &bank = banks_.back();

    // We now want to assign things to rows and have the memory bank create the
    // row if they don't exist.

    size_t num_memories = 0;
    for (size_t i = 0; i < bank_arrangement.memory_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.memory_rows[i];

      std::string template_name = absl::StrFormat(
          "lut_dfxtp_%d_%d", p, num_memories);
      std::string instance_name = absl::StrCat(template_name, "_i");
      atoms::Sky130Dfxtp::Parameters params;
      atoms::Sky130Dfxtp generator(params, design_db_);
      Cell *cell = generator.GenerateIntoDatabase(template_name);
      cell->layout()->DeletePorts("QI");

      geometry::Instance *installed =
          bank.InstantiateRight(assigned_row, instance_name, cell);

      //circuit::Instance *circuit_instance =
      //    circuit->AddInstance(instance_name, cell->circuit());
      //Cell::TieInstances(circuit_instance, installed);

      memories_.push_back(installed);
      ++num_memories;
    }
  }

  LOG_IF(FATAL, banks_.size() < 1) << "Expected at least 1 bank by this point.";

  banks_[0].MoveTo(geometry::Point(0, 0));

  // Set the grid alignment point to fall on the output port of this memory:
  std::vector<geometry::Port*> q_ports;
  memories_[0]->GetInstancePorts("Q", &q_ports);
  layout->SavePoint("grid_alignment_point", q_ports.front()->centre());

  atoms::Sky130Mux::Parameters mux_params;
  mux_params.extend_inputs_top = true;
  mux_params.extend_inputs_bottom = false;

  atoms::Sky130Mux mux(mux_params, design_db_);
  Cell *base_mux_cell = mux.GenerateIntoDatabase("sky130_mux");

  // A second version of the mux has its inputs on the bottom instead of the
  // top:
  atoms::Sky130Mux::Parameters alt_mux_params;
  alt_mux_params.extend_inputs_top = false;
  alt_mux_params.extend_inputs_bottom = true;

  Cell *alt_mux_cell = atoms::Sky130Mux(
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

  int64_t met1_x_pitch = db.Rules("met1.drawing").min_pitch;
  int64_t mux_area_horizontal_padding =
      layout_config.mux_area_horizontal_padding +
      3 * met1_x_pitch;

  int64_t x_pos =
      left_bank_bottom_row_right_x + mux_area_horizontal_padding;

  // This staggers the mux area below the memories on the left:
  //int64_t y_pos = -mux_height / 2;
  int64_t y_pos = memories_.front()->Height() / 2;

  std::vector<Cell*> mux_templates = {base_mux_cell, alt_mux_cell};

  CheckerboardGuide mux_grid(geometry::Point(x_pos, y_pos),
                             "mux",
                             layout_config.mux_area_rows,
                             layout_config.mux_area_columns,
                             layout.get(),
                             circuit.get(),
                             design_db_);
  mux_grid.set_template_cells(&mux_templates);
  // FIXME(aryap): This is a function of track pitch, really, it's not some
  // number I eyeballed. Except that it *is* some number I just eyeballed and it
  // should be a function of track pitch.
  mux_grid.set_horizontal_overlap(18 * met1_x_pitch);
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
      atoms::Sky130Buf::Parameters buf_params; // Default values.
      atoms::Sky130Buf buf_generator(buf_params, design_db_);
      Cell *buf_cell = buf_generator.GenerateIntoDatabase(cell_name);
      buf_cell->layout()->ResetY();
      geometry::Instance *installed = bank.InstantiateInside(
          assigned_row, instance_name, buf_cell);
      buf_order_.push_back(installed);

      //circuit::Instance *circuit_instance = circuit->AddInstance(
      //    instance_name, buf_cell->circuit());
      //Cell::TieInstances(circuit_instance, installed);
    }

    for (size_t i = 0; i < bank_arrangement.clk_buf_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.clk_buf_rows[i];
      size_t buf_count = clk_buf_order_.size();

      std::string instance_name = absl::StrFormat("clk_buf_%d", buf_count);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130Buf::Parameters buf_params;  // Defaults.
      atoms::Sky130Buf buf_generator(buf_params, design_db_);
      Cell *buf_cell = buf_generator.GenerateIntoDatabase(cell_name);
      buf_cell->layout()->ResetY();
      geometry::Instance *installed = bank.InstantiateInside(
          assigned_row, instance_name, buf_cell);
      clk_buf_order_.push_back(installed);

      //circuit::Instance *circuit_instance = circuit->AddInstance(
      //    instance_name, buf_cell->circuit());
      //Cell::TieInstances(circuit_instance, installed);
    }

    for (size_t i = 0; i < bank_arrangement.active_mux2_rows.size(); ++i) {
      size_t assigned_row = bank_arrangement.active_mux2_rows[i];

      std::string instance_name = absl::StrFormat("hd_mux2_1_%d", i);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130HdMux21 active_mux2_generator({}, design_db_);
      Cell *active_mux2_cell = active_mux2_generator.GenerateIntoDatabase(
          cell_name);
      active_mux2_cell->layout()->ResetY();
      geometry::Instance *instance = bank.InstantiateInside(
          assigned_row, instance_name, active_mux2_cell);
      active_mux2s_.push_back(instance);

      //circuit::Instance *circuit_instance = circuit->AddInstance(
      //    instance_name, active_mux2_cell->circuit());
      //Cell::TieInstances(circuit_instance, instance);
    }
  }

  // Place the registered output flop in the bottom-most row of the right bank.
  // Also place the input select mux here: we can register 
  // This is clocked by the application clock, not the scan clock!
  //
  // FIXME(aryap): Route these.
  //
  // First, the mux:
  {
    std::string template_name = "register_select_hd_mux2_1";
    std::string instance_name = absl::StrCat(instance_name, "_i");
    atoms::Sky130HdMux21 register_mux_generator({}, design_db_);
    Cell *register_mux_cell = register_mux_generator.GenerateIntoDatabase(
        template_name);
    register_mux_cell->layout()->ResetY();
    geometry::Instance *instance = banks_[1].InstantiateLeft(
        0, instance_name, register_mux_cell);
    // TODO(aryap): Do we need to store this?
    //active_mux2s_.push_back(instance);
  }

  // Then the register:
  {
    std::string template_name = "register_dfxtp";
    std::string instance_name = absl::StrCat(template_name, "_i");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp generator(params, design_db_);
    Cell *register_cell = generator.GenerateIntoDatabase(template_name);
    register_cell->layout()->DeletePorts("QI");
    geometry::Instance *installed =
        banks_[1].InstantiateLeft(0, instance_name, register_cell);
  }

  // Then the memory holding the configuration for the output mux:
  {
    std::string template_name = "register_config_dfxtp";
    std::string instance_name = absl::StrCat(template_name, "_i");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp generator(params, design_db_);
    Cell *register_cell = generator.GenerateIntoDatabase(template_name);
    register_cell->layout()->DeletePorts("QI");
    geometry::Instance *installed =
        banks_[1].InstantiateLeft(0, instance_name, register_cell);
  }

  // Next we add the combinational output select mux:
  {
    std::string template_name = "combinational_select_hd_mux2_1";
    std::string instance_name = absl::StrCat(instance_name, "_i");
    atoms::Sky130HdMux21 combinational_mux_generator({}, design_db_);
    Cell *combinational_mux_cell =
        combinational_mux_generator.GenerateIntoDatabase(template_name);
    combinational_mux_cell->layout()->ResetY();
    geometry::Instance *instance = banks_[1].InstantiateRight(
        0, instance_name, combinational_mux_cell);
    // TODO(aryap): Do we need to store this?
    //active_mux2s_.push_back(instance);
  }

  // Then the memory holding the configuration for the output mux:
  {
    std::string template_name = "combinational_config_dfxtp";
    std::string instance_name = absl::StrCat(template_name, "_i");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp generator(params, design_db_);
    Cell *combinational_cell = generator.GenerateIntoDatabase(template_name);
    combinational_cell->layout()->DeletePorts("QI");
    geometry::Instance *installed =
        banks_[1].InstantiateRight(1, instance_name, combinational_cell);
  }


  // Now that all instances have been assigned to the banks and their
  // dimensions are known, move them into place around the muxes. Well, move
  // the right bank because the first bank is fixed.
  int64_t right_bank_row_2_left_x = banks_[1].Row(2).LowerLeft().x();
  int64_t right_bank_row_2_width = banks_[1].Row(2).GetTilingBounds()->Width();
  int64_t right_bank_bottom_row_top_y =
      banks_[1].rows().front().UpperLeft().y();


  x_pos = mux_grid.GetBoundingBox()->upper_right().x() +
      mux_area_horizontal_padding;

  // We now have the opportunity to position the right bank so that the overall
  // tile width is a multiple of something, if required.
  //
  // TODO(aryap): This assumes that the left-most point on the layout is at x=0.
  std::optional<int64_t> width_unit = db.ToInternalUnits(
      parameters_.tiling_width_unit_nm);
  if (width_unit) {
    int64_t total_width = x_pos + right_bank_row_2_width;
    int64_t required_width = Utility::NextMultiple(total_width, *width_unit);
    x_pos += (required_width - total_width);
  }

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
      {right_bank_row_2_left_x, right_bank_bottom_row_top_y},
      {x_pos, y_pos});

  // We can now fill any gaps with decaps.
  //
  // NOTE(aryap): We are statically assuming only a single top row. That might
  // not be true if generalise this to larger LUTs.
  //
  // We could also use any available gap for a passive mux to select between two
  // adjacent 4-LUT structures.
  int64_t top_row_available_x =
      banks_[1].rows().back().GetTilingBounds()->lower_left().x() -
      banks_[0].rows().back().GetTilingBounds()->upper_right().x();
  if (top_row_available_x >= db.ToInternalUnits(
          atoms::Sky130Decap::Parameters::kMinWidthNm) &&
      top_row_available_x <= db.ToInternalUnits(
          atoms::Sky130Decap::Parameters::kMaxWidthNm)) {
    std::string template_name = "top_decap_fill";
    atoms::Sky130Decap::Parameters decap_params = {
      .width_nm = static_cast<uint64_t>(db.ToExternalUnits(top_row_available_x))
    };
    atoms::Sky130Decap decap_generator(decap_params, design_db_);
    Cell *decap_cell = decap_generator.GenerateIntoDatabase(template_name);
    geometry::Instance *decap = banks_[0].InstantiateRight(
        banks_[0].NumRows() - 1,
        absl::StrCat(template_name, "_i0"),
        decap_cell);
  }




  Route(circuit.get(), layout.get());

  // //// FIXME(aryap): remove
  // ///DEBUG
  // //lut_cell->SetLayout(layout.release());
  // //lut_cell->SetCircuit(circuit.release());
  // //Cell *pre = lut_cell.release();
  // //pre->set_name(name);
  // //design_db_->ConsumeCell(pre);
  // //return pre;
  lut_cell->SetLayout(layout.release());
  lut_cell->SetCircuit(circuit.release());
  Cell *cell = lut_cell.release();
  cell->set_name(name);
  design_db_->ConsumeCell(cell);
  return cell;
}

void LutB::Route(Circuit *circuit, Layout *layout) {
  RoutingGrid routing_grid(design_db_->physical_db());

  ConfigureRoutingGrid(&routing_grid, layout);

  // Debug only.
  //routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout.get());

  // TODO(aryap): I want to solve the general problem of connecting to a port
  // on an instance which is comprised of many, possibly connected, shapes on
  // many, possibly connected, layers. The tricky thing is that connecting on
  // one layer might create DRC violations on an adjacent layer (e.g. if you
  // connect on met2 but jump up from met1 just before, and there's a met1
  // shape near, you have a problem).
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

  AddClockAndPowerStraps(&routing_grid, circuit, layout);

  errors_.clear();

  //RouteScanChain(&routing_grid, circuit, layout, &memory_output_net_names);
  //RouteClockBuffers(&routing_grid, circuit, layout);
  //RouteMuxInputs(&routing_grid, circuit, layout, &memory_output_net_names);
  //RouteRemainder(&routing_grid, circuit, layout);
  //RouteInputs(&routing_grid, circuit, layout);
  //RouteOutputs(&routing_grid, circuit, layout);

  for (const absl::Status &error : errors_) {
    LOG(ERROR) << "Routing error: " << error;
  }

  // Debug only.
  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);
  //routing_grid.ExportEdgesAsRectangles("areaid.frameRect", true, layout.get());

  std::unique_ptr<Layout> grid_layout;
  grid_layout.reset(routing_grid.GenerateLayout());
  layout->AddLayout(*grid_layout, "routing");
}

void LutB::ConfigureRoutingGrid(
    RoutingGrid *routing_grid, Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();
  LOG(INFO) << "Pre-routing bounds: " << pre_route_bounds;
  RoutingLayerInfo met1_layer_info =
      db.GetRoutingLayerInfoOrDie("met1.drawing");
  met1_layer_info.set_direction(RoutingTrackDirection::kTrackHorizontal);
  met1_layer_info.set_area(pre_route_bounds);
  // TODO(aryap): Need an easier way of lining this up!
  // met1_layer_info.offset = 70;

  RoutingLayerInfo met2_layer_info =
      db.GetRoutingLayerInfoOrDie("met2.drawing");
  met2_layer_info.set_direction(RoutingTrackDirection::kTrackVertical);
  met2_layer_info.set_area(pre_route_bounds);

  auto alignment_point = layout->GetPoint("grid_alignment_point");
  if (alignment_point) {
    LOG(INFO) << "Aligning grid to " << *alignment_point;
    RoutingGridGeometry::AlignRoutingLayerInfos(
        *alignment_point, &met1_layer_info, &met2_layer_info);
  }

  // TODO(aryap): Store connectivity information (which layers connect through
  // which vias) in the PhysicalPropertiesDatabase's via_layers_.
  RoutingViaInfo routing_via_info =
      db.GetRoutingViaInfoOrDie("met1.drawing", "met2.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid->AddRoutingViaInfo(
      met1_layer_info.layer(), met2_layer_info.layer(), routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("li.drawing", "met1.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid->AddRoutingViaInfo(
      met1_layer_info.layer(), db.GetLayer("li.drawing"), routing_via_info)
      .IgnoreError();

  routing_via_info = db.GetRoutingViaInfoOrDie("met2.drawing", "met3.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid->AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), met2_layer_info.layer(), routing_via_info)
      .IgnoreError();

  //routing_grid.AddRoutingLayerInfo(li_layer_info);
  routing_grid->AddRoutingLayerInfo(met1_layer_info).IgnoreError();
  routing_grid->AddRoutingLayerInfo(met2_layer_info).IgnoreError();

  routing_grid->ConnectLayers(met1_layer_info.layer(), met2_layer_info.layer())
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

  // TODO(aryap): Don't want designers to have to manually duplicate this
  // information everywhere:
  routing_grid->AddGlobalNet("CLK");
}

// FIXME(aryap): The clock/power/etc straps need to connect to the left most
// ports of the left- or right-most memories on each row of the bank, depending
// on the bank, and they need to connect to every memory on that row.
void LutB::RouteClockBuffers(RoutingGrid *routing_grid,
                             Circuit *circuit,
                             Layout *layout) {
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

    circuit::Signal *signal = circuit->GetOrAddSignal(target_net, 1);
    source_spec.instance->circuit_instance()->Connect(
        source_spec.port_name, *signal);

    auto result = routing_grid->AddRouteToNet(
        *source_port, target_net, net_aliases, non_net_connectables);
  }

  PortKeyCollection clk_inputs;
  for (geometry::Instance *clk_buf : clk_buf_order_) {
    clk_inputs.port_keys.push_back({
        .instance = clk_buf,
        .port_name = "A"
    });

    // This matches the input port name, so that the connecting net label
    // matches the incoming port label.
    clk_inputs.net_name = "CLK";
  }
  auto result = AddMultiPointRoute(clk_inputs, routing_grid, circuit, layout);
  if (result.ok()) {
    for (RoutingPath *path : *result) {
      path->AddPortMidway("CLK");
    }
  }
  AccumulateAnyErrors(result.status());
}

void LutB::RouteScanChain(
    RoutingGrid *routing_grid,
    Circuit *circuit,
    Layout *layout,
    std::map<geometry::Instance*, std::string> *memory_output_net_names) {
  // For now the input/output of the first/last flip-flop (respectively) is just
  // the port for the entire LUT; later we route this to pins on the edge:
  memories_.front()->circuit_instance()->Connect("D",
      *circuit->GetOrAddSignal("CONFIG_IN", 1));

  // FIXME(aryap): This is terrible! We need a way to re-assign, or connect,
  // nets with the same names. A first and easier step is to simply rename an
  // existing signal... but that has problems too. Anyway this conflicts with
  // the way routing is done to the scan chain links when connecting the mux
  // inputs later.
  //memories_.back()->circuit_instance()->Connect("Q",
  //    *circuit->GetOrAddSignal("CONFIG_OUT", 1));

  for (size_t i = 0; i < memories_.size() - 1; ++i) {
    geometry::Instance *source = memories_[i];
    geometry::Instance *sink = memories_[i + 1];

    LOG(INFO) << "Adding scan routes for pair "
              << source->name() << ", " << sink->name();

    std::vector<geometry::Port*> ports;
    source->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();

    ports.clear();
    sink->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();

    EquivalentNets net_names({end->net(), start->net()});
    (*memory_output_net_names)[source] = net_names.primary();

    circuit::Signal *signal = circuit->GetOrAddSignal(net_names.primary(), 1);

    source->circuit_instance()->Connect("Q", *signal);
    sink->circuit_instance()->Connect("D", *signal);

    geometry::ShapeCollection non_net_connectables;
    layout->CopyConnectableShapesNotOnNets(
        net_names,
        &non_net_connectables);

    auto result = routing_grid->AddRouteBetween(
        *start, *end, non_net_connectables, net_names);
    AccumulateAnyErrors(result.status());
  }
}

void LutB::RouteMuxInputs(
    RoutingGrid *routing_grid,
    Circuit *circuit,
    Layout *layout,
    std::map<geometry::Instance*, std::string> *memory_output_net_names) {
  // Connect mux substrates.
  circuit::Signal *VPWR = circuit->GetOrAddSignal("VPWR", 1);
  circuit::Signal *VGND = circuit->GetOrAddSignal("VGND", 1);
  for (size_t i = 0; i < mux_order_.size(); ++i) {
    mux_order_[i]->circuit_instance()->Connect("VPB", *VPWR);
    mux_order_[i]->circuit_instance()->Connect("VNB", *VGND);
  }

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
  //
  // FIXME(aryap): These depend on the way the banks are filled, so they depend
  // on the bank arrangement! But the basic principle is to find the nearest
  // memories, right? This is the TODO above actually...
  std::vector<AutoMemoryMuxConnection> auto_mem_connections = {
    {banks_[1].instances()[4][0], mux_order_[0], "input_4"},
    {banks_[1].instances()[2][0], mux_order_[0], "input_7"},
    {banks_[1].instances()[5][0], mux_order_[0], "input_5"},
    {banks_[1].instances()[3][0], mux_order_[0], "input_6"},

    {banks_[0].instances()[7][0], mux_order_[1], "input_5"},
    {banks_[0].instances()[5][0], mux_order_[1], "input_6"},
    {banks_[0].instances()[6][0], mux_order_[1], "input_4"},
    {banks_[0].instances()[4][0], mux_order_[1], "input_7"},

    {banks_[0].instances()[1][0], mux_order_[0], "input_0"},
    {banks_[0].instances()[3][0], mux_order_[0], "input_3"},
    {banks_[0].instances()[2][0], mux_order_[0], "input_2"},
    {banks_[0].instances()[0][0], mux_order_[0], "input_1"},

    {banks_[1].instances()[7][0], mux_order_[1], "input_0"},
    {banks_[1].instances()[8][0], mux_order_[1], "input_3"},
    {banks_[1].instances()[8][1], mux_order_[1], "input_2"},
    {banks_[1].instances()[6][0], mux_order_[1], "input_1"},
  };

  for (auto &auto_connection : auto_mem_connections) {
    geometry::Instance *memory = auto_connection.source_memory;
    geometry::Instance *mux = auto_connection.target_mux;
    const std::string &input_name = auto_connection.mux_port_name;

    // Heuristically determine which mux port to use based on which which is
    // closest to the memory output, even if we're routing to the memory output
    // net instead of the port specifically.
    std::vector<geometry::Port*> memory_ports;
    memory->GetInstancePorts("Q", &memory_ports);
    geometry::Port *memory_output = *memory_ports.begin();

    std::vector<geometry::Port*> mux_ports_on_net;
    mux->GetInstancePorts(input_name, &mux_ports_on_net);

    geometry::Port *mux_port = mux->GetNearestPortNamed(*memory_output,
                                                        input_name);
    if (!mux_port) {
      continue;
    }
    LOG_IF(
        FATAL,
        std::find(mux_ports_on_net.begin(), mux_ports_on_net.end(), mux_port) ==
            mux_ports_on_net.end())
        << "Nearest port named " << input_name
        << " did not appear in list of all ports for same name";

    // TODO(aryap): Why can't AddMultiPointRoute just replace this? Speed?
    bool path_found = false;
    while (mux_port) {
      EquivalentNets net_names = EquivalentNets(
          {memory_output->net(), mux_port->net()});
      geometry::ShapeCollection non_net_connectables;
      layout->CopyConnectableShapesNotOnNets(net_names, &non_net_connectables);
      LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                << " avoiding " << non_net_connectables.Describe();

      std::string target_net;
      absl::StatusOr<RoutingPath*> route_result;
      circuit::Signal *signal = nullptr;
      auto named_output_it = memory_output_net_names->find(memory);
      if (named_output_it == memory_output_net_names->end()) {
        target_net = net_names.primary();
        (*memory_output_net_names)[memory] = target_net;
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to " << memory->name();
        route_result = routing_grid->AddRouteBetween(
            *mux_port, *memory_output, non_net_connectables, net_names);

        signal = circuit->GetOrAddSignal(net_names.primary(), 1);
        if (route_result.ok()) {
          memory->circuit_instance()->Connect("Q", *signal);
        }
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
        route_result = routing_grid->AddRouteToNet(
            *mux_port, target_net, net_names, non_net_connectables);

        signal = circuit->GetOrAddSignal(target_net, 1);
      }
      if (route_result.ok()) {
        mux->circuit_instance()->Connect(input_name, *signal);
        LOG(INFO) << input_name << " <- " << signal->name();

        path_found = true;
        break;
      }
      mux_ports_on_net.erase(
          std::remove(mux_ports_on_net.begin(),
                      mux_ports_on_net.end(),
                      mux_port),
          mux_ports_on_net.end());
      mux_port = mux_ports_on_net.empty() ? nullptr : *mux_ports_on_net.begin();
    }
    if (!path_found) {
      AccumulateAnyErrors(absl::NotFoundError(absl::StrCat(
          "Could not route ", memory->name(), "/Q->",
          mux->name(), "/", input_name)));
    }
  }
}

void LutB::RouteRemainder(
    RoutingGrid *routing_grid,
    Circuit *circuit,
    Layout *layout) {
  // Connect the input buffers on the selector lines.
  //
  // +-----------+
  // |           |
  // |           |
  // |           |
  // +-----------+
  //
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
    }
  };

  for (const PortKeyCollection &collection : auto_connections) {
    auto result = AddMultiPointRoute(collection, routing_grid, circuit, layout);
    AccumulateAnyErrors(result.status());
  }

  // FIXME(aryap): Make circuit-only connections (this is fake).
  for (size_t i = 0; i < buf_order_.size(); ++i) {
    std::string port_name = absl::StrCat("S", i);
    circuit::Signal *signal = circuit->GetOrAddSignal(port_name, 1);

    buf_order_[i]->circuit_instance()->Connect("A", *signal);
  }

  // The hd_mux2 output needs to be connected to the output port.
  // (FIXME(aryap): This needs to be a a real pin.)
  circuit::Signal *output = circuit->GetOrAddSignal("Z", 1);
  active_mux2s_[0]->circuit_instance()->Connect("X", *output);

  // Create floating signals for unconnected ports.
  // FIXME(aryap): This can be handled automatically. It should be a function of
  // Circuits to make sure all instances have floating nets generated, if
  // needed.
  circuit::Signal *floating_signal = circuit->GetOrAddSignal("", 1);
  buf_order_[3]->circuit_instance()->Connect("P", *floating_signal);
}

void LutB::RouteInputs(
    RoutingGrid *routing_grid,
    Circuit *circuit,
    Layout *layout) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // buf input pin is on li.drawing, so we put a port on li.pin. This could be
  // handled automatically, since we already have a facility for finding a Via
  // stack between two layers.

  // Expect buffer inputs to be on li.drawing, identified by li.pin.
  const std::array<PortKeyAlias, 5> pin_map = {
    PortKeyAlias {{buf_order_[0], "port_A_centre"}, "S0"},
    PortKeyAlias {{buf_order_[1], "port_A_centre"}, "S1"},
    PortKeyAlias {{buf_order_[2], "port_A_centre"}, "S2"},
    PortKeyAlias {{buf_order_[3], "port_A_centre"}, "S3"},
    PortKeyAlias {{memories_.front(), "port_D_centre"}, "CONFIG_IN"}
  };

  layout->SetActiveLayerByName("li.pin");
  for (const auto &entry : pin_map) {
    const std::string &port_name = entry.alias;
    geometry::Point pin_centre =
        entry.key.instance->GetPointOrDie(entry.key.port_name);
    geometry::Rectangle *pin = layout->AddSquareAsPort(
        pin_centre,
        db.Rules("mcon.drawing").via_width,
        port_name);
    pin->set_net(port_name);
  }
}

void LutB::RouteOutputs(
    RoutingGrid *routing_grid,
    Circuit *circuit,
    Layout *layout) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const std::array<PortKeyAlias, 2> pin_map = {
    // Take the output from the final 2:1 mux output (for now).
    PortKeyAlias {{active_mux2s_[0], "port_X_centre_middle"}, "Z"},
    PortKeyAlias {{memories_.back(), "port_Q_centre"}, "CONFIG_OUT"}
  };
  layout->SetActiveLayerByName("li.pin");
  for (const auto &entry : pin_map) {
    const std::string &port_name = entry.alias;
    geometry::Point pin_centre =
        entry.key.instance->GetPointOrDie(entry.key.port_name);
    geometry::Rectangle *pin = layout->AddSquareAsPort(
        pin_centre,
        db.Rules("mcon.drawing").via_width,
        port_name);
    pin->set_net(port_name);
  }
}

// TODO(aryap): This clearly needs to be factored out of this class.
absl::StatusOr<std::vector<RoutingPath*>>
    LutB::AddMultiPointRoute(const PortKeyCollection &collection,
                             RoutingGrid *routing_grid,
                             Circuit *circuit,
                             Layout *layout) const {
  circuit::Signal *internal_signal = circuit->GetOrAddSignal(
      collection.net_name ? *collection.net_name : "", 1);
  std::string net = internal_signal->name();

  std::vector<std::vector<geometry::Port*>> route_targets;
  for (auto &port_key : collection.port_keys) {
    std::vector<geometry::Port*> &port_list =
        route_targets.emplace_back();
    geometry::Instance *instance = port_key.instance;

    circuit::Instance *circuit_instance = instance->circuit_instance();
    circuit_instance->Connect(port_key.port_name, *internal_signal);

    std::vector<geometry::Port*> matching_ports;
    instance->GetInstancePorts(port_key.port_name, &matching_ports);
    if (matching_ports.empty()) {
      LOG(WARNING) << "No port found named \"" << port_key.port_name
                   << "\" on instance \"" << instance->name();
      continue;
    }
    port_list.insert(
        port_list.end(), matching_ports.begin(), matching_ports.end());
  }

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
    RoutingGrid *routing_grid, Circuit *circuit, Layout *layout) const {
  struct StrapInfo {
    std::string port_name;
    std::string net_name;
    bool create_cross_bar_and_port;
  };
  static const std::array<StrapInfo, 4> kStrapInfo = {
    StrapInfo {
      .port_name = "VPWR",
      .net_name = "vpwr",
      .create_cross_bar_and_port = true
    },
    StrapInfo {
      .port_name = "VGND",
      .net_name = "vgnd",
      .create_cross_bar_and_port = true
    },
    StrapInfo {
      .port_name = "CLK",
      .net_name = "clk",
      .create_cross_bar_and_port = false
    },
    StrapInfo {
      .port_name = "CLKI",
      .net_name = "clk_i",
      .create_cross_bar_and_port = false
    }
  };

  // TODO(aryap): Merge into strap info above.
  static const std::array<std::string, 2> kCircuitOnlyPorts = {"VPB", "VNB"};
  static const std::array<std::string, 2> kCircuitOnlyPortNets = {
    "vpwr", "vgnd"};

  constexpr int64_t kOffsetNumPitches = 0;

  // FIXME(aryap): We are leaking technology-specific concerns into what was
  // previously somewhat agnostic; but was it ever really agnostic? There could
  // just be a strap configuration sction in the parameters:
  // TODO(aryap): What if we has a class SyntheticRules that created common
  // derivative rules from the base rule structs? Maybe users can define them
  // with std::functions (functors) in a standard form...
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &spine_via_rules = db.Rules("met2.drawing", "via1.drawing");
  const auto &spine_rules = db.Rules("met2.drawing");
  const auto &finger_via_rules = db.Rules("met1.drawing", "via1.drawing");
  const auto &finger_rules = db.Rules("met1.drawing");
  const auto &via_rules = db.Rules("via1.drawing");
  uint64_t via_side = std::max(via_rules.via_width, via_rules.via_height);
  uint64_t spine_bulge_width = 2 * spine_via_rules.via_overhang_wide + via_side;
  int64_t strap_pitch = std::max(
      spine_bulge_width + spine_rules.min_separation,
      via_side + 2 * finger_via_rules.via_overhang +
          finger_rules.min_separation);

  const LutB::LayoutConfig layout_config =
      *LutB::GetLayoutConfiguration(parameters_.lut_size);
  std::vector<geometry::Compass> strap_alignment_per_bank = {
      layout_config.left.strap_alignment,
      layout_config.right.strap_alignment};

  std::unordered_map<std::string, std::set<geometry::Polygon*>> spines;

  for (size_t bank = 0; bank < banks_.size(); ++bank) {
    for (size_t i = 0; i < kStrapInfo.size(); ++i) {
      const auto &strap_info = kStrapInfo[i];
      const std::string &port_name = strap_info.port_name;

      std::string net = absl::StrCat(strap_info.net_name, "_", bank);
      circuit::Wire wire = circuit->AddSignal(net);

      std::vector<geometry::Point> connections;
      for (const auto &row : banks_.at(bank).instances()) {
        for (geometry::Instance *instance : row) {
          LOG(INFO) << " instance " << instance->name();
          // We only care about the memories:
          if (std::find(memories_.begin(), memories_.end(), instance) ==
                  memories_.end()) {
            continue;
          }

          circuit::Instance *circuit_instance = instance->circuit_instance();

          std::vector<geometry::Port*> ports;
          instance->GetInstancePorts(port_name, &ports);

          for (geometry::Port *port : ports) {
            LOG(INFO) << " added " << *port;
            connections.push_back(port->centre());
          }

          // TODO(aryap): Do we need to disambiguate multiple ports with the
          // same name?
          if (circuit_instance) {
            circuit_instance->Connect(port_name, wire);
          }
        }
      }

      // Sort connections so that the left-most (lowest-x) is at the front.
      std::sort(connections.begin(), connections.end(),
                geometry::Point::CompareX);

      for (const auto &point : connections) {
        LOG(INFO) << "bank " << bank << " strap " << i << " has connection " << point;
      }

      std::optional<int64_t> spine_x;
      if (strap_alignment_per_bank[bank] == geometry::Compass::LEFT) {
        for (const geometry::Point &point : connections) {
          // Pick the left-most point.
          Utility::UpdateMin(point.x(), &spine_x);
        }
        *spine_x += kOffsetNumPitches * strap_pitch;
      } else if (strap_alignment_per_bank[bank] == geometry::Compass::RIGHT) {
        for (const geometry::Point &point : connections) {
          // Pick the right-most point.
          Utility::UpdateMax(point.x(), &spine_x);
        }
        *spine_x -= kOffsetNumPitches * strap_pitch;
      }

      geometry::Group new_shapes =
          AddVerticalSpineWithFingers("met2.drawing",
                                      "via1.drawing",
                                      "met1.drawing",
                                      net,
                                      connections,
                                      *spine_x,
                                      spine_bulge_width,
                                      layout);
      if (strap_info.create_cross_bar_and_port) {
        for (geometry::Polygon *polygon : new_shapes.polygons()) {
          if (polygon->layer() != db.GetLayer("met2.drawing"))
            continue;
          spines[net].insert(polygon);
        }
      }

      routing_grid->AddBlockages(new_shapes);
    }

    // Connect circuit-only ports.
    for (size_t i = 0; i < kCircuitOnlyPorts.size(); ++i) {
      std::string net = absl::StrCat(kCircuitOnlyPortNets[i], "_", bank);
      circuit::Signal *signal = circuit->GetOrAddSignal(net, 1);
      const std::string &port_name = kCircuitOnlyPorts[i];
      
      for (const auto &row : banks_.at(bank).instances()) {
        for (geometry::Instance *instance : row) {
          circuit::Instance *circuit_instance = instance->circuit_instance();
          if (!circuit_instance) {
            continue;
          }
          circuit_instance->Connect(port_name, *signal);
        }
      }
    }
  }

  // Find vertical range over which all spines are drawn:
  std::optional<int64_t> top_y;
  std::optional<int64_t> bottom_y;
  for (const auto &entry : spines) {
    for (geometry::Polygon *spine : entry.second) {
      geometry::Rectangle bounding_box = spine->GetBoundingBox();
      top_y = top_y ?
          std::min(*top_y, bounding_box.upper_right().y()) :
          bounding_box.upper_right().y();
      bottom_y = bottom_y ?
          std::max(*bottom_y, bounding_box.upper_right().y()) : 
          bounding_box.upper_right().y();
    }
  }

  // Now walk down from the top and assign pin locations:
  const auto &met3_rules = db.Rules("met3.drawing");
  //const auto &met3_via_rules = db.Rules("met3.drawing", "via2.drawing");

  if (top_y) {
    int64_t port_y = *top_y - met3_rules.min_pitch;
    layout->SetActiveLayerByName("met2.pin");
    for (const auto &entry : spines) {
      const std::string &net = entry.first;

      for (geometry::Polygon *spine : entry.second) {
        geometry::Rectangle *pin = layout->AddSquareAsPort(
            {spine->GetBoundingBox().centre().x(), port_y},
            db.Rules("via1.drawing").via_width,
            net);
        pin->set_net(net);
      }

      port_y -= met3_rules.min_pitch;
    }
  }
}

void LutB::AccumulateAnyErrors(const absl::Status &status) {
  if (status.ok()) {
    return;
  }
  errors_.push_back(status);
}

}  // namespace atoms
}  // namespace bfg
