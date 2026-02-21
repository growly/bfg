#ifndef TILES_REDUCED_SLICE_H_
#define TILES_REDUCED_SLICE_H_

#include <string_view>
#include <string>

#include "../atoms/sky130_interconnect_mux1.h"
#include "../atoms/sky130_interconnect_mux2.h"
#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "proto/parameters/reduced_slice.pb.h"
#include "tile.h"
#include "interconnect_wire_block.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

using namespace std::string_view_literals;

// The BFG ReducedSlice is an example of a complete FPGA slice built around a
// fixed number of LUTs and Interconnect muxes. The interconnectivity of these
// blocks is an input parameter.
//
// It is intended as an example of the greater BFG goal of producing a simple,
// reduced-complexity but still performant FPGA architectures. However, in the
// spirit of the rest of the tool, it should be possible (and desirable) to
// parameterise the numbers of LUTs and Interconnect muxes as well.
//
// Overview:
//
// Each ReducedSlice has two sides. Each side has interconnect muxes and
// connectivity to independent and shared interconnect wires, which connect to
// pins at the edge of the ReducedSlice.
//
// +------------------+-------+------------------+
// |                  |       |                  |
// |                  | Inter |                  |
// | West             | conne |             East |
// | Side             | ct wi |             Side |
// |                  | ring  |                  |
// |                  |       |                  |
// |                  |       |                  |
// |                  |       |                  |
// +------------------+-------+------------------+
// |        Interconnect Wiring                  |
// +------------------+-------+------------------+
//
// The west side looks something like:
//
// +-------------------------------+
// |   IIB S1                      |
// |    +--------------------------+
// |    |    IIB S2                |
// |    |    +-----------+---------+
// |    |    |           |         |
// |    |    |   LUTs    |         |
// |    |    |           |         |
// +----+----+-----------+         +
// |              OIB S2           |
// +-----------------------+-------+-+
// |                OIB S1 |  OIB S1 |
// |                       |         |
// +-----------------------+---------+
//
// The principle is to match the Clos-network-like layering of input/output
// mxues in layers into and out of the LUT complex.
//
// Please see documentation for more detail.
//
class ReducedSlice : public Tile {
 public:
  struct Parameters {
    static constexpr int kBundleSize = 4;
    // The units are tiles traversed:
    static constexpr int kInterconnectLengths[3] = {1, 2, 6};
    // We try C++17's string_view to avoid the pain of a statically-defined
    // C-style strings:
    static constexpr std::array kSidesOfTile = {"E"sv, "W"sv};
    // Note that the longest-length bundles are treated differently, per the
    // UltraScale-like architecture.

    // Per side.
    static constexpr int kNumLUTs = 8;
    static constexpr int kLutSize = 4;
    // TODO(aryap): Add LUT type as an option.
    //    32 logical 6:1 muxes is 16 shared 7:2 muxes.
    static constexpr int kNumIIBS1 = 16;
    static constexpr int kNumIIBS2 = 40;
    // ... of which:
    static constexpr int kNumIIBS2Bounce = 8;
    static constexpr int kNumIIBS2Bypass = 2;
    //    48 logical 4:1 is 24 shared 5:2 muxes.
    static constexpr int kNumOIBS2 = 24;
    static constexpr int kNumOIBS1 = 28;

    // There is a central structure of muxes that drives only the
    // longest-length interconnect wires. This drives one bundle in each of the
    // four directions (N, E, S, W).
    static constexpr int kNumOIBS1Shared = kBundleSize * 4;

    void ToProto(proto::parameters::ReducedSlice *pb) const;
    void FromProto(const proto::parameters::ReducedSlice &pb); 
  };

  ReducedSlice(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {
  }

  Cell *Generate() override;

  //void Route(
  //    const std::vector<std::vector<geometry::Instance*>> muxes,
  //    Layout *layout);

 private:
  void GenerateInterconnectChannels(
      const std::vector<std::string> &direction_prefixes,
      int64_t long_bundle_break_out,
      int64_t long_bundle_break_in,
      bool break_out_regular_side_first,
      bool alternate_break_out,
      InterconnectWireBlock::Parameters *iwb_params) const;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_REDUCED_SLICE_H_
