#ifndef TILES_INTERCONNECT_WIRE_BLOCK_H_
#define TILES_INTERCONNECT_WIRE_BLOCK_H_

#include <numeric>
#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../geometry/point.h"
#include "../layout.h"
#include "proto/parameters/interconnect_wire_block.pb.h"
#include "tile.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

// The block is either vertical or horizontal. It has separate horizontal and
// vertical routing directions. Wires are grouped together in bundles, and
// bundles are grouped together in channels. There can be multiple channels.
//
// For each channel, a given bundle (by index) can be broken out in the
// orthogonal routing direction to the edge of the block.
//
// e.g. A channel with bundles of length n, breaking out bundle 1:
// x = 0            OUT          IN
// +-----------------|-----------|------------------------------------+
// | 0      N        |           |                                    |
// ---------/--------|-----------|-------------------------------------
// | 1      N        |           |                                    |
// ---------/--------+           +-------------------------------------
// | 2      N                                                         |
// ---------/----------------------------------------------------------
// |                                                                  |
// +------------------------------------------------------------------+
//
// (Note also the naming convention here: the break out closest to the start of
// the block is named OUT, and the break out closest to the end is named IN.
//
// The naive way to arrange these wires requires extra spacing whenever there is
// a wire encap. This requires greater wire spacing:
//                              +----+
//                            +--------+
// ---------------------------+        |
// ---------------------------+        |
//                   +----+   +--------+
//                 +--------+   ++  ++
// ----------------+        |    |  |
// ----------------+        |    |  |
//        +----+   +--------+    |  |
//      +--------+   ++  ++      |  |
// -----+        |    |  |       |  |
// -----+        |    |  |       |  |
//      +--------+    |  |       |  |
//        ++  ++      |  |       |  |
//         |  |       |  |       |  |
//
//
// There might be a way to make this more efficient. Permutation might be
// possible if there are a great many wires, but for a small number it does not
// avoid conflicts of geometrically close neighbours:
//                              +----+
//                            +--------+
// ---------------------------+        |
// ---------------------------+        |
//                            +--------+
//        +----+                ++  ++
//      +--------+               |  |
// -----+        |               |  |
// -----+        |               |  |
//      +--------+               |  |
//        ++  ++     +----+      |  |
//         |  |    +--------+    |  |
// ----------------+        |    |  |
// ----------------+        |    |  |
//         |  |    +--------+    |  |
//         |  |      ++  ++      |  |
// 
// Put another way, there's no way to arrange these connections in a 3x3 grid
// such that there are no direct neighbours. But this would certainly allow you
// to mix pitches so that some wires can be closer together.
//
// I don't think that complexity buys us anything at this point, though.
//
// OH! An actually-useful way to make this work is to interleave bundle wires
// with one another, so that neighbouring wires will never be broken out in the
// same place. This will only fail if there is only one bundle in the channel.
// An extra-step of cleverness is to include same-index wires from all channels
// so that even the single-bundle case is handled. This is implemented as the
// "modestly clever" layout mode.
//
// TODO(aryap): Ideas for how this should work. Add a 'break_out_A_offset' and a
// 'break_out_B_offset' that are signed indicating the offset from the start (if
// positive) or end (if negative) of the length of the block that the respective
// breakouts should start being laid out. If both are not specified, use the
// default breakout gap calculation. If the user asks for them to overlap,
// that's on them.
class InterconnectWireBlock : public Tile {
 public:
  struct Parameters {
    enum class LayoutMode : uint8_t {
      kConservative = 0,
      kModestlyClever = 1
    };

    struct Break {
      // Default side (0) or the other (span of off-axis).
      bool alternate_side;

      // Offset along main axis.
      std::optional<int64_t> offset;
    };

    struct Bundle {
      int num_wires;

      // If true, the wires in this bundled are broken out (or in) with
      // orthogonal wires running to the parallel edge of the block. If false,
      // the wires in the bundle are drawn straight through from the start of
      // the block to the end.
      bool tap;

      // If tap is true and break_out is specified, the starting wires in
      // the block are drawn and broken out according to the details in the
      // Break structure. If tap is true and break_out is missing, the
      // starting wires are not drawn.
      std::optional<Break> break_out;

      // If tap is true and break_in is specified, the ending wires in the
      // block are drawn and broken out according to the details in the Break
      // structure. If tap is true and break_in is missing, the ending
      // wires are not drawn.
      std::optional<Break> break_in;
    };

    struct Channel {
      std::string name;
      std::vector<Bundle> bundles;

      size_t NumTaps() const {
        size_t count = 0;
        for (const Bundle &bundle : bundles) {
          if (bundle.tap) {
            count += bundle.num_wires;
          }
        }
        return count;
      }

