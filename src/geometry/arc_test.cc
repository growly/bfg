#include <gtest/gtest.h>
#include <glog/logging.h>

#include <cmath>
#include <vector>

#include "arc.h"
#include "line.h"
#include "point.h"

namespace bfg {
namespace geometry {
namespace {

// Helper: check that a point with the given (x, y) exists in the results,
// within some tolerance.
bool HasPointNear(const std::vector<Point> &points,
                  double expected_x, double expected_y,
                  double tolerance = 1.0) {
  for (const auto &p : points) {
    if (std::abs(p.x() - expected_x) <= tolerance &&
        std::abs(p.y() - expected_y) <= tolerance) {
      return true;
    }
  }
  return false;
}

}  // namespace

// --- IntersectingPoints(const Line &) tests ---

// A full circle (0-360) at the origin with radius 100.
// This removes arc angle filtering from consideration so we can focus on the
// line-circle intersection geometry.

TEST(ArcTest, IntersectingPoints_HorizontalLineThroughCentre) {
  // Full circle at origin, radius 100.
  Arc arc(Point(0, 0), 100, 0, 360);
  // Horizontal line y=0 from x=-200 to x=200.
  Line line(Point(-200, 0), Point(200, 0));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(2, points.size());
  EXPECT_TRUE(HasPointNear(points, -100, 0));
  EXPECT_TRUE(HasPointNear(points, 100, 0));
}

TEST(ArcTest, IntersectingPoints_VerticalLineThroughCentre) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Vertical line x=0.
  Line line(Point(0, -200), Point(0, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(2, points.size());
  EXPECT_TRUE(HasPointNear(points, 0, -100));
  EXPECT_TRUE(HasPointNear(points, 0, 100));
}

TEST(ArcTest, IntersectingPoints_VerticalLineTangent) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Vertical line x=100, tangent to the circle on the right.
  Line line(Point(100, -200), Point(100, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(1, points.size());
  EXPECT_TRUE(HasPointNear(points, 100, 0));
}

TEST(ArcTest, IntersectingPoints_VerticalLineNoIntersection) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Vertical line x=200, outside the circle.
  Line line(Point(200, -200), Point(200, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(0, points.size());
}

TEST(ArcTest, IntersectingPoints_VerticalLineOffsetCentre) {
  // Circle centred at (50, 50), radius 100.
  Arc arc(Point(50, 50), 100, 0, 360);
  // Vertical line x=50 (through centre).
  Line line(Point(50, -200), Point(50, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(2, points.size());
  EXPECT_TRUE(HasPointNear(points, 50, -50));
  EXPECT_TRUE(HasPointNear(points, 50, 150));
}

TEST(ArcTest, IntersectingPoints_DiagonalLineThroughCentre) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Line y = x, from (-200, -200) to (200, 200).
  Line line(Point(-200, -200), Point(200, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  // Intersections at (100/sqrt(2), 100/sqrt(2)) and (-100/sqrt(2), -100/sqrt(2)).
  double v = 100.0 / std::sqrt(2.0);
  EXPECT_EQ(2, points.size());
  EXPECT_TRUE(HasPointNear(points, v, v, 2));
  EXPECT_TRUE(HasPointNear(points, -v, -v, 2));
}

TEST(ArcTest, IntersectingPoints_HorizontalLineTangentTop) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Horizontal line y=100, tangent at the top.
  Line line(Point(-200, 100), Point(200, 100));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(1, points.size());
  EXPECT_TRUE(HasPointNear(points, 0, 100));
}

TEST(ArcTest, IntersectingPoints_HorizontalLineNoIntersection) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Horizontal line y=200, outside the circle.
  Line line(Point(-200, 200), Point(200, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(0, points.size());
}

TEST(ArcTest, IntersectingPoints_HorizontalLineOffsetCentre) {
  // Circle centred at (30, 40), radius 50.
  Arc arc(Point(30, 40), 50, 0, 360);
  // Horizontal line y=40 (through centre).
  Line line(Point(-200, 40), Point(200, 40));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(2, points.size());
  EXPECT_TRUE(HasPointNear(points, -20, 40));
  EXPECT_TRUE(HasPointNear(points, 80, 40));
}

TEST(ArcTest, IntersectingPoints_DiagonalLineNoIntersection) {
  Arc arc(Point(0, 0), 10, 0, 360);
  // Line y = x + 100, far from the circle of radius 10.
  Line line(Point(0, 100), Point(100, 200));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(0, points.size());
}

TEST(ArcTest, IntersectingPoints_DiagonalLineTwoIntersections) {
  // Circle at origin, radius 50.
  Arc arc(Point(0, 0), 50, 0, 360);
  // Line y = x + 10.
  Line line(Point(-200, -190), Point(200, 210));
  std::vector<Point> points = arc.IntersectingPoints(line);
  EXPECT_EQ(2, points.size());
}

// --- InArcBounds tests ---
//
// InArcBounds computes the angle from the arc's centre to the given point and
// checks whether it falls within [start_angle_deg_, end_angle_deg_] measured
// counter-clockwise. Angles follow the standard math convention:
//   0° = positive x-axis, 90° = positive y-axis, etc.

TEST(ArcTest, InArcBounds_PointOnStartAngle) {
  // Arc from 45° to 135°. A point at exactly 45° from the centre.
  Arc arc(Point(0, 0), 100, 45, 135);
  // Point at 45°: (r*cos45, r*sin45) ≈ (70, 70).
  EXPECT_TRUE(arc.InArcBounds(Point(70, 70)));
}

TEST(ArcTest, InArcBounds_PointOnEndAngle) {
  // Arc from 45° to 135°. A point at exactly 135° from the centre.
  Arc arc(Point(0, 0), 100, 45, 135);
  // Point at 135°: (r*cos135, r*sin135) ≈ (-70, 70).
  EXPECT_TRUE(arc.InArcBounds(Point(-70, 70)));
}

TEST(ArcTest, InArcBounds_PointInsideRange) {
  // Arc from 0° to 180°. A point at 90° (straight up).
  Arc arc(Point(0, 0), 100, 0, 180);
  EXPECT_TRUE(arc.InArcBounds(Point(0, 50)));
}

TEST(ArcTest, InArcBounds_PointOutsideRange) {
  // Arc from 0° to 90° (first quadrant only). A point at 180° (negative x).
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_FALSE(arc.InArcBounds(Point(-50, 0)));
}

TEST(ArcTest, InArcBounds_FullCircle) {
  // Full circle: any point should be in bounds.
  Arc arc(Point(0, 0), 100, 0, 360);
  EXPECT_TRUE(arc.InArcBounds(Point(100, 0)));    // 0°
  EXPECT_TRUE(arc.InArcBounds(Point(0, 100)));    // 90°
  EXPECT_TRUE(arc.InArcBounds(Point(-100, 0)));   // 180°
  EXPECT_TRUE(arc.InArcBounds(Point(0, -100)));   // 270°
  EXPECT_TRUE(arc.InArcBounds(Point(70, -70)));   // 315°
}

TEST(ArcTest, InArcBounds_QuadrantI) {
  // Arc spanning only quadrant I: 0° to 90°.
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_TRUE(arc.InArcBounds(Point(70, 70)));     // ~45°, inside
  EXPECT_FALSE(arc.InArcBounds(Point(-70, 70)));   // ~135°, outside
  EXPECT_FALSE(arc.InArcBounds(Point(-70, -70)));  // ~225°, outside
  EXPECT_FALSE(arc.InArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, InArcBounds_QuadrantII) {
  // Arc spanning quadrant II: 90° to 180°.
  Arc arc(Point(0, 0), 100, 90, 180);
  EXPECT_TRUE(arc.InArcBounds(Point(-70, 70)));    // ~135°, inside
  EXPECT_FALSE(arc.InArcBounds(Point(70, 70)));    // ~45°, outside
  EXPECT_FALSE(arc.InArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, InArcBounds_QuadrantIII) {
  // Arc spanning quadrant III: 180° to 270°.
  Arc arc(Point(0, 0), 100, 180, 270);
  EXPECT_TRUE(arc.InArcBounds(Point(-70, -70)));   // ~225°, inside
  EXPECT_FALSE(arc.InArcBounds(Point(70, 70)));    // ~45°, outside
  EXPECT_FALSE(arc.InArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, InArcBounds_QuadrantIV) {
  // Arc spanning quadrant IV: 270° to 360°.
  Arc arc(Point(0, 0), 100, 270, 360);
  EXPECT_TRUE(arc.InArcBounds(Point(70, -70)));    // ~315°, inside
  EXPECT_FALSE(arc.InArcBounds(Point(-70, -70)));  // ~225°, outside
  EXPECT_FALSE(arc.InArcBounds(Point(70, 70)));    // ~45°, outside
}

TEST(ArcTest, InArcBounds_SpanningZero) {
  // Arc from 315° to 45° (crossing the 0° boundary).
  // This is the tricky wraparound case where end < start numerically.
  Arc arc(Point(0, 0), 100, 315, 405);
  // Point at 0° (positive x-axis) should be inside.
  EXPECT_TRUE(arc.InArcBounds(Point(100, 0)));
  // Point at 350° should be inside.
  EXPECT_TRUE(arc.InArcBounds(Point(98, -17)));
  // Point at 30° should be inside.
  EXPECT_TRUE(arc.InArcBounds(Point(87, 50)));
  // Point at 180° should be outside.
  EXPECT_FALSE(arc.InArcBounds(Point(-100, 0)));
  // Point at 90° should be outside.
  EXPECT_FALSE(arc.InArcBounds(Point(0, 100)));
}

TEST(ArcTest, InArcBounds_NarrowArc) {
  // A very narrow arc: 89° to 91°.
  Arc arc(Point(0, 0), 100, 89, 91);
  // Point at 90° (straight up) should be inside.
  EXPECT_TRUE(arc.InArcBounds(Point(0, 100)));
  // Point at 80° should be outside.
  EXPECT_FALSE(arc.InArcBounds(Point(17, 98)));
  // Point at 100° should be outside.
  EXPECT_FALSE(arc.InArcBounds(Point(-17, 98)));
}

TEST(ArcTest, InArcBounds_OffsetCentre) {
  // Arc centred at (100, 200), from 0° to 90°.
  // InArcBounds should measure the angle relative to the centre, not the origin.
  Arc arc(Point(100, 200), 50, 0, 90);
  // Point at ~45° relative to centre: centre + (35, 35).
  EXPECT_TRUE(arc.InArcBounds(Point(135, 235)));
  // Point at ~135° relative to centre: centre + (-35, 35).
  EXPECT_FALSE(arc.InArcBounds(Point(65, 235)));
  // Point at ~225° relative to centre: centre + (-35, -35).
  EXPECT_FALSE(arc.InArcBounds(Point(65, 165)));
}

TEST(ArcTest, InArcBounds_PointOnAxis) {
  // Arc from 0° to 180°. Check points exactly on the axes.
  Arc arc(Point(0, 0), 100, 0, 180);
  EXPECT_TRUE(arc.InArcBounds(Point(100, 0)));     // 0° (start boundary)
  EXPECT_TRUE(arc.InArcBounds(Point(0, 100)));     // 90° (middle)
  EXPECT_TRUE(arc.InArcBounds(Point(-100, 0)));    // 180° (end boundary)
  EXPECT_FALSE(arc.InArcBounds(Point(0, -100)));   // 270° (outside)
}

}  // namespace geometry
}  // namespace bfg
