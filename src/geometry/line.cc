#include <glog/logging.h>
#include <cmath>
#include <sstream>

#include "line.h"
#include "point.h"

namespace bfg {
namespace geometry {

double Line::kPi = std::acos(-1);

std::string Line::Describe() const {
  std::stringstream ss;
  ss << start_ << " -> " << end_;
  return ss.str();
}

bool Line::Intersect(
    const Line &lhs, const Line &rhs, bool *incident, Point *point) {
  // (1) y1 = m1*x1 + c1
  // (2) y2 = m2*x2 + c2
  //
  // Set y's equal and solve for x:
  // xx = (c1 - c2)/(m2 - m1)
  // yy = m1*xx + c1
  *incident = false;

  if (lhs.IsVertical() && rhs.IsVertical()) {
    // Both lines are vertical. Check if they are at the same x:
    if (lhs.start().x() == rhs.start().x()) {
      // The intersection of these lines is each other, so indicate that they
      // are incident on each other and do not store a specific point.
      *incident = true;
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

  if (m1 == m2) {
    // Return true if the offsets are the same, since that means the two lines
    // are the same.
    if (c1 == c2) {
      *incident = true;
      return true;
    }
    return false;
  }

  double x = (c1 - c2)/(m2 - m1);
  double y = m1*x + c1;
  *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
  return true;
}

bool Line::Intersects(const Point &point) const {
  if (IsVertical()) {
    return point.x() == start_.x();
  }
  // Numerous ways to skin this, including finding the more general distance of
  // the point to the line, but I think this is faster:
  double y_hypothetical = Gradient() * point.x() + Offset();
  double y_error = std::abs(static_cast<double>(point.y()) - y_hypothetical);
  // Since our unit resolution is 1, we consider that the limit of error:
  return y_error < 1.0;
}

bool Line::AreSameInfiniteLine(const Line &lhs, const Line &rhs) {
  if (lhs.IsVertical() && rhs.IsVertical()) {
    // Both lines are vertical. Check if they are at the same x:
    return lhs.start().x() == rhs.start().x();
  } else if (lhs.IsVertical()) {
    return false;
  } else if (rhs.IsVertical()) {
    return false;
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

bool Line::IntersectsInBounds(const Point &point) const {
  if (!Intersects(point))
    return false;

  // This is a very common operation of order the two ys or xs:
  int64_t max_y = std::max(start_.y(), end_.y());
  int64_t min_y = std::min(start_.y(), end_.y());
  int64_t max_x = std::max(start_.x(), end_.x());
  int64_t min_x = std::min(start_.x(), end_.x());

  if (min_x <= point.x() && point.x() <= max_x &&
      min_y <= point.y() && point.y() <= max_y) {
    return true;
  }
  return false;
}

bool Line::IntersectsInBounds(
    const Line &other,
    bool *incident,
    bool *is_start_or_end,
    Point *point,
    bool ignore_end,
    bool ignore_start) const {
  Point intersection;
  if (!Intersects(other, incident, &intersection))
    return false;

  if (*incident)
    return true;

  int64_t max_y = std::max(start_.y(), end_.y());
  int64_t min_y = std::min(start_.y(), end_.y());
  int64_t max_x = std::max(start_.x(), end_.x());
  int64_t min_x = std::min(start_.x(), end_.x());

  if (min_x <= intersection.x() && intersection.x() <= max_x &&
      min_y <= intersection.y() && intersection.y() <= max_y) {
    *point = intersection;

    if (intersection == start_) {
      if (ignore_start) {
        return false;
      }
      *is_start_or_end = true;
    } else if (intersection == end_) {
      if (ignore_end) {
        return false;
      }
      *is_start_or_end = true;
    }
    return true;
  }
  return false;
}

bool Line::IntersectsInMutualBounds(
    const Line &other, bool *incident, Point *point) const {
  Point intersection_in_our_bounds;
  bool ignored;
  if (!IntersectsInBounds(
          other, incident, &ignored, &intersection_in_our_bounds))
    return false;

  if (*incident) {
    // Check if the other line overlaps this line.

    // Find the projection of each point onto the shared line.
    //      _ _      _   _   _   _     _
    // proj_s(v) = [(v . s)/(s . s)] * s

  }

  // TODO(growly): This is incomplete?
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
  int64_t divisor = end_.x() - start_.x();
  LOG_IF(FATAL, divisor == 0)
      << "This is a vertical line; do not compute gradient.";
  return static_cast<double>(end_.y() - start_.y()) /
         static_cast<double>(divisor);
}

double Line::Length() const {
  return start_.L2DistanceTo(end_);
}

Point Line::PointOnLineAtDistance(const Point &start, double distance) const {
  LOG_IF(WARNING, !Intersects(start))
      << "Point " << start << " is not on this line";

  if (distance == 0.0)
    return start;

  //           end
  //           /
  //          o <- want this point
  //         / distance, d
  //        x ---- horizon
  //       / -d
  //      o
  //     /
  //   start

  double theta = AngleToHorizon();
  int64_t dx = std::round(distance * std::cos(theta));
  int64_t dy = std::round(distance * std::sin(theta));
  return start + Point(dx, dy);
}

double Line::Offset() const {
  return end_.y() - Gradient() * end_.x();
}

double Line::AngleToHorizon() const {
  double dx = end_.x() - start_.x();
  double dy = end_.y() - start_.y();

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
  LOG(FATAL) << "Not implemented.";
  return 0.0;
}

int64_t Line::DotProduct(const Line &with) const {
  // Turn the lines into vectors by subtracting the starting point from the end
  // point:
  Point a = end_ - start_;
  Point b = with.end() - with.start();
  return a.x() * b.x() + a.y() * b.y();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Line &point) {
  os << point.start() << " -> " << point.end();
  return os;
}

}  // namespace bfg
