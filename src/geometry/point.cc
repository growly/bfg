#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "point.h"

namespace bfg {
namespace geometry {

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

int64_t Point::L2SquaredDistanceTo(const Point &other) const {
  int64_t dx = std::abs(x_ - other.x_);
  int64_t dy = std::abs(y_ - other.y_);
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
