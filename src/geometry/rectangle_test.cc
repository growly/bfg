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
  ASSERT_NE(0, intersection.size());
  EXPECT_EQ(Point(0, 0), intersection.front().first);
  EXPECT_EQ(Point(0, 4), intersection.front().second);

  ray = Line({0, 5}, {0, -1});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(0, intersection.size());
  EXPECT_EQ(Point(0, 4), intersection.front().first);
  EXPECT_EQ(Point(0, 0), intersection.front().second);

  ray = Line({-1, 1}, {4, 1});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(0, intersection.size());
  EXPECT_EQ(Point(0, 1), intersection.front().first);
  EXPECT_EQ(Point(4, 1), intersection.front().second);

  ray = Line({2, -1}, {2, 3});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(0, intersection.size());
  EXPECT_EQ(Point(2, 0), intersection.front().first);
  EXPECT_EQ(Point(2, 4), intersection.front().second);

  // The pathological case. This would intersect all four boundary lines if not
  // deliberately accounted for.
  ray = Line({0, 0}, {4, 4});
  intersection = test.IntersectingPoints(ray);
  ASSERT_NE(0, intersection.size());
  EXPECT_EQ(Point(0, 0), intersection.front().first);
  EXPECT_EQ(Point(4, 4), intersection.front().second);
}

