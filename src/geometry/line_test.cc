#include <gtest/gtest.h>

#include "line.h"
#include "point.h"

namespace bfg {
namespace geometry {
namespace {

//           o
//          /
//         /
//        x (2, 1)
//       /
//      /
//     x (0, 0)
TEST(LineTest, IntersectsPoint) {
  Point start = Point(0, 0);
  Point end = Point(2, 1);
  Line line = Line(start, end);

  EXPECT_TRUE(line.Intersects(Point(4, 2)));
  EXPECT_FALSE(line.Intersects(Point(4, 3)));
  EXPECT_FALSE(line.Intersects(Point(2, 3)));
  EXPECT_TRUE(line.Intersects(Point(0, 0)));
  EXPECT_TRUE(line.Intersects(Point(-2, -1)));
  EXPECT_FALSE(line.Intersects(Point(-2, -2)));

  // This doesn't fall on the line (which would go through (1, 0.5)), actually,
  // but it's the closest we can get on our grid so it counts:
  EXPECT_TRUE(line.Intersects(Point(1, 1)));
  EXPECT_TRUE(line.Intersects(Point(-1, -1)));
}

TEST(LineTest, IntersectsInBoundsPoint) {
  Point start = Point(0, 0);
  Point end = Point(2, 1);
  Line line = Line(start, end);

  EXPECT_FALSE(line.IntersectsInBounds(Point(4, 2)));   // Yes but OOB.
  EXPECT_FALSE(line.IntersectsInBounds(Point(4, 3)));
  EXPECT_FALSE(line.IntersectsInBounds(Point(2, 3)));
  EXPECT_TRUE(line.IntersectsInBounds(Point(0, 0)));
  EXPECT_TRUE(line.IntersectsInBounds(Point(2, 1)));
  EXPECT_FALSE(line.IntersectsInBounds(Point(-2, -1))); // Yes but OOB.
  EXPECT_FALSE(line.IntersectsInBounds(Point(-2, -2)));

  // This doesn't fall on the line (which would go through (1, 0.5)), actually,
  // but it's the closest we can get on our grid so it counts:
  EXPECT_TRUE(line.IntersectsInBounds(Point(1, 1)));
  EXPECT_FALSE(line.IntersectsInBounds(Point(-1, -1)));
}

// Using 3, 4, 5 right-angled triangle for integral coordinates:
//           x (12, 16)
//          /
//         /
//        o (6, 8)    
//       /
//      /
//     x
TEST(LineTest, PointOnLineAtDistance_Integers) {
  Line line = Line(Point(0, 0), Point(12, 16));
  Point after = line.PointOnLineAtDistance(Point(6, 8), 5.0);
  EXPECT_EQ(Point(9, 12), after);
  Point before = line.PointOnLineAtDistance(Point(6, 8), -5.0);
  EXPECT_EQ(Point(3, 4), before);

  // If the line goes in the opposite direction, 'before' and 'after' are
  // swapped.
  line = Line(Point(12, 16), Point(0, 0));
  after = line.PointOnLineAtDistance(Point(6, 8), 5.0);
  EXPECT_EQ(Point(3, 4), after);
  before = line.PointOnLineAtDistance(Point(6, 8), -5.0);
  EXPECT_EQ(Point(9, 12), before);
}

TEST(LineTest, PointOnLineAtDistance) {
  Line line = Line(Point(0, 0), Point(2, 2));
  Point after = line.PointOnLineAtDistance(Point(1, 1), 1.41);
  EXPECT_EQ(Point(2, 2), after);
  Point before = line.PointOnLineAtDistance(Point(1, 1), -1.41);
  EXPECT_EQ(Point(0, 0), before);
}

TEST(LineTest, DotProduct) {
  Line left = Line({1, 1}, {1, 2});
  Line right = Line({3, 3}, {2, 3});
  EXPECT_EQ(0, left.DotProduct(right));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
