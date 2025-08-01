#include "interconnect.h"

#include <string>

#include <absl/strings/str_format.h>
#include "../atoms/sky130_interconnect_mux6.h"
#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "../routing_layer_info.h"
#include "../routing_via_info.h"
#include "proto/parameters/interconnect.pb.h"

namespace bfg {
namespace tiles {

void Interconnect::Parameters::ToProto(
    proto::parameters::Interconnect *pb) const {
}

void Interconnect::Parameters::FromProto(
    const proto::parameters::Interconnect &pb) {
}

Cell *Interconnect::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  size_t num_cols = 16;
  size_t num_rows = 8;

  std::vector<std::vector<geometry::Instance*>> muxes;

  // TODO(aryap): Ok this is clearly a more useful structure than just a
  // "Memory" bank. Rename it. "TilingGrid"? idk.
  MemoryBank bank = MemoryBank(cell->layout(),
                               design_db_,
                               nullptr,   // No tap cells.
                               false,      // Rotate alternate rows.
                               false,      // Rotate first row.
                               geometry::Compass::LEFT);

  atoms::Sky130InterconnectMux6::Parameters default_mux6_params;

  for (size_t i = 0; i < num_rows; ++i) {
    auto &mux_row = muxes.emplace_back();
    for (size_t j = 0; j < num_cols; ++j) {
      std::string name = absl::StrFormat("interconnect_mux6_r%u_c%u", i, j);
      atoms::Sky130InterconnectMux6::Parameters row_params =
          default_mux6_params;
      atoms::Sky130InterconnectMux6 mux_gen(row_params, design_db_);
      Cell *mux6_template = mux_gen.GenerateIntoDatabase(name);
      geometry::Instance *instance = bank.InstantiateRight(
          i, absl::StrCat(name, "_i"), mux6_template->layout());
      mux_row.push_back(instance);
    }
  }

  Route(muxes, cell->layout());

  return cell.release();
}

// FIXME(aryap): This needs to be generalised ASAP. Surely most of this is like
// a default of the PDK?
void Interconnect::ConfigureRoutingGrid(
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

  routing_grid->AddGlobalNet("CLK");
}

void Interconnect::Route(
    const std::vector<std::vector<geometry::Instance*>> muxes,
    Layout *layout) {
  RoutingGrid routing_grid(design_db_->physical_db());
  ConfigureRoutingGrid(&routing_grid, layout);

  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);
}

}   // namespace tiles
}   // namespace bfg