TEST(RectangleTest, MirrorY_AtOrigin) {
  Rectangle rect(Point(0, 0), 10, 20);
  rect.MirrorY();
  // MirrorY negates x coordinates: (0,0)-(10,20) -> (-10,0)-(0,20)
  EXPECT_EQ(Point(-10, 0), rect.lower_left());
  EXPECT_EQ(Point(0, 20), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, MirrorY_OffsetFromOrigin) {
  Rectangle rect(Point(5, 10), Point(15, 30));
  rect.MirrorY();
  // MirrorY negates x coordinates: (5,10)-(15,30) -> (-15,10)-(-5,30)
  EXPECT_EQ(Point(-15, 10), rect.lower_left());
  EXPECT_EQ(Point(-5, 30), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, MirrorY_NegativeCoordinates) {
  Rectangle rect(Point(-20, -10), Point(-5, 5));
  rect.MirrorY();
  // MirrorY negates x coordinates: (-20,-10)-(-5,5) -> (5,-10)-(20,5)
  EXPECT_EQ(Point(5, -10), rect.lower_left());
  EXPECT_EQ(Point(20, 5), rect.upper_right());
}

TEST(RectangleTest, MirrorY_TwiceReturnsToOriginal) {
  Rectangle original(Point(3, 7), Point(13, 27));
  Rectangle rect = original;
  rect.MirrorY();
  rect.MirrorY();
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, MirrorX_AtOrigin) {
  Rectangle rect(Point(0, 0), 10, 20);
  rect.MirrorX();
  // MirrorX negates y coordinates: (0,0)-(10,20) -> (0,-20)-(10,0)
  EXPECT_EQ(Point(0, -20), rect.lower_left());
  EXPECT_EQ(Point(10, 0), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, MirrorX_OffsetFromOrigin) {
  Rectangle rect(Point(5, 10), Point(15, 30));
  rect.MirrorX();
  // MirrorX negates y coordinates: (5,10)-(15,30) -> (5,-30)-(15,-10)
  EXPECT_EQ(Point(5, -30), rect.lower_left());
  EXPECT_EQ(Point(15, -10), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, MirrorX_NegativeCoordinates) {
  Rectangle rect(Point(-20, -30), Point(-5, -10));
  rect.MirrorX();
  // MirrorX negates y coordinates: (-20,-30)-(-5,-10) -> (-20,10)-(-5,30)
  EXPECT_EQ(Point(-20, 10), rect.lower_left());
  EXPECT_EQ(Point(-5, 30), rect.upper_right());
}

TEST(RectangleTest, MirrorX_TwiceReturnsToOriginal) {
  Rectangle original(Point(3, 7), Point(13, 27));
  Rectangle rect = original;
  rect.MirrorX();
  rect.MirrorX();
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, Translate_ZeroOffset) {
  Rectangle rect(Point(5, 10), Point(15, 30));
  rect.Translate(Point(0, 0));
  EXPECT_EQ(Point(5, 10), rect.lower_left());
  EXPECT_EQ(Point(15, 30), rect.upper_right());
}

TEST(RectangleTest, Translate_PositiveOffset) {
  Rectangle rect(Point(0, 0), 10, 20);
  rect.Translate(Point(100, 200));
  EXPECT_EQ(Point(100, 200), rect.lower_left());
  EXPECT_EQ(Point(110, 220), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, Translate_NegativeOffset) {
  Rectangle rect(Point(50, 60), Point(70, 80));
  rect.Translate(Point(-30, -40));
  EXPECT_EQ(Point(20, 20), rect.lower_left());
  EXPECT_EQ(Point(40, 40), rect.upper_right());
}

TEST(RectangleTest, Translate_ToNegativeCoordinates) {
  Rectangle rect(Point(10, 20), Point(30, 40));
  rect.Translate(Point(-50, -60));
  EXPECT_EQ(Point(-40, -40), rect.lower_left());
  EXPECT_EQ(Point(-20, -20), rect.upper_right());
}

TEST(RectangleTest, Translate_PreservesWidthAndHeight) {
  Rectangle rect(Point(0, 0), 100, 200);
  rect.Translate(Point(1000, 2000));
  EXPECT_EQ(100, rect.Width());
  EXPECT_EQ(200, rect.Height());
}

TEST(RectangleTest, FlipHorizontal_IsNoOp) {
  Rectangle original(Point(5, 10), Point(15, 30));
  Rectangle rect = original;
  rect.FlipHorizontal();
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, FlipVertical_IsNoOp) {
  Rectangle original(Point(5, 10), Point(15, 30));
  Rectangle rect = original;
  rect.FlipVertical();
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, Rotate_90Degrees) {
  // Rectangle at origin: (0,0)-(10,20)
  // After 90 degree CCW rotation around origin:
  // (0,0) -> (0,0), (10,0) -> (0,10), (10,20) -> (-20,10), (0,20) -> (-20,0)
  // Bounding box: (-20,0)-(0,10)
  Rectangle rect(Point(0, 0), 10, 20);
  rect.Rotate(90);
  EXPECT_EQ(Point(-20, 0), rect.lower_left());
  EXPECT_EQ(Point(0, 10), rect.upper_right());
  // Width and height should swap
  EXPECT_EQ(20, rect.Width());
  EXPECT_EQ(10, rect.Height());
}

TEST(RectangleTest, Rotate_180Degrees) {
  // Rectangle (0,0)-(10,20) rotated 180 degrees around origin:
  // becomes (-10,-20)-(0,0)
  Rectangle rect(Point(0, 0), 10, 20);
  rect.Rotate(180);
  EXPECT_EQ(Point(-10, -20), rect.lower_left());
  EXPECT_EQ(Point(0, 0), rect.upper_right());
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(20, rect.Height());
}

TEST(RectangleTest, Rotate_270Degrees) {
  // Rectangle (0,0)-(10,20) rotated 270 degrees CCW (= 90 CW) around origin:
  // (0,0) -> (0,0), (10,0) -> (0,-10), (10,20) -> (20,-10), (0,20) -> (20,0)
  // Bounding box: (0,-10)-(20,0)
  Rectangle rect(Point(0, 0), 10, 20);
  rect.Rotate(270);
  EXPECT_EQ(Point(0, -10), rect.lower_left());
  EXPECT_EQ(Point(20, 0), rect.upper_right());
  EXPECT_EQ(20, rect.Width());
  EXPECT_EQ(10, rect.Height());
}

TEST(RectangleTest, Rotate_360DegreesReturnsToOriginal) {
  Rectangle original(Point(5, 10), Point(15, 30));
  Rectangle rect = original;
  rect.Rotate(360);
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, Rotate_ZeroDegrees) {
  Rectangle original(Point(5, 10), Point(15, 30));
  Rectangle rect = original;
  rect.Rotate(0);
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, Rotate_Square90Degrees) {
  // Square should maintain dimensions after rotation
  Rectangle rect(Point(0, 0), 10, 10);
  rect.Rotate(90);
  EXPECT_EQ(10, rect.Width());
  EXPECT_EQ(10, rect.Height());
}

TEST(RectangleTest, Rotate_NegativeDegrees) {
  // -90 degrees should be equivalent to 270 degrees CCW
  Rectangle rect1(Point(0, 0), 10, 20);
  Rectangle rect2(Point(0, 0), 10, 20);
  rect1.Rotate(-90);
  rect2.Rotate(270);
  EXPECT_TRUE(rect1 == rect2);
}

TEST(RectangleTest, Rotate_FourRotationsOf90ReturnsToOriginal) {
  Rectangle original(Point(1, 2), Point(5, 10));
  Rectangle rect = original;
  rect.Rotate(90);
  rect.Rotate(90);
  rect.Rotate(90);
  rect.Rotate(90);
  EXPECT_TRUE(original == rect);
}

TEST(RectangleTest, Rotate_OffsetRectangle180Degrees) {
  // Rectangle offset from origin
  Rectangle rect(Point(10, 20), Point(30, 50));
  rect.Rotate(180);
  // 180 degree rotation: (x,y) -> (-x,-y)
  EXPECT_EQ(Point(-30, -50), rect.lower_left());
  EXPECT_EQ(Point(-10, -20), rect.upper_right());
}

TEST(RectangleTest, CombinedOperations_MirrorYThenTranslate) {
  Rectangle rect(Point(5, 10), Point(15, 30));
  rect.MirrorY();
  rect.Translate(Point(20, 0));
  // After MirrorY: (-15,10)-(-5,30)
  // After Translate: (5,10)-(15,30)
  EXPECT_EQ(Point(5, 10), rect.lower_left());
  EXPECT_EQ(Point(15, 30), rect.upper_right());
}

TEST(RectangleTest, CombinedOperations_MirrorXThenMirrorY) {
  // MirrorX then MirrorY is equivalent to 180 degree rotation
  Rectangle rect1(Point(5, 10), Point(15, 30));
  Rectangle rect2(Point(5, 10), Point(15, 30));
  rect1.MirrorX();
  rect1.MirrorY();
  rect2.Rotate(180);
  EXPECT_TRUE(rect1 == rect2);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