      size_t NumWires() const {
        size_t count = 0;
        for (const Bundle &bundle : bundles) {
          count += bundle.num_wires;
        }
        return count;
      }
    };

    LayoutMode layout_mode = LayoutMode::kModestlyClever;

    RoutingTrackDirection direction = RoutingTrackDirection::kTrackVertical;

    // You could get the same effect by flipping and translating all of the
    // shapes in the generated layout, but these might be easier to use for a
    // designer.
    // TODO(aryap): Implement these. Do they even make any sense to include? I
    // don't know. If you don't need the annoyingly indirect Increment...()
    // functions remove those too.

    // Not sure if this can be automatically deduced, since other PDKs will
    // have multiple horizontal/vertical layers anyway.
    std::string horizontal_layer = "met1.drawing";
    std::string via_layer = "via1.drawing";
    std::string vertical_layer = "met2.drawing";

    // Read these as "the width of the horizontal wire", "the pitch of the
    // vertical wire", etc. They dictate sizing in the dimension orthogonal to
    // the direction of the wire.
    std::optional<int64_t> horizontal_wire_width_nm;
    std::optional<int64_t> horizontal_wire_pitch_nm;
    std::optional<int64_t> horizontal_wire_offset_nm;
    std::optional<int64_t> vertical_wire_width_nm;
    std::optional<int64_t> vertical_wire_pitch_nm;
    std::optional<int64_t> vertical_wire_offset_nm;

    // The length of the block is either its height or its width depending on
    // whether the routing is vertical or horizontal (respectively).
    uint64_t length = 30000;

    std::optional<int64_t> first_break_out_start_nm;
    std::optional<int64_t> second_break_out_start_nm;

    std::vector<Channel> channels = {
      Channel {
        .name = "6X",
        //.break_out = {0},
        //.num_bundles = 6,
        //.bundle = Bundle {
        //  .num_wires = 4
        //}
      },
      Channel {
        .name = "2X",
        //.break_out = {0},
        //.num_bundles = 2,
        //.bundle = Bundle {
        //  .num_wires = 4
        //}
      }
    };

    size_t NumTaps() const {
      // I like this more than a for-loop that increases a counter. It feels
      // like it has bigger cojones.
      return std::accumulate(
          channels.begin(), channels.end(), 0U,
          [](size_t sum, const Channel &channel) {
            return sum + channel.NumTaps();
          });
    }

    size_t NumWires() const {
      return std::accumulate(
          channels.begin(), channels.end(), 0U,
          [](size_t sum, const Channel &channel) {
            return sum + channel.NumWires();
          });
    }

    void ToProto(proto::parameters::InterconnectWireBlock *pb) const;
    void FromProto(const proto::parameters::InterconnectWireBlock &pb); 
  };

  static uint64_t PredictWidth(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);
  static uint64_t PredictHeight(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);
  static uint64_t PredictPitchOfOffAxis(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);

  static const geometry::Layer ResolveMainLayer(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters) {
    return parameters.direction == RoutingTrackDirection::kTrackVertical ?
        db.GetLayer(parameters.vertical_layer) :
        db.GetLayer(parameters.horizontal_layer);
  }
  static const geometry::Layer ResolveOffLayer(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters) {
    return parameters.direction == RoutingTrackDirection::kTrackVertical ?
        db.GetLayer(parameters.horizontal_layer) :
        db.GetLayer(parameters.vertical_layer);
  }
  static const geometry::Layer ResolveViaLayer(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters) {
    return db.GetLayer(parameters.via_layer);
  }


  InterconnectWireBlock(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters),
        main_layer_(ResolveMainLayer(design_db->physical_db(), parameters)),
        off_layer_(ResolveOffLayer(design_db->physical_db(), parameters)),
        via_layer_(ResolveViaLayer(design_db->physical_db(), parameters)) {
    auto main_layer_pins =
        design_db->physical_db().GetPinLayersFor(main_layer_);
    if (!main_layer_pins.empty()) {
      main_layer_pin_ = *main_layer_pins.begin();
    }
    auto off_layer_pins =
        design_db->physical_db().GetPinLayersFor(off_layer_);
    if (!off_layer_pins.empty()) {
      off_layer_pin_ = *off_layer_pins.begin();
    }
  }

  static std::string MakeNetName(
      const std::string &channel_name,
      int bundle_number,
      int wire_number,
      std::optional<bool> first_end_of_breakout = std::nullopt);

  Cell *GenerateIntoDatabase(const std::string &name) override;

 private:
  // These are the user-specified width, pitch and offset parameters, in
  // internal units, from the Parameters struct. If they were not specified they
  // are still std::nullopt here.
  struct MappedParameters {
    std::optional<int64_t> width;
    std::optional<int64_t> pitch;
    std::optional<int64_t> offset;
  };

