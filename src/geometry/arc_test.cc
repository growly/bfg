#include <gtest/gtest.h>
#include <glog/logging.h>

#include <cmath>
#include <vector>

#include "arc.h"
#include "line.h"
#include "point.h"
#include "rectangle.h"

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

// --- IsPointInArcBounds tests ---
//
// IsPointInArcBounds computes the angle from the arc's centre to the given point and
// checks whether it falls within [start_angle_deg_, end_angle_deg_] measured
// counter-clockwise. Angles follow the standard math convention:
//   0° = positive x-axis, 90° = positive y-axis, etc.

TEST(ArcTest, IsPointInArcBounds_PointOnStartAngle) {
  // Arc from 45° to 135°. A point at exactly 45° from the centre.
  Arc arc(Point(0, 0), 100, 45, 135);
  // Point at 45°: (r*cos45, r*sin45) ≈ (70, 70).
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(70, 70)));
}

TEST(ArcTest, IsPointInArcBounds_PointOnEndAngle) {
  // Arc from 45° to 135°. A point at exactly 135° from the centre.
  Arc arc(Point(0, 0), 100, 45, 135);
  // Point at 135°: (r*cos135, r*sin135) ≈ (-70, 70).
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(-70, 70)));
}

TEST(ArcTest, IsPointInArcBounds_PointInsideRange) {
  // Arc from 0° to 180°. A point at 90° (straight up).
  Arc arc(Point(0, 0), 100, 0, 180);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(0, 50)));
}

TEST(ArcTest, IsPointInArcBounds_PointOutsideRange) {
  // Arc from 0° to 90° (first quadrant only). A point at 180° (negative x).
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-50, 0)));
}

TEST(ArcTest, IsPointInArcBounds_FullCircle) {
  // Full circle: any point should be in bounds.
  Arc arc(Point(0, 0), 100, 0, 360);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(100, 0)));    // 0°
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(0, 100)));    // 90°
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(-100, 0)));   // 180°
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(0, -100)));   // 270°
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(70, -70)));   // 315°
}

TEST(ArcTest, IsPointInArcBounds_QuadrantI) {
  // Arc spanning only quadrant I: 0° to 90°.
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(70, 70)));     // ~45°, inside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-70, 70)));   // ~135°, outside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-70, -70)));  // ~225°, outside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, IsPointInArcBounds_QuadrantII) {
  // Arc spanning quadrant II: 90° to 180°.
  Arc arc(Point(0, 0), 100, 90, 180);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(-70, 70)));    // ~135°, inside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, 70)));    // ~45°, outside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, IsPointInArcBounds_QuadrantIII) {
  // Arc spanning quadrant III: 180° to 270°.
  Arc arc(Point(0, 0), 100, 180, 270);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(-70, -70)));   // ~225°, inside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, 70)));    // ~45°, outside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, -70)));   // ~315°, outside
}

TEST(ArcTest, IsPointInArcBounds_QuadrantIV) {
  // Arc spanning quadrant IV: 270° to 360°.
  Arc arc(Point(0, 0), 100, 270, 360);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(70, -70)));    // ~315°, inside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-70, -70)));  // ~225°, outside
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(70, 70)));    // ~45°, outside
}

TEST(ArcTest, IsPointInArcBounds_SpanningZero) {
  // Arc from 315° to 45° (crossing the 0° boundary).
  // This is the tricky wraparound case where end < start numerically.
  Arc arc(Point(0, 0), 100, 315, 405);
  // Point at 0° (positive x-axis) should be inside.
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(100, 0)));
  // Point at 350° should be inside.
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(98, -17)));
  // Point at 30° should be inside.
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(87, 50)));
  // Point at 180° should be outside.
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-100, 0)));
  // Point at 90° should be outside.
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(0, 100)));
}

TEST(ArcTest, IsPointInArcBounds_NarrowArc) {
  // A very narrow arc: 89° to 91°.
  Arc arc(Point(0, 0), 100, 89, 91);
  // Point at 90° (straight up) should be inside.
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(0, 100)));
  // Point at 80° should be outside.
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(17, 98)));
  // Point at 100° should be outside.
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(-17, 98)));
}

