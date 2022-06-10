#include <gtest/gtest.h>

#include "rectangle.h"

namespace bfg {
namespace geometry {
namespace {

TEST(RectangleTest, Width) {
  Rectangle rect(Point(0, 0), 500, 500);
  EXPECT_EQ(500, rect.Width());
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
