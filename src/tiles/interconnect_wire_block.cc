#include "interconnect_wire_block.h"

#include <string>

#include <absl/strings/str_format.h>

#include "proto/parameters/interconnect_wire_block.pb.h"
#include "../routing_track_direction.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

namespace bfg {
namespace tiles {

void InterconnectWireBlock::Parameters::ToProto(
    proto::parameters::InterconnectWireBlock *pb) const {
  // TODO(aryap): Complete.
}

void InterconnectWireBlock::Parameters::FromProto(
    const proto::parameters::InterconnectWireBlock &pb) {
  // TODO(aryap): Complete.
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::TrackTriple::Make(
    const PhysicalPropertiesDatabase &db,
    const std::optional<int64_t> &forced_width_nm,
    const std::optional<int64_t> &forced_separation_nm,
    const std::optional<int64_t> &forced_offset_nm,
    const IntraLayerConstraints &rules) {
  return TrackTriple {
    .width = db.ToInternalUnits(forced_width_nm).value_or(rules.min_width),
    .separation =
        db.ToInternalUnits(forced_separation_nm).value_or(rules.min_separation),
    .offset = db.ToInternalUnits(forced_offset_nm).value_or(0)
  };
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::GetMainAxisTrackTriple() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &forced_width_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_width_nm : parameters_.horizontal_wire_width_nm;
  const auto &forced_separation_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_separation_nm :
    parameters_.horizontal_wire_separation_nm;
  const auto &forced_offset_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_offset_nm : parameters_.horizontal_wire_offset_nm;

  const auto &rules = db.Rules(main_layer_);
  return TrackTriple::Make(
      db, forced_width_nm, forced_separation_nm, forced_offset_nm, rules);
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::GetOffAxisTrackTriple() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &forced_width_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_width_nm : parameters_.vertical_wire_width_nm;
  const auto &forced_separation_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_separation_nm :
    parameters_.vertical_wire_separation_nm;
  const auto &forced_offset_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_offset_nm : parameters_.vertical_wire_offset_nm;

  const auto &rules = db.Rules(off_layer_);
  return TrackTriple::Make(
      db, forced_width_nm, forced_separation_nm, forced_offset_nm, rules);
}

geometry::Point InterconnectWireBlock::MakePoint(
    int64_t main_axis_pos, int64_t off_axis_pos) const {
  switch (parameters_.direction) {
    default:
      // Fallthrough intended.
    case RoutingTrackDirection::kTrackVertical:
      return geometry::Point(off_axis_pos, main_axis_pos);
    case RoutingTrackDirection::kTrackHorizontal:
      return geometry::Point(main_axis_pos, off_axis_pos);
  }
}

Cell *InterconnectWireBlock::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  TrackTriple main_axis = GetMainAxisTrackTriple();
  TrackTriple off_axis = GetOffAxisTrackTriple();

  int64_t main_axis_pos = main_axis.offset + main_axis.width / 2;
  int64_t off_axis_pos = off_axis.offset + off_axis.width / 2;

  ScopedLayer sl(cell->layout(), main_layer_);

  // Laying out a bundle is just drawing N wires in the right direction.
  for (size_t c = 0; c < parameters_.channels.size(); ++c) {
    const auto &channel = parameters_.channels[c];
    for (size_t i = 0; i < channel.num_bundles; ++i) {
      geometry::PolyLine line({
          MakePoint(main_axis_pos, off_axis_pos),
          MakePoint(main_axis_pos + parameters_.length, off_axis_pos)});
      line.SetWidth(main_axis.width);
      line.set_net(absl::StrFormat("channel_%d_index_%d", c, i));

      geometry::Polygon *wire = cell->layout()->AddPolyLine(line);

      // TODO(aryap): flip if configured:
      off_axis_pos += off_axis.width + off_axis.separation;
    }
  }

  // TODO(aryap): Add tiling bounds.

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
