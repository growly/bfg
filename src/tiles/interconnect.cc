#include "interconnect.h"

#include <string>
#include <unordered_map>

#include <absl/strings/str_format.h>
#include "../equivalent_nets.h"
#include "../atoms/sky130_interconnect_mux6.h"
#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "../routing_blockage_cache.h"
#include "../routing_layer_info.h"
#include "../routing_via_info.h"
#include "proto/parameters/interconnect.pb.h"

namespace bfg {
namespace tiles {

void Interconnect::Parameters::ToProto(
    proto::parameters::Interconnect *pb) const {
  pb->set_num_rows(num_rows);
  pb->set_num_columns(num_columns);
}

void Interconnect::Parameters::FromProto(
    const proto::parameters::Interconnect &pb) {
  if (pb.has_num_rows()) {
    num_rows = pb.num_rows();
  }
  if (pb.has_num_columns()) {
    num_columns = pb.num_columns();
  }
}

Cell *Interconnect::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  muxes_.clear();

  // TODO(aryap): Ok this is clearly a more useful structure than just a
  // "Memory" bank. Rename it. "TilingGrid"? idk.
  MemoryBank bank = MemoryBank(cell->layout(),
                               design_db_,
                               nullptr,    // No tap cells.
                               false,      // Rotate alternate rows.
                               false,      // Rotate first row.
                               geometry::Compass::LEFT);

  atoms::Sky130InterconnectMux6::Parameters default_mux6_params {
    .horizontal_pitch_nm = 340
  };

  for (size_t i = 0; i < parameters_.num_rows; ++i) {
    auto &mux_row = muxes_.emplace_back();
    for (size_t j = 0; j < parameters_.num_columns; ++j) {
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

  InputPortCollection mux_inputs;
  OutputPortCollection mux_outputs;
  for (size_t i = 0; i < parameters_.num_rows; ++i) {
    mux_inputs.emplace_back();
    mux_outputs.emplace_back();
    for (size_t j = 0; j < parameters_.num_columns; ++j) {
      geometry::Instance *mux = muxes_[i][j];

      // FIXME(aryap): The number of output ports is absolutely a parameter
      // here! Or at least it must be!
      {
        geometry::Port *port = mux->GetFirstPortNamed(
            atoms::Sky130InterconnectMux6::kMuxOutputName);
        if (!port) {
          LOG(WARNING) << "No output port on " << mux;
          continue;
        }
        mux_outputs[i].push_back(port);
      }

      mux_inputs[i].emplace_back();
      for (size_t k = 0; k < default_mux6_params.num_inputs; ++k) {
        std::string port_name = absl::StrFormat("X%u", k);
        geometry::Port *port = mux->GetFirstPortNamed(port_name);
        if (!port) {
          LOG(WARNING) << "No such port " << port_name << " on " << mux;
          continue;
        }
        mux_inputs[i][j].push_back(port);
      }
    }
  }

  // TODO(aryap): Maybe it should be a feature of the MemoryBank to merge its
  // tiling bounds with whatever TilingBounds are in the layout (though then we
  // will have to distinguish set and unset tiling bounds instead of the
  // default, which is to return the bounding box).
  cell->layout()->SetTilingBounds(*bank.GetTilingBounds());

  RouteComplete(mux_inputs, mux_outputs, cell->layout());

  return cell.release();
}

// FIXME(aryap): This needs to be generalised ASAP. Surely most of this is like
// a default of the PDK?
void Interconnect::ConfigureRoutingGrid(
    RoutingGrid *routing_grid, Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &met1_rules = db.Rules("met1.drawing");
  const auto &met2_rules = db.Rules("met2.drawing");

  // Use linear-cost model (saves memory).
  //routing_grid->set_use_linear_cost_model(true);

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();
  geometry::Rectangle tiling_bounds = layout->GetTilingBounds();

  // The muxes are configured to be mutliples of met1 pitch in height, and are
  // aligned with typical sky130 std cells having met1 pitches offset by a
  // half-pitch from the bottom boundary:
  //
  // ----------------------------------------------------------------------
  //
  // -  -  -  -  -  -  -  -  -  -   tiling bounds   -  -  -  -  -  -  -  -  -  -
  //                ^   VPWR rail
  // ---------------|------------------------------------------------------
  //                |
  //                |  1.5 met1 pitches
  //                v
  //                +  <--- centre of met1 pin within cell
  //
  int64_t vertical_offset = met1_rules.min_pitch / 2 +
      (tiling_bounds.lower_left().y() - pre_route_bounds.lower_left().y());
  // JOKES! How bout we align with the output ports?
  //int64_t vertical_offset = 11300 % met1_rules.min_pitch +
  //    (tiling_bounds.lower_left().y() - pre_route_bounds.lower_left().y());

  int64_t horizontal_offset = //met2_rules.min_pitch / 2 +
      (tiling_bounds.lower_left().x() - pre_route_bounds.lower_left().x());

  LOG(INFO) << "Pre-routing bounds: " << pre_route_bounds;
  LOG(INFO) << "Tiling bounds: " << tiling_bounds;
  RoutingLayerInfo met1_layer_info =
      db.GetRoutingLayerInfoOrDie("met1.drawing");
  met1_layer_info.set_direction(RoutingTrackDirection::kTrackHorizontal);
  met1_layer_info.set_area(pre_route_bounds);
  met1_layer_info.set_offset(vertical_offset);

  RoutingLayerInfo met2_layer_info =
      db.GetRoutingLayerInfoOrDie("met2.drawing");
  met2_layer_info.set_direction(RoutingTrackDirection::kTrackVertical);
  met2_layer_info.set_area(pre_route_bounds);
  met2_layer_info.set_offset(horizontal_offset);

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

  // TODO(aryap): Figure out where to define/manage/collect power/ground nets.
  // This is important because it prevents prefixes on nets in child instances
  // with matching names:
  layout->AddGlobalNet("VPWR");
  layout->AddGlobalNet("VGND");
  layout->AddGlobalNet("CLK");
  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met1.drawing"), &shapes, 1);
    routing_grid->AddBlockages(shapes);
  }
  {
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met2.drawing"), &shapes, 1);
    routing_grid->AddBlockages(shapes);
  }
  // FIXME(aryap): So we need to add the met1.drawing CLK/CLKI bars in each
  // flip flop as blockages. These are connectable so are not included above.
  // They are named so we could select them that way. We could remove their
  // "connectability" when we're done routing to them, such as when they are
  // added to the Sky130InterconnectMux6 (currently best option). We could also
  // add a "search" for shapes matching nets, or at a given depth in the
  // hierarchy. These seem brittle.
  //{
  //  geometry::ShapeCollection shapes;
  //  layout->CopyConnectableShapes(&shapes, std::nullopt);
  //  EquivalentNets rails(layout->global_nets());
  //  shapes.KeepOnlyNets(rails);
  //  routing_grid->AddBlockages(shapes);
  //}
}

void Interconnect::RouteComplete(
    const InputPortCollection &mux_inputs,
    const OutputPortCollection &mux_outputs,
    Layout *layout) {
  RoutingGrid routing_grid(
      design_db_->physical_db());
  ConfigureRoutingGrid(&routing_grid, layout);

  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);

