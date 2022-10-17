#include <glog/logging.h>
#include <cmath>

#include "line.h"
#include "point.h"

namespace bfg {
namespace geometry {

double Line::kPi = std::acos(-1);

bool Line::Intersect(const Line &lhs, const Line &rhs, Point *point) {
  // (1) y1 = m1*x1 + c1
  // (2) y2 = m2*x2 + c2
  //
  // Set y's equal and solve for x:
  // xx = (c1 - c2)/(m2 - m1)
  // yy = m1*xx + c1

  if (lhs.IsVertical() && rhs.IsVertical()) {
    // Both lines are vertical. Check if they are at the same x:
    if (lhs.start().x() == rhs.start().x()) {
      // The intersection of these lines is each other, so we'll just return
      // the point between their starts and ends:
      double lhs_mid_y 
          = (lhs.end().y() - lhs.start().y())/2.0 + lhs.start().y();
      double rhs_mid_y 
          = (rhs.end().y() - rhs.start().y())/2.0 + rhs.start().y();
      double lhs_rhs_mid_y = (lhs_mid_y + rhs_mid_y) / 2.0;
      *point = Point(lhs.start().x(), lhs_rhs_mid_y);
      // TODO(aryap): This intersection point is arbitrary, so we can choose it
      // to be something more useful or simply something faster (i.e. just take
      // the end of the left-hand side).
      return true;
    }
    // Lines are vertical and parallel.
    return false;
  } else if (lhs.end().x() == lhs.start().x()) {
    // The left line is vertical.
    double x = static_cast<double>(lhs.end().x());
    double y = rhs.Gradient() * x + rhs.Offset();
    *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
    return true;
  } else if (rhs.end().x() == rhs.start().x()) {
    // Only the other line is vertical.
    double x = static_cast<double>(rhs.end().x());
    double y = lhs.Gradient() * x + lhs.Offset();
    *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
    return true;
  }

  double m1 = lhs.Gradient();
  double c1 = lhs.Offset();

  VLOG(11) << lhs.start() << " -> " << lhs.end()
           << ": y1 = " << m1 << "*x1 + " << c1;

  double m2 = rhs.Gradient();
  double c2 = rhs.Offset();

  VLOG(11) << rhs.start() << " -> " << rhs.end()
           << ": y2 = " << m2 << "*x2 + " << c2;

  // TODO(aryap): What if two overlapping horizontal lines are intersected?

  if (m1 == m2) {
    // Line are parallel.
    return false;
  }

  double x = (c1 - c2)/(m2 - m1);
  double y = m1*x + c1;
  *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
  return true;
}

bool Line::AreSameInfiniteLine(const Line &lhs, const Line &rhs) {
  if (lhs.IsVertical() && rhs.IsVertical()) {
    // Both lines are vertical. Check if they are at the same x:
    return lhs.start().x() == rhs.start().x();
  }
  // We can now ask for the gradient because it's not vertical (i.e. NaN):
  if (lhs.Gradient() != rhs.Gradient())
    return false;
  if (lhs.Offset() != rhs.Offset())
    return false;
  return true;
}

bool Line::IsVertical() const {
  return start_.x() == end_.x();
}

bool Line::IntersectsInBounds(const Line &other, Point *point) const {
  Point intersection;
  if (!Intersects(other, &intersection))
    return false;

  int64_t max_y = std::max(start_.y(), end_.y());
  int64_t min_y = std::min(start_.y(), end_.y());
  int64_t max_x = std::max(start_.x(), end_.x());
  int64_t min_x = std::min(start_.x(), end_.x());

  if (min_x <= intersection.x() && intersection.x() <= max_x &&
      min_y <= intersection.y() && intersection.y() <= max_y) {
    *point = intersection;
    return true;
  }
  return false;
}

//           _
//           /|
//          /
//         / theta
//        x------
//    dl /|
//      / | dy
//     x'_+
//       dx
void Line::StretchStart(int64_t dl) {
  double theta = AngleToHorizon();
  double dx = static_cast<double>(dl) * std::cos(theta);
  double dy = static_cast<double>(dl) * std::sin(theta);
  ShiftStart(-dx, -dy);
}

void Line::StretchEnd(int64_t dl) {
  double theta = AngleToHorizon();
  double dx = static_cast<double>(dl) * std::cos(theta);
  double dy = static_cast<double>(dl) * std::sin(theta);
  ShiftEnd(dx, dy);
}

void Line::ShiftStart(int64_t dx, int64_t dy) {
  start_.set_x(start_.x() + dx);
  start_.set_y(start_.y() + dy);
}

void Line::ShiftEnd(int64_t dx, int64_t dy) {
  end_.set_x(end_.x() + dx);
  end_.set_y(end_.y() + dy);
}

double Line::Gradient() const {
  return static_cast<double>(end_.y() - start_.y()) /
         static_cast<double>(end_.x() - start_.x());
}

double Line::Offset() const {
  return end_.y() - Gradient()*end_.x();
}

double Line::AngleToHorizon() const {
  int64_t dx = end_.x() - start_.x();
  int64_t dy = end_.y() - start_.y();

  double theta = 0;
  if (dx == 0) {
    theta = dy >= 0? kPi / 2.0 : -kPi / 2.0;
  } else if (dx < 0) {
    theta = kPi + std::atan(dy/dx);
  } else {
    theta = std::atan(dy/dx);
  }

  return theta;
}

//     /
//    /
//   /
//  / ) theta   a . b = ||a|| ||b|| cos (theta)
// -----------
double Line::AngleToLine(const Line &other) const {

}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Line &point) {
  os << point.start() << " -> " << point.end();
  return os;
}

}  // namespace bfg