  // These are the internal-unit-valued, non-optional values of the
  // MappedParameters with default values applied if necessary.
  struct ResolvedParameters {
    int64_t width;
    int64_t pitch;
    int64_t offset;
  };

  struct WireIndex {
    std::string channel_name;
    size_t channel_number;
    size_t bundle_number;
    size_t wire_number;
    const Parameters::Bundle &bundle;
    int64_t pos_on_off_axis;
    // If present, this wire is broken out:
    std::optional<int64_t> pos_on_main_axis_out;
    // If present, this wire is broken in:
    std::optional<int64_t> pos_on_main_axis_in;
    // If present, the broken-out pin is shifted to this off-axis position:
    std::optional<int64_t> off_axis_edge_pos;
  };

  static uint64_t PredictSpanAlongOffAxis(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);
  static MappedParameters GetMainAxisMappedParameters(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);
  static MappedParameters GetOffAxisMappedParameters(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters);

  // Compute the minimum distance between the incoming and outgoing wire when
  // the bundle is being broken out. This measurement is between the centres of
  // the two wires on either side of the gap, so inclues 1x wire width.
  static int64_t GetMinBreakoutGap(
      const Parameters &parameters,
      int64_t off_axis_pitch);

  static void ResolveParameters(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters,
      ResolvedParameters *main_axis,
      ResolvedParameters *off_axis);

  static void ResolveParametersConservative(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters,
      ResolvedParameters *main_axis,
      ResolvedParameters *off_axis);

  static void ResolveParametersModestlyClever(
      const PhysicalPropertiesDatabase &db,
      const Parameters &parameters,
      ResolvedParameters *main_axis,
      ResolvedParameters *off_axis);

  static std::optional<int64_t> GetBreakEdgePosition(
      const std::optional<Parameters::Break> &break_spec,
      int64_t default_case,
      int64_t alternate_case);

  MappedParameters GetMainAxisMappedParameters() const {
    return GetMainAxisMappedParameters(design_db_->physical_db(), parameters_);
  }
  MappedParameters GetOffAxisMappedParameters() const {
    return GetOffAxisMappedParameters(design_db_->physical_db(), parameters_);
  }

  geometry::Point MapToPoint(
      int64_t pos_on_main_axis, int64_t pos_on_off_axis) const;

  void DrawElbowWire(
    const geometry::Point start_of_main_axis_wire,
    const geometry::Point corner,
    const geometry::Point end_of_off_axis_wire,
    int64_t main_wire_width,
    int64_t off_wire_width,
    const std::string &net,
    Layout *layout) const;

  // The pos_on_main_axis is the coordinate along the main axis, e.g. for the
  // vertical orientation, it is the y coordinate. The pos_on_off_axis is the
  // other coordinate, e.g. x in the same case.
  //
  // off_axis_edge_pos, is given, determines the position of the off-axis wire
  // pin, i.e. the point to which the off-axis wire is broken out.
  void DrawBrokenOutWire(
    int64_t pos_on_off_axis,
    int64_t pos_on_main_axis_out,
    int64_t pos_on_main_axis_in,
    int64_t main_wire_width,
    int64_t off_wire_width,
    const std::string &net_0,
    const std::string &net_1,
    std::optional<int64_t> off_axis_edge_pos_out,
    std::optional<int64_t> off_axis_edge_pos_in,
    Layout *layout) const;

  void DrawStraightWire(
          int64_t pos_on_off_axis,
      int64_t length,
      int64_t width,
      const std::string &net,
      Layout *layout) const;

  void DrawConservative(
      geometry::Point *diagonal_corner, Layout *layout) const;

  // The modestly clever scheme distributes wires in groups of similar indices,
  // so that the 0-index wires from all bundles across all channels are routed
  // together, and the 1-index wires are all routed together, and so on. If
  // there is only 1 bundle per channel, the scheme is the same as in the
  // conservative case. This enables tighter pitches since same-index wires from
  // different bundles are never broken out near each other.
  void DrawModestlyClever(
      geometry::Point *diagonal_corner, Layout *layout) const;

  // TODO(aryap): Why don't I keep more state in the generator itself? It can
  // always be reset when Generate() is called again... these function
  // signatures are nuts!
  void IncrementPositionOnMainAxis(
      int64_t *pos_on_main_axis, int64_t amount) const;
  void IncrementPositionOnOffAxis(
      int64_t *pos_on_off_axis, int64_t amount) const;

  Parameters parameters_;

  const geometry::Layer main_layer_;
  const geometry::Layer via_layer_;
  const geometry::Layer off_layer_;
  std::optional<geometry::Layer> main_layer_pin_;
  std::optional<geometry::Layer> off_layer_pin_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_WIRE_BLOCK_H_
