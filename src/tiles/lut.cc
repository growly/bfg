#include "lut.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>

#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_mux.h"

namespace bfg {
namespace tiles {

bfg::Cell *Lut::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Cell> lut_cell(new bfg::Cell("lut"));
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  constexpr size_t kNumRows = 4;
  constexpr size_t kNumColumns = 4;

  // Let's assume N = 16 for now.
  std::vector<geometry::Instance*> flip_flops;
  for (size_t j = 0; j < kNumRows; j++) {
    for (size_t i = 0; i < kNumColumns; i++) {
      std::string instance_name = absl::StrFormat("lut_dfxtp_%d_%d", i, j);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      bfg::atoms::Sky130Dfxtp::Parameters params;
      bfg::atoms::Sky130Dfxtp generator(params, design_db_);
      bfg::Cell *cell = generator.Generate();
      cell->set_name(cell_name);
      //cell->layout()->ResetOrigin();
      design_db_->ConsumeCell(cell);
      circuit->AddInstance(instance_name, cell->circuit());
      geometry::Rectangle bounding_box = cell->layout()->GetTilingBounds();
      int64_t height = static_cast<int64_t>(bounding_box.Height());
      int64_t width = static_cast<int64_t>(bounding_box.Width());
      // For every other row, place backwards from the end.
      int64_t x_pos = (j % 2 != 0 ? kNumColumns - 1 - i : i) * width;
      int64_t y_pos = j * height;
      LOG(INFO) << "placing " << instance_name;
      geometry::Instance geo_instance(
          cell->layout(), geometry::Point { x_pos, y_pos });
      geo_instance.set_name(instance_name);
      if (j % 2 != 0) {
        geo_instance.set_rotation_degrees_ccw(180);
        geo_instance.Translate(geometry::Point(width, height));
      }
      geometry::Instance *installed = layout->AddInstance(geo_instance);
      flip_flops.push_back(installed);
    }
  }

  geometry::Rectangle ff_bounds = layout->GetBoundingBox();

  int64_t x_pos = static_cast<int64_t>(ff_bounds.Width());
  int64_t y_pos = static_cast<int64_t>(ff_bounds.Height());

  bfg::atoms::Sky130Mux::Parameters mux_params;
  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *mux_cell = mux.GenerateIntoDatabase("mux_template");

  std::vector<geometry::Instance*> mux_order;
  circuit->AddInstance("mux_0", mux_cell->circuit());
  {
    geometry::Instance geo_instance(
        mux_cell->layout(), geometry::Point { x_pos + 500, 0 });
    geo_instance.set_name("mux_0");
    geometry::Instance *instance = layout->AddInstance(geo_instance);
    mux_order.push_back(instance);
  }

  circuit->AddInstance("mux_1", mux_cell->circuit());
  {
    geometry::Instance geo_instance(
        mux_cell->layout(),
        geometry::Point {
            x_pos + 500,
            static_cast<int64_t>(mux_cell->layout()->GetBoundingBox().Height())
        });
    geo_instance.set_name("mux_1");
    geometry::Instance *instance = layout->AddInstance(geo_instance);
    mux_order.push_back(instance);
  }

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();

  // Set every property the RoutingGrid needs. TODO(aryap): Probably need a
  // simpler interface to this.
  bfg::RoutingLayerInfo vertical_routing;
  vertical_routing.layer = db.GetLayer("met1.drawing");
  const IntraLayerConstraints vertical_rules = db.Rules("met2.drawing");
  vertical_routing.area = pre_route_bounds;
  vertical_routing.wire_width = vertical_rules.min_width;
  vertical_routing.offset = 50;
  vertical_routing.pitch = vertical_rules.min_pitch;
  vertical_routing.direction = bfg::RoutingTrackDirection::kTrackVertical;

  bfg::RoutingLayerInfo horizontal_routing;
  horizontal_routing.layer = db.GetLayer("met2.drawing");
  const IntraLayerConstraints horizontal_rules =
      db.Rules("met2.drawing");
  horizontal_routing.area = pre_route_bounds;
  horizontal_routing.wire_width = horizontal_rules.min_width;
  horizontal_routing.offset = 50;
  horizontal_routing.pitch = horizontal_rules.min_pitch;
  horizontal_routing.direction = bfg::RoutingTrackDirection::kTrackHorizontal;

  bfg::RoutingViaInfo routing_via_info;
  routing_via_info.layer = db.GetLayer("mcon.drawing");
  const IntraLayerConstraints via_rules = db.Rules("mcon.drawing");
  routing_via_info.cost = 1.0;
  routing_via_info.width = via_rules.via_width;
  routing_via_info.height = via_rules.via_width;
  routing_via_info.overhang = db.Rules(
      "mcon.drawing", "met2.drawing").via_overhang;

  RoutingGrid routing_grid(db);
  routing_grid.AddRoutingLayerInfo(vertical_routing);
  routing_grid.AddRoutingLayerInfo(horizontal_routing);
  routing_grid.AddRoutingViaInfo(
      vertical_routing.layer, horizontal_routing.layer, routing_via_info);
  routing_grid.ConnectLayers(vertical_routing.layer, horizontal_routing.layer);

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->GetShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
    routing_grid.AddBlockages(shapes);
    //LOG(INFO) << shapes.Describe();
  }

  // Connect the scan chain.
  for (size_t i = 0; i < flip_flops.size(); ++i) {
    geometry::Instance *instance = flip_flops[i];
    LOG(INFO) << "adding routes for flip flop " << i;
    if (i == flip_flops.size() - 1)
      continue;
    geometry::Instance *next_in_chain = flip_flops[i+1];
    std::set<geometry::Port*> ports;
    instance->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();
    // HACK HACK HACK
    start->set_layer(db.GetLayer("met1.drawing"));
    ports.clear();
    next_in_chain->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();
    // HACK HACK HACK
    end->set_layer(db.GetLayer("met1.drawing"));
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
    for (size_t k = 0; k < 8 && j < flip_flops.size(); ++k) {
      std::set<geometry::Port*> ff_ports;
      geometry::Instance *ff = flip_flops[j];
      ff->GetInstancePorts("D", &ff_ports);
      geometry::Port *start = *ff_ports.begin();

      std::set<geometry::Port*> ports;
      size_t mux_index = j % mux_order.size();
      geometry::Instance *mux = mux_order[mux_index];
      mux->GetInstancePorts(mux_input_order[k], &ports);

      for (geometry::Port *port : ports) {
        LOG(INFO) << "adding routes for ff " << j << " to mux " << mux_index;
        // HACK HACK HACK
        start->set_layer(db.GetLayer("met1.drawing"));
        port->set_layer(db.GetLayer("met1.drawing"));
        routing_grid.AddRouteBetween(*start, *port);
      }
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
