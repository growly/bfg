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

InterconnectWireBlock::MappedParameters
InterconnectWireBlock::GetMainAxisMappedParameters() const {
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

  return MappedParameters {
    .width = db.ToInternalUnits(forced_width_nm),
    .pitch = db.ToInternalUnits(forced_pitch_nm),
    .offset = db.ToInternalUnits(forced_offset_nm)
  };
}

InterconnectWireBlock::MappedParameters
InterconnectWireBlock::GetOffAxisMappedParameters() const {
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

  return MappedParameters {
    .width = db.ToInternalUnits(forced_width_nm),
    .pitch = db.ToInternalUnits(forced_pitch_nm),
    .offset = db.ToInternalUnits(forced_offset_nm)
  };
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

// TODO(aryap): I factored these out so that you could flip the incremement
// direction depending on settings (like 'flip'), if desired. But in reality you
// can just flip the whole cell with the primitive Layout methods.
void InterconnectWireBlock::IncrementMainAxisPosition(
    int64_t *main_axis_pos, int64_t amount) const {
  *main_axis_pos += amount;
}
void InterconnectWireBlock::IncrementOffAxisPosition(
    int64_t *off_axis_pos, int64_t amount) const {
  *off_axis_pos += amount;
}

// TODO(aryap): This is similar to Layout::MakeWire or
// Layout::MakeAlternatingWire and should really be subsumed as a more
// general-purpose method there. 
void InterconnectWireBlock::DrawElbowWire(
    const geometry::Point start_of_main_axis_wire,
    const geometry::Point corner,
    const geometry::Point end_of_off_axis_wire,
    int64_t main_wire_width,
    int64_t off_wire_width,
    const std::string &net,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  {
    ScopedLayer sl(layout, main_layer_);
    auto via_encap = db.TypicalViaEncap(main_layer_, via_layer_);
    geometry::PolyLine main_line({start_of_main_axis_wire, corner});
    main_line.SetWidth(main_wire_width);
    main_line.set_net(net);
    main_line.InsertBulge(corner, via_encap.width, via_encap.length);
    // FIXME(aryap): Using the centre positions and setting an overhang did
    // not work!
    //main_line.set_overhang_end(overhang);
    geometry::Polygon *main_wire = layout->AddPolyLine(main_line);

    layout->MakeVia(via_layer_, corner, net);

    if (main_layer_pin_) {
      layout->MakePin(net, start_of_main_axis_wire, *main_layer_pin_);
    }
  }

  {
    ScopedLayer sl(layout, off_layer_);
    auto via_encap = db.TypicalViaEncap(off_layer_, via_layer_);
    geometry::PolyLine off_line({end_of_off_axis_wire, corner});
    off_line.SetWidth(off_wire_width);
    off_line.set_net(net);
    off_line.InsertBulge(corner, via_encap.width, via_encap.length);

    geometry::Polygon *off_wire = layout->AddPolyLine(off_line);

    if (off_layer_pin_) {
      layout->MakePin(net, end_of_off_axis_wire, *off_layer_pin_);
    }
  }
}

void InterconnectWireBlock::DrawStraightWire(
    int64_t off_axis_pos,
    int64_t length,
    int64_t width,
    const std::string &net,
    Layout *layout) const {
  ScopedLayer sl(layout, main_layer_);
  geometry::Point start_edge = MakePoint(0, off_axis_pos);
  geometry::Point end_edge = MakePoint(length, off_axis_pos);
  geometry::PolyLine line({start_edge, end_edge});
  line.SetWidth(width);
  line.set_net(net);

  if (main_layer_pin_) {
    layout->MakePin(net, start_edge, *main_layer_pin_);
    layout->MakePin(net, end_edge, *main_layer_pin_);
  }

  geometry::Polygon *wire = layout->AddPolyLine(line);
}

void InterconnectWireBlock::DrawBrokenOutWire(
    int64_t main_axis_pos,
    int64_t off_axis_pos,
    int64_t main_axis_gap,
    int64_t main_wire_width,
    int64_t off_wire_width,
    const std::string &net_0,
    const std::string &net_1,
    Layout *layout) const {
  geometry::Point start_edge = MakePoint(0, off_axis_pos);
  geometry::Point break_start = MakePoint(main_axis_pos, off_axis_pos);
  geometry::Point off_axis_pin_0 = MakePoint(main_axis_pos, 0);

  // Draw the first half, which could be incoming or outgoing.
  DrawElbowWire(start_edge,
                break_start,
                off_axis_pin_0,
                main_wire_width,
                off_wire_width,
                net_0,
                layout);

  geometry::Point break_end = 
      MakePoint(main_axis_pos + main_axis_gap, off_axis_pos);
  geometry::Point end_edge = MakePoint(parameters_.length, off_axis_pos);
  geometry::Point off_axis_pin_1 = MakePoint(main_axis_pos + main_axis_gap, 0);

  // Draw the second half, which could be outgoing or incoming.
  DrawElbowWire(end_edge,
                break_end,
                off_axis_pin_1,
                main_wire_width,
                off_wire_width,
                net_1,
                layout);
}

int64_t InterconnectWireBlock::GetMinMainAxisBreakoutGap(
    const MappedParameters &main_axis) const {
  uint64_t num_broken_out_wires = 0;
  for (const auto &channel : parameters_.channels) {
    num_broken_out_wires +=
        channel.break_out.size() * channel.bundle.num_wires;
  }

  return num_broken_out_wires * main_axis.pitch.value_or(0);
}

void InterconnectWireBlock::DrawConservatively(Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  MappedParameters main_axis = GetMainAxisMappedParameters();
  MappedParameters off_axis = GetOffAxisMappedParameters();

  // The defaults (and also bounds) for the track parameters in the conservative
  // case.
  //
  // The min pitch is actually dictated by how closely we can put the via encaps
  // together.
  //
  // TODO(aryap): If we assume that the off-axis wires will need to connect on
  // a smaller pitch, typically the PDK min pitch used for routing, and that the
  // main-axis wires must be separated to enable this. (This is much less area
  // efficient than the opposite case, but it's easier for routing, probably.
  // Also the main-axis wires are more likely to be thicker to cover longer
  // distances.)
  const auto &main_rules = db.Rules(main_layer_);
  auto main_via_encap = db.TypicalViaEncap(main_layer_, via_layer_);
  int64_t main_pitch = main_rules.min_separation +
      main_axis.width.value_or(
          std::max(main_via_encap.width, main_rules.min_width));
  main_axis.pitch = main_axis.pitch.value_or(main_pitch);

  const auto &off_rules = db.Rules(off_layer_);
  auto off_via_encap = db.TypicalViaEncap(off_layer_, via_layer_);
  int64_t off_pitch = off_rules.min_separation +
      off_axis.width.value_or(
          std::max(off_via_encap.width, off_rules.min_width));
  off_axis.pitch = off_axis.pitch.value_or(off_pitch);

  int64_t breakout_gap = GetMinMainAxisBreakoutGap(main_axis);

  // These values should now be set.
  int64_t main_width = main_axis.width.value_or(0);
  int64_t main_offset = main_axis.offset.value_or(0);
  int64_t off_width = off_axis.width.value_or(0);
  int64_t off_offset = off_axis.offset.value_or(0);

  int64_t main_axis_pos = main_offset + main_width / 2;
  int64_t off_axis_pos = off_offset + off_width / 2;

  // Laying out a bundle is just drawing N wires in the right direction.
  for (size_t c = 0; c < parameters_.channels.size(); ++c) {
    const auto &channel = parameters_.channels[c];
    for (size_t b = 0; b < channel.num_bundles; ++b) {
      // Wires that don't need to be broken out are the simple case:
      for (size_t w = 0; w < channel.bundle.num_wires; ++w) {
        std::string net_name = absl::StrFormat(
            "channel_%s_bundle_%d_wire_%d", channel.name, b, w);
        if (channel.break_out.find(b) == channel.break_out.end()) {
          DrawStraightWire(off_axis_pos,
                           parameters_.length,
                           main_width,
                           net_name,
                           layout);

          IncrementOffAxisPosition(&off_axis_pos, off_pitch);
          continue;
        }
        std::string net_name_0 = absl::StrCat(net_name, "_A");
        std::string net_name_1 = absl::StrCat(net_name, "_B");

        DrawBrokenOutWire(main_axis_pos,
                          off_axis_pos,
                          breakout_gap,
                          main_width,
                          off_width,
                          net_name_0,
                          net_name_1,
                          layout);

        IncrementOffAxisPosition(&off_axis_pos, off_pitch);
        IncrementMainAxisPosition(&main_axis_pos, main_pitch);
      }
    }
  }
}

void InterconnectWireBlock::DrawModestlyCleverlike(Layout *layout) const {
}

Cell *InterconnectWireBlock::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  switch (parameters_.layout_mode) {
    default:
      // Fallthrough intended.
    case Parameters::LayoutMode::kConservative:
      DrawConservatively(cell->layout());
      break;
    case Parameters::LayoutMode::kModestlyClever:
      DrawModestlyCleverlike(cell->layout());
      break;
  }

  //geometry::Point diagonal_corner = MakePoint(
  //    parameters_.length, off_axis_pos);
  //// TODO(aryap): Add tiling bounds.
  //geometry::Rectangle tiling_bounds({0, 0}, diagonal_corner);
  //{
  //  ScopedLayer sl(cell->layout(), "areaid.standardc");
  //  cell->layout()->AddRectangle(tiling_bounds);
  //}
  //cell->layout()->SetTilingBounds(tiling_bounds);

  //cell->layout()->FlipVertical();
  //cell->layout()->ResetOrigin();

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
