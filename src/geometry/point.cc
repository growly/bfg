#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "point.h"
#include "radian.h"

namespace bfg {
namespace geometry {

// TODO(aryap): Not sure why we'd every want to compare _only_ x or _only_ y,
// since that wouldn't guarantee consistent ordering between points with
// differing y or x values (respectively).
bool Point::CompareX(const Point &lhs, const Point &rhs) {
  return lhs.x() < rhs.x();
}

bool Point::CompareY(const Point &lhs, const Point &rhs) {
  return lhs.y() < rhs.y();
}

bool Point::CompareXThenY(const Point &lhs, const Point &rhs) {
  if (lhs.x() == rhs.x()) {
    return lhs.y() < rhs.y();
  }
  return lhs.x() < rhs.x();
}

bool Point::CompareYThenX(const Point &lhs, const Point &rhs) {
  if (lhs.y() == rhs.y()) {
    return lhs.x() < rhs.x();
  }
  return lhs.y() < rhs.y();
}

bool Point::ShareHorizontalOrVerticalAxis(
    const Point &lhs, const Point &rhs) {
  return lhs.x() == rhs.x() || lhs.y() == rhs.y();
}

Point Point::MidpointOf(const Point &lhs, const Point &rhs) {
  return Point((lhs.x() + rhs.x()) / 2, (lhs.y() + rhs.y()) / 2);
}

Point Point::PickMinY(const Point &lhs, const Point &rhs) {
  return lhs.y() <= rhs.y() ? lhs : rhs;
}

Point Point::PickMaxY(const Point &lhs, const Point &rhs) {
  return lhs.y() >= rhs.y() ? lhs : rhs;
}


Point Point::UnitVector(double angle_to_horizon_radians) {
  return {std::llround(std::cos(angle_to_horizon_radians)),
          std::llround(std::sin(angle_to_horizon_radians))};
}

Point &Point::operator+=(const Point &other) {
  x_ = x_ + other.x_;
  y_ = y_ + other.y_;
  return *this;
}

Point &Point::operator-=(const Point &other) {
  x_ = x_ - other.x_;
  y_ = y_ - other.y_;
  return *this;
}

void Point::MirrorY() {
  x_ = -x_;
}

void Point::MirrorX() {
  y_ = -y_;
}

void Point::Translate(const Point &offset) {
  x_ += offset.x_;
  y_ += offset.y_;
}

void Point::Rotate(int32_t degrees_ccw) {
  // Apparently they did this for us in C++20.
  constexpr double two_pi = 2 * 3.14159265358979323846;
  double theta = static_cast<double>(degrees_ccw)/360.0 * two_pi;
  Rotate(theta);
}

void Point::Rotate(double theta_radians) {
  // Rotate each of the corners by through the anti-clockwise angle theta.
  // If we had a linear algebra library available we'd multiply by the rotation
  // matrix.
  //
  // But we don't, so we use the fact that:
  //  x' = x cos(theta) - y sin(theta)
  //  y' = x sin(theta) + y cos(theta)
  double x = x_;
  double y = y_;
  x_ = std::llround(x * std::cos(theta_radians) - y * std::sin(theta_radians));
  y_ = std::llround(x * std::sin(theta_radians) + y * std::cos(theta_radians));
}

void Point::Scale(double scalar) {
  x_ = std::llround(static_cast<double>(x_) * scalar);
  y_ = std::llround(static_cast<double>(y_) * scalar);
}

// The L-1 norm, or Manhattan distance.
int64_t Point::L1DistanceTo(const Point &point) const {
  int64_t dx = point.x() - x_;
  int64_t dy = point.y() - y_;
  return std::abs(dx) + std::abs(dy);
}

void Point::AddComponents(double amount, double angle_rads) {
  int64_t x = std::llround(amount * std::cos(angle_rads));
  int64_t y = std::llround(amount * std::sin(angle_rads));
  x_ += x;
  y_ += y;
}

int64_t Point::Component(double angle_rads) const {
  // Shortcuts for common uses.
  //if (angle_rads == 0.0) {
  //  return x_;
  //} else if (angle_rads == Radian::kPi / 2) {
  //  return y_;
  //}
  return std::llround(UnitVector(angle_rads).ProjectionCoefficient(*this));
}

//                          _      _
// The scalar projection of a onto b is
//          _
//    a = ||a||cos(theta)
//
// and the dot product is
//    _   _     _     _
//    a . b = ||a|| ||b|| cos(theta)
//
// so the vector projection is
//         _ _          _
//    proj_b(a) = a *   b  
//                    --_--
//                    ||b||
//                                                               _
//                (the right most term is the unit vector in the b direction)
//                _   _   _
//              = a . b * b
//                -----
//                  _  2
//                ||b||
//
// But we also have
//     _  2    _   _
//   ||b||   = b . b
//
// so             _   _
//         _ _    a . b   _
//    proj_b(a) = ----- * b
//                _   _
//                b . b
//
// This is the projection coefficient of projecting 'other' onto *this.
double Point::ProjectionCoefficient(const Point &other) const {
  double a_dot_b = this->DotProduct(other);
  double b_dot_b = this->DotProduct(*this);
  return a_dot_b / b_dot_b;
}

// Project the vector other onto *this.
Point Point::Project(const Point &other) const {
  double projection_coefficient = ProjectionCoefficient(other);
  return {
    std::llround(projection_coefficient * static_cast<double>(x_)),
    std::llround(projection_coefficient * static_cast<double>(y_))
  };
}

int64_t Point::DotProduct(const Point &other) const {
  return x_ * other.x() + y_ * other.y();
}

int64_t Point::L2SquaredDistanceTo(const Point &other) const {
  int64_t dx = std::abs(x_ - other.x_);
  int64_t dy = std::abs(y_ - other.y_);
  // dx^2 + dy^2. We can do the shift trick because we forced dx, dy to be
  // positive.
  return (dx << 1) + (dy << 1);
}

double Point::L2DistanceTo(const Point &other) const {
  double dx = x_ - other.x_;
  double dy = y_ - other.y_;
  return std::sqrt(std::pow(dx, 2.0) + std::pow(dy, 2.0));
}

std::string Point::Describe() const {
  std::stringstream ss;
  ss << "(" << x_ << ", " << y_ << ")";
  return ss.str();
}

Point operator*(const Point &lhs, double &rhs) {
  Point copy(lhs);
  copy.Scale(rhs);
  return copy;
}
Point operator*(double &lhs, const Point &rhs) {
  // Call above.
  return rhs * lhs;
}

Point operator+(const Point &lhs, const Point &rhs) {
  return Point(lhs.x() + rhs.x(), lhs.y() + rhs.y());
}

Point operator-(const Point &lhs, const Point &rhs) {
  return Point(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

Point operator-(const Point &other) {
  return Point(-other.x(), -other.y());
}

bool operator<(const Point &lhs, const Point &rhs) {
  if (lhs.x() != rhs.x()) {
    return lhs.x() < rhs.x();
  }
  return lhs.y() < rhs.y();
}

bool operator==(const Point &lhs, const Point &rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

bool operator!=(const Point &lhs, const Point &rhs) {
  return !(lhs == rhs);
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Point &point) {
  os << point.Describe();
  return os;
}

}  // namespace bfg
