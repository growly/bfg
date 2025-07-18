#include <gtest/gtest.h>

#include "point.h"
#include "radian.h"

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

TEST(PointTest, AddComponents) {
  Point test;
  ASSERT_EQ(0, test.x());
  ASSERT_EQ(0, test.y());

  test.AddComponents(3.5, 0.0);
  EXPECT_EQ(4, test.x());
  EXPECT_EQ(0, test.y());

  test.AddComponents(1.2, 0.0);
  EXPECT_EQ(5, test.x());
  EXPECT_EQ(0, test.y());

  test.AddComponents(3, Radian::kPi / 2);
  EXPECT_EQ(5, test.x());
  EXPECT_EQ(3, test.y());
}

TEST(PointTest, Component) {
  Point test(3, 4);
  EXPECT_EQ(3, test.Component(0.0));
  EXPECT_EQ(4, test.Component(Radian::kPi / 2));
}

// TODO(aryap): Test Point::SortAscendingX, Point::SortDescendingY.

}  // namespace
}  // namespace geometry
}  // namespace bfg
