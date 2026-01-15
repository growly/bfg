#ifndef TILES_INTERCONNECT_WIRE_BLOCK_H_
#define TILES_INTERCONNECT_WIRE_BLOCK_H_

#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
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
class InterconnectWireBlock : public Tile {
 public:
  struct Parameters {
    struct Bundle {
      int num_wires;
    };

    struct Channel {
      std::string name;
      std::set<int> break_out;
      int num_bundles;
      Bundle bundle;
    };

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
  struct TrackTriple {
    int64_t width;
    int64_t pitch;
    int64_t offset;
  };

  TrackTriple MakeTrackTriple(
      const geometry::Layer &wire_layer,
      const std::optional<int64_t> forced_width_nm,
      const std::optional<int64_t> forced_pitch_nm,
      const std::optional<int64_t> forced_offset_nm) const;

  geometry::Point MakePoint(
      int64_t main_axis_pos, int64_t off_axis_pos) const;

  void DrawStraightThroughBundle(
      const Parameters::Channel &channel,
      const TrackTriple &main_axis,
      const TrackTriple &off_axis,
      size_t bundle_number,
      int64_t *off_axis_pos,
      Layout *layout) const;

  void DrawBrokenOutBundle(
      const Parameters::Channel &channel,
      const TrackTriple &main_axis,
      const TrackTriple &off_axis,
      int64_t breakout_gap,
      size_t bundle_number,
      int64_t *main_axis_pos,
      int64_t *off_axis_pos,
      Layout *layout) const;

  // TODO(aryap): Why don't I keep more state in the generator itself? It can
  // always be reset when Generate() is called again... these function
  // signatures are nuts!
  void IncrementMainAxisPosition(int64_t *main_axis_pos, int64_t amount) const;
  void IncrementOffAxisPosition(int64_t *off_axis_pos, int64_t amount) const;

  TrackTriple GetMainAxisTrackTriple() const;
  TrackTriple GetOffAxisTrackTriple() const;

  // Compute the minimum distance between the incoming and outgoing wire when
  // the bundle is being broken out. This measurement is between the centres of
  // the two wires on either side of the gap, so inclues 1x wire width.
  int64_t GetMinMainAxisBreakoutGap(const TrackTriple &main_axis) const;

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
