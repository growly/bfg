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

    bool flip = false;

    // Not sure if this can be automatically deduced, since other PDKs will
    // have multiple horizontal/vertical layers anyway.
    std::string horizontal_layer = "met1.drawing";
    std::string via_layer = "via1.drawing";
    std::string vertical_layer = "met2.drawing";

    std::optional<int64_t> horizontal_wire_width_nm;
    std::optional<int64_t> horizontal_wire_separation_nm;
    std::optional<int64_t> horizontal_wire_offset_nm;
    std::optional<int64_t> vertical_wire_width_nm;
    std::optional<int64_t> vertical_wire_separation_nm;
    std::optional<int64_t> vertical_wire_offset_nm;

    // The length of the block is either its height or its width depending on
    // whether the routing is vertical or horizontal (respectively).
    uint64_t length = 10000;

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

  struct TrackTriple {
    static TrackTriple Make(
        const PhysicalPropertiesDatabase &db,
        const std::optional<int64_t> &forced_width_nm,
        const std::optional<int64_t> &forced_separation_nm,
        const std::optional<int64_t> &forced_offset_nm,
        const IntraLayerConstraints &rules);

    int64_t width;
    int64_t separation;
    int64_t offset;
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
  }

  Cell *GenerateIntoDatabase(const std::string &name) override;

  //void Route(
  //    const std::vector<std::vector<geometry::Instance*>> muxes,
  //    Layout *layout);

 private:
  geometry::Point MakePoint(
      int64_t main_axis_pos, int64_t off_axis_pos) const;

  TrackTriple GetMainAxisTrackTriple() const;
  TrackTriple GetOffAxisTrackTriple() const;

  Parameters parameters_;

  const geometry::Layer main_layer_;
  const geometry::Layer via_layer_;
  const geometry::Layer off_layer_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_WIRE_BLOCK_H_
