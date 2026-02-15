#ifndef GEOMETRY_ARC_H_
#define GEOMETRY_ARC_H_

#include <glog/logging.h>
#include <ostream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../modulo.h"
#include "line.h"
#include "point.h"
#include "shape.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

class Polygon;

// The angular range of the arc is determined by the start and end angles
// _counter clockwise_.
class Arc : public Shape {
 public:
  Arc();

  Arc(const Point &centre,
      int64_t radius,
      int32_t start_angle_deg,
      int32_t end_angle_deg)
      : centre_(centre),
        radius_(radius),
        start_angle_deg_(modulo(start_angle_deg, 360)),
        end_angle_deg_(modulo(end_angle_deg, 360)) {}

  void MirrorY() override { LOG(FATAL) << "Unimplemented."; }
  void MirrorX() override { LOG(FATAL) << "Unimplemented."; }
  void FlipHorizontal() override { LOG(FATAL) << "Unimplemented."; }
  void FlipVertical() override { LOG(FATAL) << "Unimplemented."; }
  void Translate(const Point &offset) override { LOG(FATAL) << "Unimplemented."; }
  void ResetOrigin() override { LOG(FATAL) << "Unimplemented."; }
  void Rotate(int32_t degrees_ccw) override { LOG(FATAL) << "Unimplemented."; }

  const Rectangle GetBoundingBox() const override;

  // Treating the line as infinite, return the points at which it intersects
  // the arc.
  std::vector<Point> IntersectingPoints(const Line &line) const;

  // Treating the line as bounded (by its start() and end()), return the points
  // on that line where it intersects the arc.
  std::vector<Point> IntersectingPointsInBounds(const Line &line) const;

  bool Overlaps(const Rectangle &rectangle) const;
  bool Overlaps(const Polygon &polygon) const;

  // Returns true if the point is within the region defined by the arc.
  //
  // Test if distance from point to arc is less than or equal to the arc's
  // radius, and also if the angle formed from the point to the centre is within
  // the start and end angles of the arc.
  bool Intersects(const Point &point) const;
  bool Intersects(const std::vector<Line> &lines) const;

  Point Start() const;
  Point End() const;

  std::string Describe() const;

 private:
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_ZeroDegrees);
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_90Degrees);
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_180Degrees);
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_270Degrees);
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_45Degrees);
  FRIEND_TEST(ArcTest, PointOnArcAtAngle_OffsetCentre);
  FRIEND_TEST(ArcTest, StartAndEnd_ConsistentWithPointOnArcAtAngle);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_PointOnStartAngle);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_PointOnEndAngle);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_PointInsideRange);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_PointOutsideRange);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_FullCircle);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_QuadrantI);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_QuadrantII);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_QuadrantIII);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_QuadrantIV);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_SpanningZero);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_NarrowArc);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_OffsetCentre);
  FRIEND_TEST(ArcTest, IsPointInArcBounds_PointOnAxis);

  // Test whether the angle made to the given point is within the angular
  // bounds of the arc.
  bool IsPointInArcBounds(const Point &point) const;

  bool IsAngleInArcBoundsRadians(double angle_rad) const;
  bool IsAngleInArcBoundsDegrees(double angle_deg) const;

  Point PointOnArcAtAngle(double angle_deg) const;

  Point centre_;
  int64_t radius_;
  int32_t start_angle_deg_;
  int32_t end_angle_deg_;
};
}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Arc &arc);

}  // namespace bfg

#endif  // GEOMETRY_ARC_H_
