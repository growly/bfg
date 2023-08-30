#include "lut.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>

#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../atoms/sky130_buf.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_mux.h"

namespace bfg {
namespace tiles {

const Lut::LayoutConfig *Lut::GetLayoutConfiguration(size_t lut_size) {
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

bfg::Cell *Lut::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Cell> lut_cell(new bfg::Cell("lut"));
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  const Lut::LayoutConfig layout_config =
      *Lut::GetLayoutConfiguration(lut_size_); 
  constexpr int64_t kMuxSize = 8;

  int64_t buf_y_pos = 0;

  std::vector<geometry::Instance*> flip_flops;
  std::vector<std::unique_ptr<bfg::Layout>> bank_layouts;
  int64_t max_row_height = 0;
  for (size_t b = 0; b < layout_config.num_banks; ++b) {
    bfg::Layout *bank_layout = new bfg::Layout(db);
    bank_layouts.emplace_back(bank_layout);
    int64_t y_pos = 0;
    for (size_t j = 0; j < layout_config.bank_rows; j++) {
      size_t row_width = 0;
      for (size_t i = 0; i < layout_config.bank_columns; i++) {
        std::string instance_name = absl::StrFormat(
            "lut_dfxtp_%d_%d_%d", b, i, j);
        std::string cell_name = absl::StrCat(instance_name, "_template");
        bfg::atoms::Sky130Dfxtp::Parameters params;
        bfg::atoms::Sky130Dfxtp generator(params, design_db_);
        bfg::Cell *cell = generator.Generate();
        cell->set_name(cell_name);
        //cell->layout()->ResetOrigin();
        design_db_->ConsumeCell(cell);
        geometry::Rectangle bounding_box = cell->layout()->GetTilingBounds();
        int64_t height = static_cast<int64_t>(bounding_box.Height());
        int64_t width = static_cast<int64_t>(bounding_box.Width());
        row_width += width;
        max_row_height = std::max(max_row_height, height);
        // For every other row, place backwards from the end.
        int64_t x_pos = (
            j % 2 != 0 ? layout_config.bank_columns - 1 - i : i) * width;
        y_pos = j * height;
        LOG(INFO) << "placing " << instance_name;
        geometry::Instance geo_instance(
            cell->layout(), geometry::Point { x_pos, y_pos });
        geo_instance.set_name(instance_name);
        if (j % 2 != 0) {
          geo_instance.set_rotation_degrees_ccw(180);
          geo_instance.Translate(geometry::Point(width, height));
        }
        geometry::Instance *installed = bank_layout->AddInstance(geo_instance);
        flip_flops.push_back(installed);
      }
      bank_layout->SavePoint(absl::StrCat("row_", j, "_lr"),
                             geometry::Point(row_width, y_pos));
    }
  }

  LOG_IF(FATAL, bank_layouts.size() < 1)
      << "Expected at least 1 bank by this point.";

  layout->AddLayout(*bank_layouts[0], "bank_0");
  geometry::Rectangle left_bounds = layout->GetBoundingBox();

  bfg::atoms::Sky130Mux::Parameters mux_params;
  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *mux_cell = mux.GenerateIntoDatabase("mux_template");

  std::vector<geometry::Instance*> mux_order;

  {
    size_t num_muxes = (1 << lut_size_) / kMuxSize;

    // Muxes are positioned like so:
    //
    // | 4-LUT | 5-LUT | 6-LUT
    //                 
    // |   x   |   x   |   x x
    // | x     | x     | x     x
    // |       |   x   |   x x
    // |       | x     | x     x
    //
    // The number of columns is defined in the LayoutConfig struct in
    // kLayoutConfigurations. Here we must compute the position based on where
    // they are in this chain.
    size_t column_select = 0;
    int64_t column_spacing = 300;
    int64_t row_spacing = -500;
    int64_t mux_height = mux_cell->layout()->GetBoundingBox().Height();
    int64_t mux_width = mux_cell->layout()->GetBoundingBox().Width();
    int64_t effective_mux_height = mux_height + row_spacing;
    int64_t effective_mux_width = mux_width + column_spacing;

    int64_t x_pos = static_cast<int64_t>(
        left_bounds.Width()) + layout_config.mux_area_padding;
    int64_t y_offset = -(mux_height - 2 * max_row_height) / 2;
    int64_t y_pos = y_offset;

    size_t p = 0;
    for (size_t i = 0; i < num_muxes; ++i) {
      std::string mux_name = absl::StrCat("mux_", i);
      circuit->AddInstance(mux_name, mux_cell->circuit());
      int64_t x_shift = column_select * effective_mux_width;
      geometry::Instance geo_instance(
          mux_cell->layout(), geometry::Point {
              x_pos + static_cast<int64_t>(column_select * effective_mux_width),
              y_pos
          });
      geo_instance.set_name(mux_name);
      geometry::Instance *instance = layout->AddInstance(geo_instance);
      mux_order.push_back(instance);
      if (p < layout_config.mux_area_rows - 1) {
        y_pos += effective_mux_height;
        ++p;
      } else {
        column_select += 1;   // Alternate the alternation.
        x_pos += 2 * (effective_mux_width);
        y_pos = y_offset;
        p = 0;
      }

      // Alternates between 0 and 1.
      column_select = (column_select + 1) % 2;
    }
  }

  {
    // NOTE(aryap): Can only gracefully deal with two banks.
    int64_t x_pos =
        layout->GetBoundingBox().Width() + layout_config.mux_area_padding;
    for (size_t i = 1; i < bank_layouts.size(); ++i) {
      bank_layouts[i]->MoveTo(geometry::Point(x_pos, 0));
      layout->AddLayout(*bank_layouts[i], absl::StrCat("bank_", i));
      x_pos += bank_layouts[i - 1]->GetBoundingBox().Width();
    }
  }

  {
    // Add input buffers.
    int64_t buf_width = 0;
    geometry::Point start_position = layout->GetPoint("bank_0.row_3_lr");
    for (size_t i = 0; i < 4; ++i) {
      std::string instance_name = absl::StrFormat("buf_%d", i);
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
      geometry::Instance *instance = layout->AddInstance(
          geometry::Instance (
              buf_cell->layout(),
              start_position + geometry::Point(buf_width, 0)
              //geometry::Point {
              //-200, -200
              //}
          )
      );
      geometry::Rectangle bounding_box = buf_cell->layout()->GetTilingBounds();
      buf_width += bounding_box.Width();
    }
  }

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();

  RoutingGrid routing_grid(db);

  // Set every property the RoutingGrid needs.
  // TODO(aryap): Probably need a simpler interface to this.
  bfg::RoutingLayerInfo vertical_routing;
  vertical_routing.layer = db.GetLayer("met1.drawing");
  const IntraLayerConstraints vertical_rules = db.Rules("met2.drawing");
  vertical_routing.direction = bfg::RoutingTrackDirection::kTrackVertical;
  vertical_routing.area = pre_route_bounds;
  vertical_routing.wire_width = vertical_rules.min_width;
  vertical_routing.offset = 50;
  vertical_routing.pitch = vertical_rules.min_pitch;

  bfg::RoutingLayerInfo horizontal_routing;
  horizontal_routing.layer = db.GetLayer("met2.drawing");
  const IntraLayerConstraints horizontal_rules =
      db.Rules("met2.drawing");
  horizontal_routing.direction = bfg::RoutingTrackDirection::kTrackHorizontal;
  horizontal_routing.area = pre_route_bounds;
  horizontal_routing.wire_width = horizontal_rules.min_width;
  horizontal_routing.offset = 50;
  horizontal_routing.pitch = horizontal_rules.min_pitch;

  // TODO(aryap): Store connectivity information (which layers connect through
  // which vias) in the PhysicalPropertiesDatabase's via_layers_.
  bfg::RoutingViaInfo routing_via_info;
  //routing_via_info.layer = db.GetViaLayer("met1.drawing", "met2.drawing");
  routing_via_info.layer = db.GetLayer("via1.drawing");
  IntraLayerConstraints via_rules = db.Rules("via1.drawing");
  routing_via_info.cost = 0.5;
  routing_via_info.width = via_rules.via_width;
  routing_via_info.height = via_rules.via_width;
  routing_via_info.overhang_length = db.Rules(
      "via1.drawing", "met2.drawing").via_overhang;
  routing_via_info.overhang_width = db.Rules(
      "via1.drawing", "met2.drawing").via_overhang_wide;
  routing_grid.AddRoutingViaInfo(
      vertical_routing.layer, horizontal_routing.layer, routing_via_info);

  routing_via_info.layer = db.GetLayer("mcon.drawing");
  via_rules = db.Rules("mcon.drawing");
  routing_via_info.cost = 0.5;
  routing_via_info.width = via_rules.via_width;
  routing_via_info.height = via_rules.via_width;
  routing_via_info.overhang_length = db.Rules(
      "mcon.drawing", "met1.drawing").via_overhang;
  routing_via_info.overhang_width = db.Rules(
      "mcon.drawing", "met1.drawing").via_overhang_wide;
  routing_grid.AddRoutingViaInfo(
      vertical_routing.layer, db.GetLayer("li.drawing"), routing_via_info);

  routing_via_info.layer = db.GetLayer("via2.drawing");
  via_rules = db.Rules("via2.drawing");
  routing_via_info.cost = 0.5;
  routing_via_info.width = via_rules.via_width;
  routing_via_info.height = via_rules.via_width;
  routing_via_info.overhang_length = db.Rules(
      "via2.drawing", "met3.drawing").via_overhang;
  routing_via_info.overhang_width = db.Rules(
      "via2.drawing", "met3.drawing").via_overhang_wide;
  routing_grid.AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), horizontal_routing.layer, routing_via_info);

