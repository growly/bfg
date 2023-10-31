#include <cmath>

#include <optional>

#include <gtest/gtest.h>

#include <glog/logging.h>

#include "line.h"
#include "point.h"
#include "rectangle.h"

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

TEST(LineTest, AngleToHorizon) {
  Line a = Line({0, 0}, {1, 0});
  EXPECT_EQ(a.AngleToHorizon(), 0);

  Line b = Line({0, 0}, {0, 1});
  EXPECT_EQ(b.AngleToHorizon(), Line::kPi / 2);

  Line c = Line({0, 0}, {1, 1});
  EXPECT_EQ(c.AngleToHorizon(), Line::kPi / 4);

  Line d = Line({0, 0}, {-1, 0});
  EXPECT_EQ(d.AngleToHorizon(), Line::kPi);

  Line f = Line({0, 0}, {0, -1});
  EXPECT_EQ(f.AngleToHorizon(), -Line::kPi / 2);

  Line g = Line({0, 0}, {-1, -1});
  EXPECT_EQ(g.AngleToHorizon(), 2 * Line::kPi -3 * Line::kPi / 4);
}

TEST(LineTest, AngleToLine) {
  Line right = Line({0, 0}, {1, 0});
  Line up = Line({0, 0}, {0, 1});

  EXPECT_EQ(up.AngleToLine(up), 0.0);
  EXPECT_EQ(right.AngleToLine(right), 0.0);
  EXPECT_EQ(up.AngleToLine(right), 3 * Line::kPi / 2);
  EXPECT_EQ(right.AngleToLine(up), Line::kPi / 2);

  //     /
  //    /
  //   /
  //  / ) theta   a . b = ||a|| ||b|| cos (theta)
  // -----------
  //
  double from_inner_product = std::acos(
      right.DotProduct(up) / (up.Length() * right.Length()));
  EXPECT_EQ(right.AngleToLine(up), from_inner_product);

  // This is not true, however, because the dot-product method always gives us
  // the smaller of the angles between the two lines, and we make sure
  // AngleToLine gives us the same angle of rotation.
  EXPECT_NE(up.AngleToLine(right), from_inner_product);
}

TEST(LineTest, DotProduct) {
  Line left = Line({1, 1}, {1, 2});
  Line right = Line({3, 3}, {2, 3});
  EXPECT_EQ(0, left.DotProduct(right));
}

TEST(LineTest, Intersect) {
  Line a = Line({0, 0}, {1, 1});
  Line b = Line({0, 0}, {1, 1});

  // TODO(aryap): Complete.
}

TEST(LineTest, ProjectionCoefficient) {
  Line projectee = Line({0, 1}, {2, 1});

  EXPECT_EQ(0.5, projectee.ProjectionCoefficient({1, 1}));
  EXPECT_EQ(-0.5, projectee.ProjectionCoefficient({-1, -1}));

  // Move the same line to {123, 123}.
  projectee = Line({123, 124}, {125, 124});
  EXPECT_EQ(0.5, projectee.ProjectionCoefficient({124, 123}));
  EXPECT_EQ(-0.5, projectee.ProjectionCoefficient({122, 120}));
  EXPECT_EQ(-2.0, projectee.ProjectionCoefficient({119, 6}));
}

TEST(LineTest, ExtendToNearestIntersection) {
  geometry::Rectangle rectangle = Rectangle({0, 0}, {20, 20});

  std::vector<Line> boundary_lines;
  rectangle.GetBoundaryLines(&boundary_lines);

  for (const auto &line: boundary_lines) {
    LOG(INFO) << line;
  }

  Line test = Line({3, 3}, {4, 4});
  
  std::optional<Line> extension = test.ExtendToNearestIntersection(
      boundary_lines);

  EXPECT_TRUE(extension);
  EXPECT_EQ(Point({4, 4}), extension.value().start());
  EXPECT_EQ(Point({20, 20}), extension.value().end());
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
