#ifndef GEOMETRY_LINE_H_
#define GEOMETRY_LINE_H_

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
  Line() = default;
  Line(const Point &start, const Point &end)
      : start_(start), end_(end) {}

  static double kPi;

  // Returns true if the lines defined by lhs and rhs intersect, and if so,
  // fills `point` with the intersection point. Returns false if the lines do
  // not intersect (are parallel).
  static bool Intersect(
      const Line &lhs, const Line &rhs, bool *incident, Point *point);

  static bool AreSameInfiniteLine(const Line &lhs, const Line &rhs);

  bool Intersects(const Point &point) const;

  bool Intersects(const Line &other, bool *incident, Point *point) const {
    return Intersect(*this, other, incident, point);
  }

  bool IntersectsInBounds(const Point &point) const;

  bool IntersectsInBounds(const Line &other, bool *incident, Point *point) const;

  bool IsSameInfiniteLine(const Line &other) const {
    return AreSameInfiniteLine(*this, other);
  }

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