TEST(ArcTest, IsPointInArcBounds_OffsetCentre) {
  // Arc centred at (100, 200), from 0° to 90°.
  // IsPointInArcBounds should measure the angle relative to the centre, not the origin.
  Arc arc(Point(100, 200), 50, 0, 90);
  // Point at ~45° relative to centre: centre + (35, 35).
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(135, 235)));
  // Point at ~135° relative to centre: centre + (-35, 35).
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(65, 235)));
  // Point at ~225° relative to centre: centre + (-35, -35).
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(65, 165)));
}

TEST(ArcTest, IsPointInArcBounds_PointOnAxis) {
  // Arc from 0° to 180°. Check points exactly on the axes.
  Arc arc(Point(0, 0), 100, 0, 180);
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(100, 0)));     // 0° (start boundary)
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(0, 100)));     // 90° (middle)
  EXPECT_TRUE(arc.IsPointInArcBounds(Point(-100, 0)));    // 180° (end boundary)
  EXPECT_FALSE(arc.IsPointInArcBounds(Point(0, -100)));   // 270° (outside)
}

// --- PointOnArcAtAngle tests ---
//
// PointOnArcAtAngle takes an angle in degrees and returns the point on the arc
// at that angle: centre + (r*cos(angle), r*sin(angle)).

TEST(ArcTest, PointOnArcAtAngle_ZeroDegrees) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(0);
  // cos(0) = 1, sin(0) = 0.
  EXPECT_NEAR(100, p.x(), 1);
  EXPECT_NEAR(0, p.y(), 1);
}

TEST(ArcTest, PointOnArcAtAngle_90Degrees) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(90);
  // cos(90) = 0, sin(90) = 1.
  EXPECT_NEAR(0, p.x(), 1);
  EXPECT_NEAR(100, p.y(), 1);
}

TEST(ArcTest, PointOnArcAtAngle_180Degrees) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(180);
  EXPECT_NEAR(-100, p.x(), 1);
  EXPECT_NEAR(0, p.y(), 1);
}

TEST(ArcTest, PointOnArcAtAngle_270Degrees) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(270);
  EXPECT_NEAR(0, p.x(), 1);
  EXPECT_NEAR(-100, p.y(), 1);
}

TEST(ArcTest, PointOnArcAtAngle_45Degrees) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(45);
  double v = 100.0 / std::sqrt(2.0);
  EXPECT_NEAR(v, p.x(), 2);
  EXPECT_NEAR(v, p.y(), 2);
}

TEST(ArcTest, PointOnArcAtAngle_OffsetCentre) {
  Arc arc(Point(50, 30), 100, 0, 360);
  Point p = arc.PointOnArcAtAngle(0);
  // Should be centre + (100, 0) = (150, 30).
  EXPECT_NEAR(150, p.x(), 1);
  EXPECT_NEAR(30, p.y(), 1);
}

// --- Start() and End() tests ---

TEST(ArcTest, Start_AtZeroDegrees) {
  Arc arc(Point(0, 0), 100, 0, 90);
  Point s = arc.Start();
  EXPECT_NEAR(100, s.x(), 1);
  EXPECT_NEAR(0, s.y(), 1);
}

TEST(ArcTest, Start_At90Degrees) {
  Arc arc(Point(0, 0), 100, 90, 180);
  Point s = arc.Start();
  EXPECT_NEAR(0, s.x(), 1);
  EXPECT_NEAR(100, s.y(), 1);
}

TEST(ArcTest, Start_OffsetCentre) {
  Arc arc(Point(50, 30), 100, 0, 90);
  Point s = arc.Start();
  EXPECT_NEAR(150, s.x(), 1);
  EXPECT_NEAR(30, s.y(), 1);
}

TEST(ArcTest, End_At90Degrees) {
  Arc arc(Point(0, 0), 100, 0, 90);
  Point e = arc.End();
  EXPECT_NEAR(0, e.x(), 1);
  EXPECT_NEAR(100, e.y(), 1);
}

