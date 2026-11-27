// Smoke tests that generate whole tiles with default parameters and check for
// a non-empty layout and, where circuits are implemented, ports. These take
// minutes each (most of it routing), so they live in bfg_slow_test, which is
// not run by ctest. Use an optimised build:
//
//   cmake -S . -B build_release -DCMAKE_CXX_FLAGS=-O2
//   cmake --build build_release --target bfg_slow_test
//   cd build_release && ./bfg_slow_test

#include <gtest/gtest.h>

#include "../atoms/generator_test_fixture.h"
#include "fabric.h"
#include "interconnect.h"
#include "lut.h"
#include "lut_b.h"
#include "reduced_tile.h"
#include "s44.h"
#include "slice.h"

namespace bfg {
namespace tiles {
namespace {

using TileTest = GeneratorTest;

TEST_F(TileTest, LutB) {
  ExpectWellFormed(Generate<LutB>({}, "lut_b"), {});
}

// TODO(aryap): Lut (the older LUT design) aborts while routing: two nets are
// assigned to neighbouring vertices too close together for their vias.
TEST_F(TileTest, DISABLED_Lut) {
  Lut generator(&design_db_);
  ExpectWellFormed(generator.GenerateIntoDatabase("lut"), {});
}

// S44 does not add ports to its circuit yet.
TEST_F(TileTest, S44) {
  ExpectLayout(Generate<S44>({}, "s44"));
}

// The default Interconnect (8 x 16 muxes) builds a routing grid with ~173M
// edges under the default (all-pairs-per-track) cost model and needs ~54 GB.
// These tests use a 2 x 4 interconnect to keep the grid small while still
// exercising that cost model. Neither adds ports to its circuit yet.
Interconnect::Parameters SmallInterconnectParameters() {
  return Interconnect::Parameters {
      .num_rows = 2,
      .num_columns = 4
  };
}

TEST_F(TileTest, Slice) {
  Slice::Parameters parameters = {
      .num_luts_left = 2,
      .num_luts_right = 2,
      .interconnect = SmallInterconnectParameters()
  };
  ExpectLayout(Generate<Slice>(parameters, "slice"));
}

TEST_F(TileTest, Interconnect) {
  ExpectLayout(
      Generate<Interconnect>(SmallInterconnectParameters(), "interconnect"));
}

// ReducedTile (and so Fabric) reads its interconnect from an edge list CSV
// (bfg_edges.csv by default) that is generated outside BFG and is not in the
// repository. Enable these once a test edge list is checked in.
TEST_F(TileTest, DISABLED_ReducedTile) {
  ExpectWellFormed(Generate<ReducedTile>({}, "reduced_tile"), {});
}

TEST_F(TileTest, DISABLED_Fabric) {
  ExpectWellFormed(Generate<Fabric>({}, "fabric"), {});
}

}  // namespace
}  // namespace tiles
}  // namespace bfg
