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

// TODO(aryap): Slice names every LUT instance "lut_i", so the second one is
// a fatal duplicate instance name.
TEST_F(TileTest, DISABLED_Slice) {
  ExpectWellFormed(Generate<Slice>({}, "slice"), {});
}

TEST_F(TileTest, Interconnect) {
  ExpectWellFormed(Generate<Interconnect>({}, "interconnect"), {});
}

TEST_F(TileTest, ReducedTile) {
  ExpectWellFormed(Generate<ReducedTile>({}, "reduced_tile"), {});
}

TEST_F(TileTest, Fabric) {
  ExpectWellFormed(Generate<Fabric>({}, "fabric"), {});
}

}  // namespace
}  // namespace tiles
}  // namespace bfg
