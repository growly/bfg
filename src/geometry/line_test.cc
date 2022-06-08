#include <gtest/gtest.h>

namespace bfg {
namespace geometry {
namespace {

TEST(LineTest, TestsWork) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