  routing_grid.AddRoutingLayerInfo(vertical_routing);
  routing_grid.AddRoutingLayerInfo(horizontal_routing);
  routing_grid.ConnectLayers(vertical_routing.layer, horizontal_routing.layer);

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->GetShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
    LOG(INFO) << "met1 shapes: \n" << shapes.Describe();
    routing_grid.AddBlockages(shapes, db.Rules("met1.drawing").min_separation);
  }
  {
    geometry::ShapeCollection shapes;
    layout->GetShapesOnLayer(db.GetLayer("met2.drawing"), &shapes);
    LOG(INFO) << "met2 shapes: \n" << shapes.Describe();
    routing_grid.AddBlockages(shapes, db.Rules("met2.drawing").min_separation);
  }

  // Connect the scan chain.
  for (size_t i = 0; i < flip_flops.size(); ++i) {
    geometry::Instance *instance = flip_flops[i];
    LOG(INFO) << "Adding routes for flip flop " << i;
    if (i == flip_flops.size() - 1)
      continue;
    geometry::Instance *next_in_chain = flip_flops[i+1];
    std::set<geometry::Port*> ports;
    instance->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();
    ports.clear();
    next_in_chain->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();
    //routing_grid.AddRouteBetween(*start, *end);
    LOG(INFO) << i << " start port: " << *start << " end: " << *end;
  }

  // Connect flip-flops to mux.
  std::vector<std::string> mux_input_order = {
    "input_1",
    "input_0",
    "input_2",
    "input_3",
    "input_7",
    "input_6",
    "input_4",
    "input_5",
  };
  size_t j = 0;
  while (j < flip_flops.size()) {
    for (size_t k = 0; k < kMuxSize && j < flip_flops.size(); ++k) {
      std::set<geometry::Port*> ff_ports;
      geometry::Instance *ff = flip_flops[j];
      ff->GetInstancePorts("D", &ff_ports);
      geometry::Port *start = *ff_ports.begin();

      std::set<geometry::Port*> ports;
      size_t mux_index = j / kMuxSize;
      geometry::Instance *mux = mux_order[mux_index];
      mux->GetInstancePorts(mux_input_order[k], &ports);

      if (ports.empty())
        continue;

      std::string net_name = absl::StrCat("net_", j, "_", k);

      geometry::Port *first_port = *ports.begin();
      LOG(INFO) << "Adding routes for (ff, mux) = (" << j << ", " << mux_index << ")";
      routing_grid.AddRouteBetween(*start, *first_port, net_name);

      //auto it = ports.begin();
      //it++;  // Skip first port.
      //while (it != ports.end()) {
      //  geometry::Port *port = *it;
      //  LOG(INFO) << "Adding routes for (ff, mux) = (" << j << ", "
      //            << mux_index << ")" << " to net " << net_name;
      //  // HACK HACK HACK
      //  start->set_layer(db.GetLayer("met1.drawing"));
      //  port->set_layer(db.GetLayer("met1.drawing"));
      //  routing_grid.AddRouteToNet(*port, net_name);
      //  it++;
      //}
      j++;
    }
  }

  std::unique_ptr<bfg::Layout> grid_layout(routing_grid.GenerateLayout());
  layout->AddLayout(*grid_layout, "routing");

  lut_cell->SetLayout(layout.release());
  lut_cell->SetCircuit(circuit.release());
  bfg::Cell *cell = lut_cell.release();
  cell->set_name(name);
  design_db_->ConsumeCell(cell);
  return cell;
}

}  // namespace atoms
}  // namespace bfg
