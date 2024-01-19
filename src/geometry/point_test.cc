#include <gtest/gtest.h>

#include "point.h"

namespace bfg {
namespace geometry {
namespace {

TEST(PointTest, Scale) {
  Point p = {10, 11};

  Point a = Point(p);
  a.Scale(0.5);
  // Note rounding up instead of truncation to integer:
  EXPECT_EQ(Point({5, 6}), a);

  Point b = Point(p);
  b.Scale(2.0);
  EXPECT_EQ(Point({20, 22}), b);

  Point c = Point(p);
  c.Scale(0.3333333);
  EXPECT_EQ(Point({3, 4}), c);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
