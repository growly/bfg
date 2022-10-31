#include <gtest/gtest.h>

#include "line.h"
#include "point.h"
#include "poly_line.h"

namespace bfg {
namespace geometry {
namespace {

//         x
//  +--o--o
//        |x
//   x    o
//        +
TEST(PolyLineTest, IntersectsPoint) {
  PolyLine line = PolyLine({{0, 3}, {6, 3}, {6, 0}});
  size_t index = 0;
  EXPECT_TRUE(line.Intersects(Point(3, 3), &index));
  EXPECT_EQ(0, index);
  EXPECT_FALSE(line.Intersects(Point(1, 1), &index));
  EXPECT_EQ(-1, index);
  EXPECT_FALSE(line.Intersects(Point(7, 2), &index));
  EXPECT_TRUE(line.Intersects(Point(6, 1), &index));
  EXPECT_EQ(1, index);
  EXPECT_TRUE(line.Intersects(Point(6, 3), &index));
  EXPECT_EQ(0, index);
  EXPECT_FALSE(line.Intersects(Point(7, 4), &index));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
