#include <optional>
#include <glog/logging.h>
#include <cmath>
#include <sstream>

#include "line.h"
#include "point.h"
#include "vector.h"

namespace bfg {
namespace geometry {

double Line::kPi = std::acos(-1);

std::string Line::Describe() const {
  std::stringstream ss;
  ss << start_ << " -> " << end_;
  return ss.str();
}

bool Line::AreAntiParallel(const Line &lhs, const Line &rhs) {
  return lhs.AngleToLine(rhs) == kPi;
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

  // FIXME(aryap): What about antiparallel lines?
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

bool Line::IntersectsWithAny(
    const std::vector<Line> &lines,
    IntersectionInfo *any_intersection) const {
  for (const Line &line : lines) {
    if (Line::Intersect(*this,
                        line,
                        &any_intersection->incident,
                        &any_intersection->point)) {
      return true;
    }
  }
  return false;
}

void Line::IntersectsWithAll(
    const std::vector<Line> &lines,
    std::vector<IntersectionInfo> *intersections) const {
  for (const Line &line : lines) {
    IntersectionInfo info;
    if (Intersects(line, &info)) {
      intersections->push_back(info);
    }
  }
}

// Projection of some point on to the line. Let vector v be the the vector from
// the start of this line to the given point, and the vector s be the vector
// from the start of *this line to the end. Then
//
//      _ _      _   _   _   _     _
// proj_s(v) = [(v . s)/(s . s)] * s
//
//                _   _   _   _  
// And we return (v . s)/(s . s).
//
// If the project is negative, the vector faces in the opposite direction of
// the original.
double Line::ProjectionCoefficient(const Point &point) const {
  int64_t v_dot_s = DotProduct(Line(start_, point));  // *this dot (start,
                                                      // point)
  int64_t s_dot_s = DotProduct(*this);                // *this dot *this
  double coeff = static_cast<double>(v_dot_s) / static_cast<double>(s_dot_s);
  return coeff;
}

std::optional<Line> Line::ExtendToNearestIntersection(
    const std::vector<Line> &intersectors) const {
  std::vector<IntersectionInfo> intersections;
  IntersectsWithAll(intersectors, &intersections);
  if (intersections.empty()) {
    return std::nullopt;
  }

  // Choose closest intersection to the start but not before it, assuming that
  // since none of the intersections occur within the line, this is the closest
  // intersection to the end (saves us having to measure distance to the end
  // explicltly).
  const Point *point = nullptr;
  double projection_coefficient = 0;
  for (const IntersectionInfo &info : intersections) {
    double candidate_coefficient = ProjectionCoefficient(info.point);
    if (candidate_coefficient < 0.0) {
      // Skip projection that go behind start_ on *this line. We want
      // intersections that appear out 
      continue;
    }
    LOG_IF(WARNING, candidate_coefficient < 1.0)
        << "Lines seem to intersect current line rather than need extensions";
    if (!point || candidate_coefficient < projection_coefficient) {
      point = &info.point;
      projection_coefficient = candidate_coefficient;
    }
  }

  return Line(end_, *point);
}

//   (lines defining boundary)
//  +------------------------------7------------------------+
//  |                             /                         |
//  |                            / (extension to boundary)  |
//  |                           L                           |
//  |                    (end) 7                            |
//  |                         /                             |
//  |                        /                              |
//  |                       /                               |
//  |                      /                                |
//  |                     / (line)                          |
//  |                    /                                  |
//  |                   /                                   |
//  |          (start) +                                    |
//  |                 7                                     |
//  |                /                                      |
//  |               / (other extension to boundary)         |
//  |              /                                        |
//  +-------------L-----------------------------------------+
//
// Assumes that the given line doesn't actually intersect any of the boundary
// lines. If it does the picture would be quite different.
void Line::GetExtensionsToBoundaries(
    const std::vector<Line> &boundaries,
    std::vector<Line> *extensions) const {
  Line reversed = Reversed();
  std::vector<const Line*> forward_and_backward = {this, &reversed};
  for (const Line *line : forward_and_backward) {
    std::optional<Line> first = line->ExtendToNearestIntersection(boundaries);
    if (first) {
      extensions->push_back(first.value());
    }
  }
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

  // TODO(aryap): What about anti-incident? Put that in IntersectionInfo and use
  // it here.
  if (*incident) {
    // Check if the other line overlaps this line.

    // Convert all points to scalar distances along the mutual line:
    //
    //                    start
    // start --------->     -----------> end
    //               end
    //       |
    //       v
    //       0
    //       |--------|-----|----------|---->

    double start = 0.0;
    double end = ProjectionCoefficient(end_);
    double other_start = ProjectionCoefficient(other.start());
    double other_end = ProjectionCoefficient(other.end());

    // TODO(aryap): I swear I've written this somewhere else. Refactor.
    if (other_start > end || start > other_end) {
      // No intersection.
      return false;
    } else if (start >= other_start) {
      *point = start_;
      return true;
    } else if (other_start >= start) {
      *point = other.start();
      return true;
    }
    LOG(FATAL) << "Did not account for overlap case where this: " << *this
               << " other: " << other;
  }

  if (!other.IntersectsInBounds(intersection_in_our_bounds))
    return false;
 
  return true;
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

// This can return a positive or negative angle.
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

// TODO(aryap): Is this right...?
// Always returns an angle in [0, 2pi].
//
// The convention for reading this is as follow. Given two lines a and b:
//
//       b
//     /
//    / _
//   / |\
//  /    ) theta
// ----------- a
//
// The angle "a to b" is the rotation from a to b. The angle "from b to a" is
// the rotation the other way, always counter-clockwise:
//
//            b
//          /
//     __  /
//    /  \/
//   /   /
//   \  ----------- a
//    \_/|
//
// i.e. the angle "from a to b" and the angle "from b to a" always sum to 2 *
// pi.
double Line::AngleToLine(const Line &other) const {
  double angle_rads = other.AngleToHorizon() - AngleToHorizon();
  if (angle_rads < 0) {
    angle_rads += 2 * kPi;
  }
  return angle_rads;
}

int64_t Line::DotProduct(const Line &with) const {
  // Turn the lines into vectors by subtracting the starting point from the end
  // point. Call them "Vectors" to make it clear what we're doing, even though
  // Vectors are just Points (i.e. Points are Vectors from the origin (0, 0)).
  Vector a = end_ - start_;
  Vector b = with.end() - with.start();
  return a.x() * b.x() + a.y() * b.y();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Line &point) {
  os << point.start() << " -> " << point.end();
  return os;
}

}  // namespace bfg
