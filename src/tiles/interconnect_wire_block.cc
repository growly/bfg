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
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      pb->set_direction(
          proto::parameters::InterconnectWireBlock::TRACK_HORIZONTAL);
      break;
    case RoutingTrackDirection::kTrackVertical:
      pb->set_direction(
          proto::parameters::InterconnectWireBlock::TRACK_VERTICAL);
      break;
  }

  pb->set_grow_down(grow_down);
  pb->set_grow_left(grow_left);

  pb->set_horizontal_layer(horizontal_layer);
  pb->set_via_layer(via_layer);
  pb->set_vertical_layer(vertical_layer);

  if (horizontal_wire_width_nm) {
    pb->set_horizontal_wire_width_nm(*horizontal_wire_width_nm);
  }
  if (horizontal_wire_pitch_nm) {
    pb->set_horizontal_wire_pitch_nm(*horizontal_wire_pitch_nm);
  }
  if (horizontal_wire_offset_nm) {
    pb->set_horizontal_wire_offset_nm(*horizontal_wire_offset_nm);
  }
  if (vertical_wire_width_nm) {
    pb->set_vertical_wire_width_nm(*vertical_wire_width_nm);
  }
  if (vertical_wire_pitch_nm) {
    pb->set_vertical_wire_pitch_nm(*vertical_wire_pitch_nm);
  }
  if (vertical_wire_offset_nm) {
    pb->set_vertical_wire_offset_nm(*vertical_wire_offset_nm);
  }

  pb->set_length(length);

  for (const Channel &channel : channels) {
    auto *pb_channel = pb->add_channels();
    pb_channel->set_name(channel.name);
    for (int break_out_idx : channel.break_out) {
      pb_channel->add_break_out(break_out_idx);
    }
    pb_channel->set_num_bundles(channel.num_bundles);
    pb_channel->mutable_bundle()->set_num_wires(channel.bundle.num_wires);
  }
}

void InterconnectWireBlock::Parameters::FromProto(
    const proto::parameters::InterconnectWireBlock &pb) {
  if (pb.has_direction()) {
    switch (pb.direction()) {
      case proto::parameters::InterconnectWireBlock::TRACK_HORIZONTAL:
        direction = RoutingTrackDirection::kTrackHorizontal;
        break;
      case proto::parameters::InterconnectWireBlock::TRACK_VERTICAL:
        direction = RoutingTrackDirection::kTrackVertical;
        break;
      default:
        break;
    }
  }

  if (pb.has_grow_down()) {
    grow_down = pb.grow_down();
  }
  if (pb.has_grow_left()) {
    grow_left = pb.grow_left();
  }

  if (pb.has_horizontal_layer()) {
    horizontal_layer = pb.horizontal_layer();
  }
  if (pb.has_via_layer()) {
    via_layer = pb.via_layer();
  }
  if (pb.has_vertical_layer()) {
    vertical_layer = pb.vertical_layer();
  }

  if (pb.has_horizontal_wire_width_nm()) {
    horizontal_wire_width_nm = pb.horizontal_wire_width_nm();
  }
  if (pb.has_horizontal_wire_pitch_nm()) {
    horizontal_wire_pitch_nm = pb.horizontal_wire_pitch_nm();
  }
  if (pb.has_horizontal_wire_offset_nm()) {
    horizontal_wire_offset_nm = pb.horizontal_wire_offset_nm();
  }
  if (pb.has_vertical_wire_width_nm()) {
    vertical_wire_width_nm = pb.vertical_wire_width_nm();
  }
  if (pb.has_vertical_wire_pitch_nm()) {
    vertical_wire_pitch_nm = pb.vertical_wire_pitch_nm();
  }
  if (pb.has_vertical_wire_offset_nm()) {
    vertical_wire_offset_nm = pb.vertical_wire_offset_nm();
  }

  if (pb.has_length()) {
    length = pb.length();
  }

  channels.clear();
  for (const auto &pb_channel : pb.channels()) {
    Channel channel;
    if (pb_channel.has_name()) {
      channel.name = pb_channel.name();
    }
    for (int break_out_idx : pb_channel.break_out()) {
      channel.break_out.insert(break_out_idx);
    }
    if (pb_channel.has_num_bundles()) {
      channel.num_bundles = pb_channel.num_bundles();
    }
    if (pb_channel.has_bundle() && pb_channel.bundle().has_num_wires()) {
      channel.bundle.num_wires = pb_channel.bundle().num_wires();
    }
    channels.push_back(channel);
  }
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::MakeTrackTriple(
    const geometry::Layer &wire_layer,
    const std::optional<int64_t> forced_width_nm,
    const std::optional<int64_t> forced_pitch_nm,
    const std::optional<int64_t> forced_offset_nm) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &rules = db.Rules(wire_layer);
  // The min pitch is actually dictated by how closely we can put the via encaps
  // together.
  auto via_encap = db.TypicalViaEncap(wire_layer, via_layer_);
  int64_t min_pitch = rules.min_separation +
      db.ToInternalUnits(forced_width_nm).value_or(
          std::max(via_encap.width, rules.min_width));

  return TrackTriple {
    .width = db.ToInternalUnits(forced_width_nm).value_or(rules.min_width),
    .pitch = db.ToInternalUnits(forced_pitch_nm).value_or(min_pitch),
    .offset = db.ToInternalUnits(forced_offset_nm).value_or(0)
  };
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::GetMainAxisTrackTriple() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &forced_width_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_width_nm : parameters_.horizontal_wire_width_nm;
  const auto &forced_pitch_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_pitch_nm :
    parameters_.horizontal_wire_pitch_nm;
  const auto &forced_offset_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.vertical_wire_offset_nm : parameters_.horizontal_wire_offset_nm;

  return MakeTrackTriple(
      main_layer_, forced_width_nm, forced_pitch_nm, forced_offset_nm);
}