TEST(ArcTest, End_At180Degrees) {
  Arc arc(Point(0, 0), 100, 90, 180);
  Point e = arc.End();
  EXPECT_NEAR(-100, e.x(), 1);
  EXPECT_NEAR(0, e.y(), 1);
}

TEST(ArcTest, End_At270Degrees) {
  Arc arc(Point(0, 0), 100, 180, 270);
  Point e = arc.End();
  EXPECT_NEAR(0, e.x(), 1);
  EXPECT_NEAR(-100, e.y(), 1);
}

TEST(ArcTest, StartAndEnd_FullCircle) {
  // For a full circle (0 to 360, stored as 0 to 0 after modulo), Start and End
  // should both be at 0° = (r, 0).
  Arc arc(Point(0, 0), 100, 0, 360);
  Point s = arc.Start();
  Point e = arc.End();
  EXPECT_NEAR(s.x(), e.x(), 1);
  EXPECT_NEAR(s.y(), e.y(), 1);
}

TEST(ArcTest, StartAndEnd_ConsistentWithPointOnArcAtAngle) {
  // Start() and End() should return the same points as PointOnArcAtAngle
  // called with the start and end angles.
  Arc arc(Point(20, 10), 80, 30, 150);
  Point s = arc.Start();
  Point e = arc.End();
  Point s_manual = arc.PointOnArcAtAngle(30);
  Point e_manual = arc.PointOnArcAtAngle(150);
  EXPECT_EQ(s.x(), s_manual.x());
  EXPECT_EQ(s.y(), s_manual.y());
  EXPECT_EQ(e.x(), e_manual.x());
  EXPECT_EQ(e.y(), e_manual.y());
}

// --- Intersects(const Point &) tests ---
//
// Intersects checks that the point is within the arc's radius AND within
// the arc's angular bounds.

TEST(ArcTest, Intersects_PointAtCentre) {
  // The centre is distance 0 from itself, which is <= radius.
  // Angle is ill-defined but atan2(0,0) = 0, which is within [0, 90].
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_TRUE(arc.Intersects(Point(0, 0)));
}

TEST(ArcTest, Intersects_PointInsideArcRegion) {
  // Arc from 0° to 90°, radius 100.
  Arc arc(Point(0, 0), 100, 0, 90);
  // Point at ~45°, distance 70 < 100.
  EXPECT_TRUE(arc.Intersects(Point(50, 50)));
}

TEST(ArcTest, Intersects_PointOnArcBoundary) {
  // Point exactly on the arc (distance = radius, within angle).
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_TRUE(arc.Intersects(Point(100, 0)));
}

TEST(ArcTest, Intersects_PointOutsideRadius) {
  // Point at correct angle but beyond radius.
  Arc arc(Point(0, 0), 100, 0, 90);
  EXPECT_FALSE(arc.Intersects(Point(200, 0)));
}

TEST(ArcTest, Intersects_PointInsideRadiusButOutsideAngle) {
  // Point within radius but at an angle outside the arc's range.
  Arc arc(Point(0, 0), 100, 0, 90);
  // Point at 180° (negative x), distance 50 < 100.
  EXPECT_FALSE(arc.Intersects(Point(-50, 0)));
}

TEST(ArcTest, Intersects_PointOutsideRadiusAndAngle) {
  Arc arc(Point(0, 0), 100, 0, 90);
  // Point at ~225°, distance > 100.
  EXPECT_FALSE(arc.Intersects(Point(-200, -200)));
}

TEST(ArcTest, Intersects_FullCircle) {
  Arc arc(Point(0, 0), 100, 0, 360);
  // Any point within radius should intersect regardless of angle.
  EXPECT_TRUE(arc.Intersects(Point(50, 0)));
  EXPECT_TRUE(arc.Intersects(Point(-50, 0)));
  EXPECT_TRUE(arc.Intersects(Point(0, 50)));
  EXPECT_TRUE(arc.Intersects(Point(0, -50)));
  EXPECT_TRUE(arc.Intersects(Point(70, 70)));
  // Just outside.
  EXPECT_FALSE(arc.Intersects(Point(101, 0)));
}

