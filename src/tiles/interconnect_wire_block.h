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
    RoutingTrackDirection direction = RoutingTrackDirection::kTrackVertical;

    // Not sure if this can be automatically deduced, since other PDKs will
    // have multiple horizontal/vertical layers anyway.
    std::string horizontal_layer = "met1.drawing";
    std::string vertical_layer = "met2.drawing";

    // The length of the block is either its height or its width depending on
    // whether the routing is vertical or horizontal (respectively).
    uint64_t length;

    // TODO(aryap): Complete.
    struct Bundle {
      int num_wires;
    };

    struct Channel {
      std::string name;
      std::set<int> break_out;
      int num_bundles;
      Bundle bundle;
    };

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
        parameters_(parameters) {
    // Determine the layers to use based on the parameters:
    DetermineLayersAndRules(parameters_.direction);
  }

  Cell *GenerateIntoDatabase(const std::string &name) override;

  //void Route(
  //    const std::vector<std::vector<geometry::Instance*>> muxes,
  //    Layout *layout);

 private:
  void DetermineLayersAndRules(const RoutingTrackDirection &main_direction);

  Parameters parameters_;

  const geometry::Layer &main_layer_;
  const geometry::Layer &via_layer_;
  const geometry::Layer &off_layer_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_WIRE_BLOCK_H_