  // All of the different port net names attached to the same driver need to be
  // merged.
  std::map<geometry::Port*, EquivalentNets> nets;

  std::vector<std::vector<size_t>> next_free_input =
      std::vector<std::vector<size_t>>(
          parameters_.num_rows, std::vector<size_t>(parameters_.num_columns));

  std::map<std::string, std::map<std::string, std::string>> statuses;

  size_t num_muxes = parameters_.num_rows * parameters_.num_columns;
  for (size_t i = 0; i < num_muxes; ++i) {
    size_t source_row = (i / parameters_.num_columns) % parameters_.num_rows;
    size_t source_col = i % parameters_.num_columns;

    geometry::Instance *source = muxes_[source_row][source_col];
    // Only one output per mux right now.
    geometry::Port *from = mux_outputs[source_row][source_col];

    for (size_t j = 0; j < num_muxes; ++j) {
      size_t dest_row = (j / parameters_.num_columns) % parameters_.num_rows;
      size_t dest_col = j % parameters_.num_columns;

      geometry::Instance *destination = muxes_[dest_row][dest_col];

      // TODO(aryap): Have to find unused inputs:
      size_t &input_index = next_free_input[dest_row][dest_col];
      geometry::Port *to = mux_inputs[dest_row][dest_col][input_index];
      // Only connect up to 6 inputs per destination.
      input_index = input_index + 1;
      if (input_index > 5) {
        continue;
      }

      if (nets.find(from) != nets.end()) {
        EquivalentNets targets = nets[from];
        nets[from].Add(to->net());
        EquivalentNets &usable = nets[from];

        EquivalentNets ok_nets = nets[from];
        ok_nets.Add(layout->global_nets());
        geometry::ShapeCollection non_net_connectables;
        layout->CopyConnectableShapesNotOnNets(
            ok_nets, &non_net_connectables);

        auto status = routing_grid.AddRouteToNet(*to,
                                                 targets,
                                                 usable,
                                                 non_net_connectables);
        //LOG_IF(WARNING, !status.ok())
        //    << "Could not connect " << to->Describe() << " to any of "
        //    << targets;
        statuses[from->Describe()][to->Describe()] = absl::StrCat(
            status.status().ToString(), " nets: ", usable.Describe());
      } else {
        EquivalentNets usable({from->net(), to->net()});
        nets[from] = usable;

        EquivalentNets ok_nets = usable;
        ok_nets.Add(layout->global_nets());
        geometry::ShapeCollection non_net_connectables;
        layout->CopyConnectableShapesNotOnNets(
            ok_nets, &non_net_connectables);

        auto status = routing_grid.AddRouteBetween(*from,
                                                   *to,
                                                   non_net_connectables,
                                                   usable);

        //LOG_IF(WARNING, !status.ok())
        //    << "Could not connect " << from->Describe() << " to "
        //    << to->Describe();
        statuses[from->Describe()][to->Describe()] = absl::StrCat(
            status.status().ToString(), " nets: ", usable.Describe());
      }
    }
  }

