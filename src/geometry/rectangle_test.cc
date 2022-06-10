#include <gtest/gtest.h>

#include "rectangle.h"

namespace bfg {
namespace geometry {
namespace {

TEST(RectangleTest, Width) {
  Rectangle rect_a(Point(0, 0), 500, 500);
  EXPECT_EQ(500, rect_a.Width());

  Rectangle rect_b(Point(-50, -60), Point(70, 90));
  EXPECT_EQ(120, rect_b.Width());
}

TEST(RectangleTest, Height) {
  Rectangle rect_a(Point(0, 0), 500, 650);
  EXPECT_EQ(650, rect_a.Height());

  Rectangle rect_b(Point(-50, -60), Point(70, 90));
  EXPECT_EQ(150, rect_b.Height());
}

TEST(RectangleTest, Centre) {
  Rectangle rect_a(Point(0, 0), 500, 650);
  Point centre = rect_a.centre();
  EXPECT_EQ(250, centre.x());
  EXPECT_EQ(325, centre.y());
  
  // TODO(aryap): Needs operator== in the right namespace:
  // EXPECT_EQ(Point(250, 325), rect_a.centre());
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
