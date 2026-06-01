#include <gtest/gtest.h>

#include <sstream>

#include "slice.h"

namespace bfg {
namespace circuit {
namespace {

TEST(Slice, WidthSpanningMultipleBits) {
  Signal s("bus", 4);
  Slice slice(s, 1, 3);
  EXPECT_EQ(slice.Width(), 3u);
}

TEST(Slice, SingleBitWidth) {
  Signal s("bit", 1);
  Slice slice(s, 0, 0);
  EXPECT_EQ(slice.Width(), 1u);
}

TEST(Slice, CopyConstructorPreservesFields) {
  Signal s("sig", 8);
  Slice original(s, 2, 5);
  Slice copy(original);
  EXPECT_EQ(copy.low_index(), 2u);
  EXPECT_EQ(copy.high_index(), 5u);
  EXPECT_EQ(&copy.signal(), &s);
  EXPECT_EQ(copy.Width(), original.Width());
}

TEST(Slice, ToVLSIRSlice) {
  Signal s("data", 8);
  Slice slice(s, 0, 3);
  auto pb = slice.ToVLSIRSlice();
  EXPECT_EQ(pb.signal(), "data");
  EXPECT_EQ(pb.bot(), 0u);
  EXPECT_EQ(pb.top(), 3u);
}

TEST(Slice, StreamOperatorPrintsSignalName) {
  Signal s("mybus", 8);
  Slice slice(s, 0, 3);
  std::ostringstream os;
  os << slice;
  EXPECT_EQ(os.str(), "mybus");
}

}  // namespace
}  // namespace circuit
}  // namespace bfg