TEST(ArcTest, Intersects_OffsetCentre) {
  Arc arc(Point(100, 100), 50, 0, 90);
  // Inside the arc region: centre + (30, 30), distance ~42 < 50, angle ~45°.
  EXPECT_TRUE(arc.Intersects(Point(130, 130)));
  // Inside radius but wrong angle: centre + (-30, -30), angle ~225°.
  EXPECT_FALSE(arc.Intersects(Point(70, 70)));
  // Outside radius in the right angle: centre + (60, 0), distance 60 > 50.
  EXPECT_FALSE(arc.Intersects(Point(160, 100)));
}

TEST(ArcTest, Intersects_SpanningZeroBoundary) {
  // Arc from 315° to 45° (wraps around 0°).
  Arc arc(Point(0, 0), 100, 315, 405);
  // Point at 0°, distance 50. Should be inside.
  EXPECT_TRUE(arc.Intersects(Point(50, 0)));
  // Point at 350°, distance ~50. Should be inside.
  EXPECT_TRUE(arc.Intersects(Point(49, -8)));
  // Point at 90°, distance 50. Outside angular range.
  EXPECT_FALSE(arc.Intersects(Point(0, 50)));
}

// --- Overlaps(const Rectangle &) tests ---

TEST(ArcTest, Overlaps_RectangleClearlyInside) {
  // Full circle, rectangle entirely inside.
  Arc arc(Point(0, 0), 100, 0, 360);
  Rectangle rect(Point(-10, -10), Point(10, 10));
  EXPECT_TRUE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleClearlyOutside) {
  Arc arc(Point(0, 0), 100, 0, 360);
  Rectangle rect(Point(200, 200), Point(300, 300));
  EXPECT_FALSE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleCrossingArcBoundary) {
  // Rectangle straddles the arc boundary (partially inside, partially outside).
  Arc arc(Point(0, 0), 100, 0, 360);
  Rectangle rect(Point(80, -10), Point(120, 10));
  EXPECT_TRUE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleOutsideAngularRange) {
  // Quarter-circle arc in quadrant I. Rectangle in quadrant III.
  Arc arc(Point(0, 0), 100, 0, 90);
  Rectangle rect(Point(-80, -80), Point(-60, -60));
  EXPECT_FALSE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleInsideRadiusButOutsideAngle) {
  // Quarter-circle in quadrant I. Rectangle in quadrant II, within radius.
  Arc arc(Point(0, 0), 100, 0, 90);
  Rectangle rect(Point(-50, 10), Point(-30, 30));
  EXPECT_FALSE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleTouchingArcEdge) {
  // Arc from 0° to 90°. Rectangle abutting the arc's straight boundary along
  // the positive x-axis (the start boundary line from centre to Start()).
  Arc arc(Point(0, 0), 100, 0, 90);
  // Rectangle sitting just below the x-axis, touching it.
  Rectangle rect(Point(20, -20), Point(80, 0));
  EXPECT_TRUE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_RectangleContainsEntireArc) {
  // A large rectangle that fully contains the arc.
  Arc arc(Point(0, 0), 50, 0, 90);
  Rectangle rect(Point(-100, -100), Point(100, 100));
  EXPECT_TRUE(arc.Overlaps(rect));
}

TEST(ArcTest, Overlaps_OffsetCentre) {
  Arc arc(Point(100, 100), 50, 0, 360);
  // Rectangle clearly overlapping.
  Rectangle overlap(Point(90, 90), Point(110, 110));
  EXPECT_TRUE(arc.Overlaps(overlap));
  // Rectangle far away.
  Rectangle miss(Point(300, 300), Point(400, 400));
  EXPECT_FALSE(arc.Overlaps(miss));
}

}  // namespace geometry
}  // namespace bfg
