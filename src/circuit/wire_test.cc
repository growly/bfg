#include <gtest/gtest.h>

#include "wire.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Wire, IsAlwaysSingleBit) {
  Signal s("clk", 1);
  Wire w(s, 0);
  EXPECT_EQ(w.Width(), 1u);
  EXPECT_EQ(w.low_index(), 0u);
  EXPECT_EQ(w.high_index(), 0u);
}

TEST(Wire, IndexIntoWiderSignal) {
  Signal s("bus", 8);
  Wire w(s, 5);
  EXPECT_EQ(w.Width(), 1u);
  EXPECT_EQ(w.low_index(), 5u);
  EXPECT_EQ(w.high_index(), 5u);
}

}  // namespace
}  // namespace circuit
}  // namespace bfg