InterconnectWireBlock::TrackTriple
InterconnectWireBlock::GetOffAxisTrackTriple() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &forced_width_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_width_nm : parameters_.vertical_wire_width_nm;
  const auto &forced_pitch_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_pitch_nm :
    parameters_.vertical_wire_pitch_nm;
  const auto &forced_offset_nm =
    parameters_.direction == RoutingTrackDirection::kTrackVertical ?
    parameters_.horizontal_wire_offset_nm : parameters_.vertical_wire_offset_nm;

  return MakeTrackTriple(
      off_layer_, forced_width_nm, forced_pitch_nm, forced_offset_nm);
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

void InterconnectWireBlock::IncrementMainAxisPosition(
    int64_t *main_axis_pos, int64_t amount) const {
  *main_axis_pos += amount;
}
void InterconnectWireBlock::IncrementOffAxisPosition(
    int64_t *off_axis_pos, int64_t amount) const {
  *off_axis_pos += amount;
}

void InterconnectWireBlock::DrawStraightThroughBundle(
    const Parameters::Channel &channel,
    const TrackTriple &main_axis,
    const TrackTriple &off_axis,
    size_t bundle_number,
    int64_t *off_axis_pos,
    Layout *layout) const {
  ScopedLayer sl(layout, main_layer_);
  for (size_t w = 0; w < channel.bundle.num_wires; ++w) {
    std::string net_name = absl::StrFormat(
        "channel_%s_bundle_%d_wire_%d", channel.name, bundle_number, w);
    geometry::Point start_edge = MakePoint(0, *off_axis_pos);
    geometry::Point end_edge = MakePoint(parameters_.length, *off_axis_pos);
    geometry::PolyLine line({start_edge, end_edge});
    line.SetWidth(main_axis.width);
    line.set_net(net_name);

    if (main_layer_pin_) {
      layout->MakePin(net_name, start_edge, *main_layer_pin_);
      layout->MakePin(net_name, end_edge, *main_layer_pin_);
    }

    geometry::Polygon *wire = layout->AddPolyLine(line);

    // TODO(aryap): flip if configured:
    IncrementOffAxisPosition(off_axis_pos, off_axis.pitch);
  }
}

