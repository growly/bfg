#ifndef GEOMETRY_LINE_H_
#define GEOMETRY_LINE_H_

#include <optional>
#include <utility>

#include "point.h"

namespace bfg {
namespace geometry {

// This is a line as defined by two points. It's also a vector from the start
// to the end point and is sometimes treated as such, and sometimes we take the
// extension of the line on which that vector falls.
// TODO(growly): do we need separate concepts for an infinite length line and a
// line connecting only two points?
class Line {
 public:
  // TODO(aryap): Use this everywhere?
  struct IntersectionInfo {
    bool does_intersect;
    bool incident;
    Point point;
  };

  Line() = default;
  Line(const Point &start, const Point &end)
      : start_(start), end_(end) {}

  static double kPi;

  // Returns true if the lines defined by lhs and rhs intersect, and if so,
  // fills `point` with the intersection point. Returns false if the lines do
  // not intersect (are parallel).
  static bool Intersect(
      const Line &lhs, const Line &rhs, bool *incident, Point *point);

  static bool AreAntiParallel(const Line &lhs, const Line &rhs);

  std::string Describe() const;

  static bool AreSameInfiniteLine(const Line &lhs, const Line &rhs);

  bool Intersects(const Point &point) const;

  bool IntersectsWithAny(
      const std::vector<Line> &lines,
      IntersectionInfo *any_intersection) const;

  void IntersectsWithAll(
      const std::vector<Line> &lines,
      std::vector<IntersectionInfo> *intersections) const;

  std::optional<Line> ExtendToNearestIntersection(
      const std::vector<Line> &intersectors) const;

  // TODO(aryap): I think this is now pointless?
  void GetExtensionsToBoundaries(
      const std::vector<Line> &boundaries,
      std::vector<Line> *extensions) const;

  bool Intersects(const Line &other, IntersectionInfo *intersection) const {
    intersection->does_intersect = Intersect(
        *this, other, &intersection->incident, &intersection->point);
    return intersection->does_intersect;
  }

  bool Intersects(const Line &other, bool *incident, Point *point) const {
    return Intersect(*this, other, incident, point);
  }

  bool IntersectsInBounds(const Point &point) const;

  // Tests if *this line intersects the other line within the bounds of this
  // line. The other line is treated as infinitely long.
  bool IntersectsInBounds(const Line &other,
                          bool *incident,
                          bool *is_start_or_end,
                          Point *point,
                          bool ignore_end = false,
                          bool ignore_start = false) const;

  bool IntersectsInMutualBounds(
      const Line &other, bool *incident, Point *point) const;

  bool IsSameInfiniteLine(const Line &other) const {
    return AreSameInfiniteLine(*this, other);
  }

  double Length() const;

  double ProjectionCoefficient(const Point &point) const;

  Point PointOnLineAtDistance(const Point &start, double distance) const;

  void Shift(int64_t dx, int64_t dy) {
    ShiftStart(dx, dy);
    ShiftEnd(dx, dy);
  }

  void ShiftStart(int64_t dx, int64_t dy);
  void ShiftEnd(int64_t dx, int64_t dy);

  // Move the start/end points dl units in the direction of the line, away from
  // the midpoint.
  void StretchStart(int64_t dl);
  void StretchEnd(int64_t dl);

  // Reverse the direction of this line.
  void Reverse() { std::swap(start_, end_); }

  Line Reversed() const {
    Line copy = *this;
    copy.Reverse();
    return copy;
  }

  // If 'm' is not a number in y = m*x + c;
  bool IsVertical() const;

  // Returns 'm' in y = m*x + c;
  double Gradient() const;

  // Returns 'c' in y = m*x + c;
  double Offset() const;

  // Find angle between our line and the x-axis, where 0 means pointing
  // horizontally to the right. Returns the angle in radians.
  double AngleToHorizon() const;

  // Angle in radians.
  double AngleToLine(const Line &other) const;

  int64_t DotProduct(const Line &with) const;

  void set_start(const Point &start) { start_ = start; }
  void set_end(const Point &end) { end_ = end; }

  const Point &start() const { return start_; }
  const Point &end() const { return end_; }

 private:
  Point start_;
  Point end_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Line &point);

}  // namespace bfg

#endif  // GEOMETRY_LINE_H_
