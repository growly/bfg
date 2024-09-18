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

TEST(RectangleTest, Intersects) {
  Rectangle test = Rectangle({1, 1}, {3, 3});
  for (int64_t i = 1; i <= 3; i++) {
    for (int64_t j = 1; j <= 3; j++) {
      EXPECT_TRUE(test.Intersects({i, j}));
    }
  }
  EXPECT_FALSE(test.Intersects({4, 4}));
  EXPECT_FALSE(test.Intersects({0, 0}));
  EXPECT_FALSE(test.Intersects({1, 5}));
  EXPECT_FALSE(test.Intersects({2, -3}));
}

TEST(RectangleTest, Intersects_WithNegativeMargin) {
  Rectangle test = Rectangle({0, 0}, {4, 4});
  for (int64_t i = 1; i <= 3; i++) {
    for (int64_t j = 1; j <= 3; j++) {
      EXPECT_TRUE(test.Intersects({i, j}, -1));
    }
  }
  EXPECT_FALSE(test.Intersects({4, 4}, -1));
  EXPECT_FALSE(test.Intersects({0, 0}, -1));
  EXPECT_FALSE(test.Intersects({1, 5}, -1));
  EXPECT_FALSE(test.Intersects({2, -3}, -1));
}

TEST(RectangleTest, Intersects_WithPositiveMargin) {
  Rectangle test = Rectangle({1, 1}, {3, 3});
  for (int64_t i = 0; i <= 4; i++) {
    for (int64_t j = 0; j <= 4; j++) {
      EXPECT_TRUE(test.Intersects({i, j}, 1));
    }
  }
  EXPECT_FALSE(test.Intersects({5, 5}, 1));
  EXPECT_FALSE(test.Intersects({-1, -1}, 1));
  EXPECT_FALSE(test.Intersects({1, 5}, 1));
  EXPECT_FALSE(test.Intersects({2, -3}, 1));
}

TEST(RectangleTest, IntersectingPoints) {
  Rectangle test = Rectangle({0, 0}, {4, 4});

  Line ray = Line({0, -1}, {0, 5});
  auto intersection = test.IntersectingPoints(ray);
  ASSERT_NE(std::nullopt, intersection);
  EXPECT_EQ(Point(0, 0), intersection->first);
  EXPECT_EQ(Point(0, 4), intersection->second);

  ray = Line({0, 5}, {0, -1});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(std::nullopt, intersection);
  EXPECT_EQ(Point(0, 4), intersection->first);
  EXPECT_EQ(Point(0, 0), intersection->second);

  ray = Line({-1, 1}, {4, 1});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(std::nullopt, intersection);
  EXPECT_EQ(Point(0, 1), intersection->first);
  EXPECT_EQ(Point(4, 1), intersection->second);

  ray = Line({2, -1}, {2, 3});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(std::nullopt, intersection);
  EXPECT_EQ(Point(2, 0), intersection->first);
  EXPECT_EQ(Point(2, 4), intersection->second);

  // The pathological case. This would intersect all four boundary lines if not
  // deliberately accounted for.
  ray = Line({0, 0}, {4, 4});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(std::nullopt, intersection);
  EXPECT_EQ(Point(0, 0), intersection->first);
  EXPECT_EQ(Point(4, 4), intersection->second);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
