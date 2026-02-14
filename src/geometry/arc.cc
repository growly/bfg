#include "arc.h"

#include <cmath>
#include <glog/logging.h>
#include <sstream>
#include <vector>

#include "../utility.h"
#include "point.h"
#include "radian.h"
#include "rectangle.h"
#include "shape.h"
#include "line.h"

namespace bfg {
namespace geometry {

const Rectangle Arc::GetBoundingBox() const {
  LOG(FATAL) << "Unimplemented.";
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
      result.emplace_back(k, y_value);
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
    result.emplace_back(x_value, m*x_value + c);
  }
  return result;
}

bool Arc::InArcBounds(const Point &point) const {
  double angle_rad = std::atan2(
      static_cast<double>(point.y() - centre_.y()),
      static_cast<double>(point.x() - centre_.x()));

  if (angle_rad < 0) {
    angle_rad += 2 * Radian::kPi;
  }

  double angle_deg = Radian::RadiansToDegrees(angle_rad);

  if (end_angle_deg_ > start_angle_deg_) {
    return angle_deg >= start_angle_deg_ &&
           angle_deg <= end_angle_deg_;
  }
  return (angle_deg >= start_angle_deg_ && angle_deg < 360.0) ||
         (angle_deg >= 0 && angle_deg <= end_angle_deg_);
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
