#include "arc.h"

#include <cmath>
#include <glog/logging.h>
#include <sstream>
#include <vector>

#include "../utility.h"
#include "line.h"
#include "point.h"
#include "polygon.h"
#include "radian.h"
#include "rectangle.h"
#include "shape.h"

namespace bfg {
namespace geometry {

Arc::Arc()
    : centre_(Point(0, 0)),
      radius_(0),
      start_angle_deg_(0),
      end_angle_deg_(0) {}

const Rectangle Arc::GetBoundingBox() const {
  Rectangle bounding_box;

  int64_t min_x = std::min({centre_.x(), Start().x(), End().x()});
  int64_t min_y = std::min({centre_.y(), Start().y(), End().y()});
  int64_t max_x = std::max({centre_.x(), Start().x(), End().x()});
  int64_t max_y = std::max({centre_.y(), Start().y(), End().y()});

  if (IsAngleInArcBoundsDegrees(0)) {
    max_x = centre_.x() + radius_;
  }
  if (IsAngleInArcBoundsDegrees(90)) {
    max_y = centre_.y() + radius_;
  }
  if (IsAngleInArcBoundsDegrees(180)) {
    min_x = centre_.x() - radius_;
  }
  if (IsAngleInArcBoundsDegrees(270)) {
    min_y = centre_.y() - radius_;
  }
  return Rectangle({min_x, min_y}, {max_x, max_y});
}

std::vector<Point> Arc::IntersectingPoints(const Line &line) const {
  // Equation of circle is
  //
  //  (x - x_c)^2 + (y - y_c)^2 = r^2
  //
  // where x_c, y_c = centre_ and r = radius_.
  double x_c = centre_.x();
  double y_c = centre_.y();
  double r = radius_;
 
  // If vertical, we substitute x = k into the circle equation:
  //
  //                (k - x_c)^2 + (y - y_c)^2 = r^2 
  //                     y^2 - 2y_c*y + y_c^2 = r^2 - (k - x_c)^2
  // y^2 - 2y_c*y + y_c^2 - r^2 + (k - x_c)^2 = 0
  //
  // which is quadratic
  //                            Ay^2 + By + C = 0
  //
  // with A = 1
  //      B = -2y_c
  //      C = y_c^2 - r^2 + (k - x_c)^2
  //
  // so we just solve the quadratic formula:
  //      y = (-B +- sqrt(B^2 - 4AC)) / 2A
  if (line.IsVertical()) {
    double k = line.start().x();
    double B = -2.0 * y_c;
    double C = y_c * y_c - r * r + (k - x_c) * (k - x_c);
    std::vector<double> y_values = Utility::SolveQuadraticReal(1.0, B, C);

    std::vector<Point> result;
    for (double y_value : y_values) {
      Point point(k, y_value);
      if (IsPointInArcBounds(point)) {
        result.push_back(point);
      }
    }
    return result;
  }

  double m = line.Gradient();
  // Little c! The line's offset at the y-intercept.
  double c = line.Offset();
    
  // If not vertical, equation of line is y = mx + c.
  //
  // Substituting y into the circle equation,
  //
  //                                (x - x_c)^2 + (mx + (c - y_c))^2 = r^2
  //     x^2 -2x_c*x + x_c^2 + m^2*x^2 + 2m(c - y_c)*x + (c - y_c)^2 = r^2
  // x^2 -2x_c*x + x_c^2 m^2*x^2 + 2m(c - y_c)*x + (c - y_c)^2 - r^2 = 0
  // (1 + m^2)x + (2m(c - y_c) - 2x_c)*x + x_c^2 + (c - y_c)^2 - r^2 = 0
  //
  // This is a quadratic in x,
  //                           Ax^2 + Bx + C = 0
  //
  // with A =  (1 + m^2)
  //      B = 2(mc - my_c - x_c)
  //      C = x_c^2 + (c - y_c)^2 - r^2
  double A = 1 + m*m;
  double B = 2*(m*(c - y_c) - x_c);
  double C = x_c * x_c + (c - y_c) * (c - y_c) - r * r;
  std::vector<double> x_values = Utility::SolveQuadraticReal(A, B, C);

  std::vector<Point> result;
  for (double x_value : x_values) {
    Point point(x_value, m*x_value + c);
    if (IsPointInArcBounds(point)) {
      result.push_back(point);
    }
  }
  return result;
}

std::vector<Point> Arc::IntersectingPointsInBounds(
    const Line &line) const {
  auto points = IntersectingPoints(line);
  std::vector<Point> in_bounds;
  for (const Point &point : points) {
    if (line.IntersectsInBounds(point)) {
      in_bounds.push_back(point);
    }
  }
  return in_bounds;
}
// This should handle rectangles at odd angles, even though using those would
// break a lot of other stuff.
bool Arc::Overlaps(const Rectangle &rectangle) const {
  if (!GetBoundingBox().Overlaps(rectangle)) {
    return false;
  }

  std::vector<Line> boundary_lines = rectangle.GetBoundaryLines();
  if (Intersects(boundary_lines)) {
    return true;
  }

  // It's possible that the rectangle contains the entire arc.
  Rectangle bounding_box = GetBoundingBox();
  if (rectangle.EntirelyContains(bounding_box)) {
    return true;
  }

  // The last possibility is that the other rectangle is contained entirely
  // within the arc.
  //
  // Since there are no boundary intersections, if any of the rectangle's four
  // corners are in the arc region, all of them are.
  return Intersects(rectangle.lower_left());
}

bool Arc::Overlaps(const Polygon &polygon) const {
  if (!GetBoundingBox().Overlaps(polygon.GetBoundingBox())) {
    return false;
  }

  std::vector<Line> boundary_lines = polygon.Edges();
  if (Intersects(boundary_lines)) {
    return true;
  }

  // It's possible that the entire polygon fits in the arc or that the entire
  // arc fits in the polygon.
  //
  // TODO(arypa): Until we have a Polygon triangulation + point intersection
  // method, this is too hard to answer. It's also usually unnecessary. So in
  // the interests of me graduating, I'll leave it for later.
  LOG(WARNING) << "Fudging overlap without proper test: " << *this << " and "
               << polygon;
  return true;
}

bool Arc::Intersects(const Point &other) const {
  double distance = centre_.L2DistanceTo(other);
  return distance <= radius_ && IsPointInArcBounds(other);
}

bool Arc::Intersects(const std::vector<Line> &lines) const {
  // Additionally test the two straight boundary lines on the arc:
  Line boundary_start = Line(centre_, Start());
  Line boundary_end = Line(centre_, End());

  for (const Line &line : lines) {
    std::vector<Point> intersections = IntersectingPointsInBounds(line);
    if (!intersections.empty()) {
      return true;
    }

    for (const Line &boundary : {boundary_start, boundary_end}) {
      bool incident;
      Point unused_point;
      if (boundary.IntersectsInMutualBounds(line, &incident, &unused_point)) {
        return true;
      }
    }
  }

  return false;
}

Point Arc::Start() const {
  return PointOnArcAtAngle(start_angle_deg_);
}

Point Arc::End() const {
  return PointOnArcAtAngle(end_angle_deg_);
}

bool Arc::IsPointInArcBounds(const Point &point) const {
  double angle_rad = std::atan2(
      static_cast<double>(point.y() - centre_.y()),
      static_cast<double>(point.x() - centre_.x()));

  if (angle_rad < 0) {
    angle_rad += 2 * Radian::kPi;
  }

  return IsAngleInArcBoundsRadians(angle_rad);
}

bool Arc::IsAngleInArcBoundsRadians(double angle_rad) const {
  double angle_deg = Radian::RadiansToDegrees(angle_rad);
  return IsAngleInArcBoundsDegrees(angle_deg);
}

bool Arc::IsAngleInArcBoundsDegrees(double angle_deg) const {
  if (end_angle_deg_ > start_angle_deg_) {
    return angle_deg >= start_angle_deg_ &&
           angle_deg <= end_angle_deg_;
  }
  return (angle_deg >= start_angle_deg_ && angle_deg < 360.0) ||
         (angle_deg >= 0 && angle_deg <= end_angle_deg_);
}

Point Arc::PointOnArcAtAngle(double angle_deg) const {
  double angle_rad = Radian::DegreesToRadians(angle_deg);
  int64_t y = static_cast<double>(radius_) * std::sin(angle_rad);
  int64_t x = static_cast<double>(radius_) * std::cos(angle_rad);
  return Point(x, y) + centre_;
}

std::string Arc::Describe() const {
  std::stringstream ss;
  ss << "[Arc " << centre_ << " r: " << radius_ << " degrees: ["
     << start_angle_deg_ << ", " << end_angle_deg_ << "]";
  return ss.str();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Arc &arc) {
  os << arc.Describe();
  return os;
}

}  // namespace bfg
