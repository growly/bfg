#ifndef LINE_H_
#define LINE_H_

#include <utility>

#include "point.h"

namespace bfg {
namespace geometry {

// This is a line as defined by two points. It's actually a vector, and is
// sometimes treated as such, and sometimes we take the extension of the line
// on which that vector falls.
class Line {
 public:
  Line() = default;
  Line(const Point &start, const Point &end)
      : start_(start), end_(end) {}

  static double kPi;

  // Returns true if the lines defined by lhs and rhs intersect, and if so,
  // fills `point` with the intersection point. Returns false if the lines do
  // not intersect (are parallel).
  static bool Intersect(const Line &lhs, const Line &rhs, Point *point);

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

  // Returns 'm' in y = m*x + c;
  double Gradient() const;

  // Returns 'c' in y = m*x + c;
  double Offset() const;

  // Find angle between our line and the x-axis, where 0 means pointing
  // horizontally to the right. Returns the angle in radians.
  double AngleToHorizon() const;

  void set_start(const Point &start) { start_ = start; }
  void set_end(const Point &end) { end_ = end; }

  const Point &start() const { return start_; }
  const Point &end() const { return end_; }

 private:
  Point start_;
  Point end_;
};

std::ostream &operator<<(std::ostream &os, const Line &point);

}  // namespace geometry
}  // namespace bfg

#endif  // LINE_H_