// The main_axis_pos is the coordinate along the main axis, e.g. for the
// vertical orientation, it is the y coordinate. The off_axis_pos is the other
// coordinate, e.g. x in the same case.
void InterconnectWireBlock::DrawBrokenOutBundle(
    const Parameters::Channel &channel,
    const TrackTriple &main_axis,
    const TrackTriple &off_axis,
    int64_t breakout_gap,
    size_t bundle_number,
    int64_t *main_axis_pos,
    int64_t *off_axis_pos,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  for (size_t w = 0; w < channel.bundle.num_wires; ++w) {
    std::string net_name_0 = absl::StrFormat(
        "channel_%s_bundle_%d_wire_%d_A", channel.name, bundle_number, w);
    std::string net_name_1 = absl::StrFormat(
        "channel_%s_bundle_%d_wire_%d_B", channel.name, bundle_number, w);

    {
      auto via_encap = db.TypicalViaEncap(main_layer_, via_layer_);
      ScopedLayer sl(layout, main_layer_);
      geometry::Point break_start =
          MakePoint(*main_axis_pos, *off_axis_pos);
      geometry::Point start_edge = MakePoint(0, *off_axis_pos);
      geometry::PolyLine main_line_0({start_edge, break_start});
      main_line_0.SetWidth(main_axis.width);
      main_line_0.set_net(net_name_0);
      main_line_0.InsertBulge(break_start, via_encap.width, via_encap.length);
      // FIXME(aryap): Using the centre positions and setting an overhang did
      // not work!
      //main_line_0.set_overhang_end(overhang);
      geometry::Polygon *main_wire_0 = layout->AddPolyLine(main_line_0);

      layout->MakeVia(via_layer_, break_start, net_name_0);

      geometry::Point break_end = 
          MakePoint(*main_axis_pos + breakout_gap, *off_axis_pos);
      geometry::Point end_edge = MakePoint(parameters_.length, *off_axis_pos);
      geometry::PolyLine main_line_1({break_end, end_edge});
      main_line_1.SetWidth(main_axis.width);
      main_line_1.set_net(net_name_1);
      main_line_1.InsertBulge(break_end, via_encap.width, via_encap.length);

      geometry::Polygon *main_wire_1 = layout->AddPolyLine(main_line_1);

      layout->MakeVia(via_layer_, break_end, net_name_1);

      if (main_layer_pin_) {
        layout->MakePin(net_name_0, start_edge, *main_layer_pin_);
        layout->MakePin(net_name_1, end_edge, *main_layer_pin_);
      }
    }

    {
      auto via_encap = db.TypicalViaEncap(off_layer_, via_layer_);
      ScopedLayer sl(layout, off_layer_);
      geometry::Point edge_0 = MakePoint(*main_axis_pos, 0);
      geometry::Point end = MakePoint(*main_axis_pos, *off_axis_pos);
      geometry::PolyLine off_line_0({edge_0, end});
      off_line_0.SetWidth(off_axis.width);
      off_line_0.set_net(net_name_0);
      off_line_0.InsertBulge(end, via_encap.width, via_encap.length);

      geometry::Polygon *off_wire_0 = layout->AddPolyLine(off_line_0);

      end = MakePoint(*main_axis_pos + breakout_gap, *off_axis_pos);
      geometry::Point edge_1 = MakePoint(*main_axis_pos + breakout_gap, 0);
      geometry::PolyLine off_line_1({edge_1, end});
      off_line_1.SetWidth(off_axis.width);
      off_line_1.set_net(net_name_1);
      off_line_1.InsertBulge(end, via_encap.width, via_encap.length);

      geometry::Polygon *off_wire_1 = layout->AddPolyLine(off_line_1);

      if (off_layer_pin_) {
        layout->MakePin(net_name_0, edge_0, *off_layer_pin_);
        layout->MakePin(net_name_1, edge_1, *off_layer_pin_);
      }
    }

    IncrementOffAxisPosition(off_axis_pos, off_axis.pitch);
    IncrementMainAxisPosition(main_axis_pos, main_axis.pitch);
  }
}

int64_t InterconnectWireBlock::GetMinMainAxisBreakoutGap(
    const TrackTriple &main_axis) const {
  uint64_t num_broken_out_wires = 0;
  for (const auto &channel : parameters_.channels) {
    num_broken_out_wires +=
        channel.break_out.size() * channel.bundle.num_wires;
  }

  return num_broken_out_wires * main_axis.pitch;
}

Cell *InterconnectWireBlock::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  TrackTriple main_axis = GetMainAxisTrackTriple();
  TrackTriple off_axis = GetOffAxisTrackTriple();

  int64_t breakout_gap = GetMinMainAxisBreakoutGap(main_axis);

  int64_t main_axis_pos = main_axis.offset + main_axis.width / 2;
  int64_t off_axis_pos = off_axis.offset + off_axis.width / 2;

  // Laying out a bundle is just drawing N wires in the right direction.
  for (size_t c = 0; c < parameters_.channels.size(); ++c) {
    const auto &channel = parameters_.channels[c];
    for (size_t b = 0; b < channel.num_bundles; ++b) {
      // Wires that don't need to be broken out are the simple case:
      if (channel.break_out.find(b) == channel.break_out.end()) {
        DrawStraightThroughBundle(
            channel, main_axis, off_axis, b, &off_axis_pos, cell->layout());
      } else {
        DrawBrokenOutBundle(
            channel, main_axis, off_axis, breakout_gap, b, &main_axis_pos,
            &off_axis_pos, cell->layout());
      }
    }
  }

  geometry::Point diagonal_corner = MakePoint(
      parameters_.length, off_axis_pos);
  // TODO(aryap): Add tiling bounds.
  geometry::Rectangle tiling_bounds({0, 0}, diagonal_corner);
  {
    ScopedLayer sl(cell->layout(), "areaid.standardc");
    cell->layout()->AddRectangle(tiling_bounds);
  }
  cell->layout()->SetTilingBounds(tiling_bounds);

  //cell->layout()->FlipVertical();
  //cell->layout()->ResetOrigin();

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
