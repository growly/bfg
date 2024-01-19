#include <gtest/gtest.h>
#include <glog/logging.h>

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

TEST(RectangleTest, BoundingBoxIfRotated) {
  Rectangle initial(Point(1, 1), Point(2, 2));
  Rectangle rotated = initial.BoundingBoxIfRotated(
      Point(0, 0), 180);
  EXPECT_TRUE(Rectangle(Point(-2, -2), Point(-1, -1)) == rotated);
}

TEST(RectangleTest, ClosestDistanceBetween) {
  Rectangle anchor = Rectangle({-1, -1}, {1, 1});

  EXPECT_EQ(0, Rectangle::ClosestDistanceBetween(
      anchor, Rectangle({1, 1}, {2, 2})));
  // sqrt(2)
  LOG(INFO) << Rectangle::ClosestDistanceBetween(
      anchor, Rectangle({2, 2}, {3, 3}));
  EXPECT_EQ(1, Rectangle::ClosestDistanceBetween(
      anchor, Rectangle({2, 1}, {3, 3})));
  // sqrt(2)
  LOG(INFO) << Rectangle::ClosestDistanceBetween(
      anchor, Rectangle({2, -3}, {3, -2}));
  EXPECT_EQ(3, Rectangle::ClosestDistanceBetween(
      anchor, Rectangle({-4, -5}, {2, -4})));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
