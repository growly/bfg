#ifndef TILES_INTERCONNECT_WIRE_BLOCK_H_
#define TILES_INTERCONNECT_WIRE_BLOCK_H_

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
// TODO(aryap): 
//
//
// e.g. A channel with bundles of length n, breaking out bundle 1:
//                  IN/OUT      OUT/IN
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
class InterconnectWireBlock : public Tile {
 public:
  struct Parameters {
    enum class LayoutMode {
      kConservative = 0,
      kModestlyClever = 1
    };

    struct Bundle {
      int num_wires;
    };

    struct Channel {
      std::string name;
      std::set<int> break_out;
      int num_bundles;
      Bundle bundle;
    };

    LayoutMode layout_mode = LayoutMode::kModestlyClever;

    RoutingTrackDirection direction = RoutingTrackDirection::kTrackVertical;

    // You could get the same effect by flipping and translating all of the
    // shapes in the generated layout, but these might be easier to use for a
    // designer.
    // TODO(aryap): Implement these. Do they even make any sense to include? I
    // don't know. If you don't need the annoyingly indirect Increment...()
    // functions remove those too.
    bool grow_down = false;
    bool grow_left = true;

    // Not sure if this can be automatically deduced, since other PDKs will
    // have multiple horizontal/vertical layers anyway.
    std::string horizontal_layer = "met1.drawing";
    std::string via_layer = "via1.drawing";
    std::string vertical_layer = "met2.drawing";

    std::optional<int64_t> horizontal_wire_width_nm;
    std::optional<int64_t> horizontal_wire_pitch_nm;
    std::optional<int64_t> horizontal_wire_offset_nm = 340;
    std::optional<int64_t> vertical_wire_width_nm;
    std::optional<int64_t> vertical_wire_pitch_nm;
    std::optional<int64_t> vertical_wire_offset_nm = 340;

    // The length of the block is either its height or its width depending on
    // whether the routing is vertical or horizontal (respectively).
    uint64_t length = 30000;

    std::vector<Channel> channels = {
      Channel {
        .name = "6X",
        .break_out = {0},
        .num_bundles = 6,
        .bundle = Bundle {
          .num_wires = 4
        }
      },
      Channel {
        .name = "2X",
        .break_out = {0},
        .num_bundles = 2,
        .bundle = Bundle {
          .num_wires = 4
        }
      }
    };

    void ToProto(proto::parameters::InterconnectWireBlock *pb) const;
    void FromProto(const proto::parameters::InterconnectWireBlock &pb); 
  };

  InterconnectWireBlock(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters),
        main_layer_(
            parameters_.direction == RoutingTrackDirection::kTrackVertical ?
            design_db->physical_db().GetLayer(parameters.vertical_layer) :
            design_db->physical_db().GetLayer(parameters.horizontal_layer)),
        off_layer_(
            parameters_.direction == RoutingTrackDirection::kTrackVertical ?
            design_db->physical_db().GetLayer(parameters.horizontal_layer) :
            design_db->physical_db().GetLayer(parameters.vertical_layer)),
        via_layer_(
            design_db->physical_db().GetLayer(parameters.via_layer)) {
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

  Cell *GenerateIntoDatabase(const std::string &name) override;

  //void Route(
  //    const std::vector<std::vector<geometry::Instance*>> muxes,
  //    Layout *layout);

 private:
  // These are the user-specified width, pitch and offset parameters, in
  // internal units, from the Parameters struct. If they were not specified they
  // are still std::nullopt here.
  struct MappedParameters {
    std::optional<int64_t> width;
    std::optional<int64_t> pitch;
    std::optional<int64_t> offset;
  };

  struct WireIndex {
    size_t channel_number;
    size_t bundle_number;
    size_t wire_number;
    std::string net;
    int64_t pos_on_off_axis;
    // If present, this wire is broken out:
    std::optional<int64_t> pos_on_main_axis;
    // If present, the broken-out pin is shifted to this off-axis position:
    std::optional<int64_t> off_axis_edge_pos;
  };

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
    int64_t pos_on_main_axis,
    int64_t pos_on_off_axis,
    int64_t main_axis_gap,
    int64_t main_wire_width,
    int64_t off_wire_width,
    const std::string &net_0,
    const std::string &net_1,
    std::optional<int64_t> off_axis_edge_pos,
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
  void IncrementMainAxisPosition(int64_t *pos_on_main_axis, int64_t amount) const;
  void IncrementOffAxisPosition(int64_t *pos_on_off_axis, int64_t amount) const;

  MappedParameters GetMainAxisMappedParameters() const;
  MappedParameters GetOffAxisMappedParameters() const;

  // Compute the minimum distance between the incoming and outgoing wire when
  // the bundle is being broken out. This measurement is between the centres of
  // the two wires on either side of the gap, so inclues 1x wire width.
  int64_t GetMinMainAxisBreakoutGap(const MappedParameters &main_axis) const;

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