  LOG(INFO) << "Route summary:";
  for (const auto &outer : statuses) {
    for (const auto &inner : outer.second) {
      const auto &status = inner.second;
      LOG(INFO) << outer.first << " -> " << inner.first << ": "
                << status;
    }
  }

  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);

  routing_grid.ExportToLayout("routing", layout);
}

void Interconnect::Route(
    const InputPortCollection &mux_inputs,
    const OutputPortCollection &mux_outputs,
    Layout *layout) {
  RoutingGrid routing_grid(
      design_db_->physical_db());
  ConfigureRoutingGrid(&routing_grid, layout);

  // All of the different port net names attached to the same driver need to be
  // merged.
  std::map<geometry::Port*, EquivalentNets> nets;

  //// What if just added 20 routes?
  //for (size_t i = 0; i < 20; ++i) {
  //  geometry::Instance *source = muxes[i / 4][i % 6];
  //  geometry::Port *from = mux_outputs[i / 4][i % 16];
  //  geometry::Instance *destination = muxes[i / 4 + 1][(i + 1) % 16];
  //  geometry::Port *to = mux_inputs[i / 4 + 1][(i + 1) % 16][0];
  //  routing_grid.AddRouteBetween(
  //      *from, *to,
  //      {},
  //      EquivalentNets({from->net(), to->net()}));
  //}
  size_t num_muxes = parameters_.num_rows * parameters_.num_columns;
  for (size_t i = 0; i < num_muxes; ++i) {
    size_t source_row = (i / parameters_.num_columns) % parameters_.num_rows;
    size_t source_col = i % parameters_.num_columns;

    geometry::Instance *source = muxes_[source_row][source_col];
    // Only one output per mux right now.
    geometry::Port *from = mux_outputs[source_row][source_col];

    for (size_t j = i + 1, count = 0; count < 6; j += 4, ++count) {
      size_t dest_row = (j / parameters_.num_columns) % parameters_.num_rows;
      size_t dest_col = j % parameters_.num_columns;

      if (source_row == dest_row && source_col == dest_col)
        continue;

      geometry::Instance *destination = muxes_[dest_row][dest_col];

      // TODO(aryap): Have to find unused inputs:
      geometry::Port *to = mux_inputs[dest_row][dest_col][j % 6];

      if (nets.find(from) != nets.end()) {
        EquivalentNets targets = nets[from];
        nets[from].Add(to->net());
        EquivalentNets &usable = nets[from];

        EquivalentNets ok_nets = nets[from];
        ok_nets.Add(layout->global_nets());
        geometry::ShapeCollection non_net_connectables;
        layout->CopyConnectableShapesNotOnNets(
            ok_nets, &non_net_connectables);

        auto status = routing_grid.AddRouteToNet(*to,
                                                 targets,
                                                 usable,
                                                 non_net_connectables);
        LOG_IF(WARNING, !status.ok())
            << "Could not connect " << to->Describe() << " to any of "
            << targets;
        if (!status.ok()) {

        }
      } else {
        EquivalentNets usable({from->net(), to->net()});
        nets[from] = usable;

        EquivalentNets ok_nets = usable;
        ok_nets.Add(layout->global_nets());
        geometry::ShapeCollection non_net_connectables;
        layout->CopyConnectableShapesNotOnNets(
            ok_nets, &non_net_connectables);

        auto status = routing_grid.AddRouteBetween(*from,
                                                   *to,
                                                    non_net_connectables,
                                                    usable);
        LOG_IF(WARNING, !status.ok())
            << "Could not connect " << from->Describe() << " to "
            << to->Describe();
      }
    }
  }

  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout);
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout);

  routing_grid.ExportToLayout("routing", layout);
}

}   // namespace tiles
}   // namespace bfg
