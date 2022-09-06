#include <ostream>

#include "point.h"

namespace bfg {
namespace geometry {

Point &Point::operator+=(const Point &other){
  x_ = x_ + other.x_;
  y_ = y_ + other.y_;
  return *this;
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Point &point) {
  os << "(" << point.x() << ", " << point.y() << ")";
  return os;
}

geometry::Point operator+(
    const geometry::Point &lhs, const geometry::Point &rhs) {
  return geometry::Point(lhs.x() + rhs.x(), lhs.y() + rhs.y());
}

geometry::Point operator-(
    const geometry::Point &lhs, const geometry::Point &rhs) {
  return geometry::Point(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

geometry::Point operator-(const geometry::Point &other) {
  return geometry::Point(-other.x(), -other.y());
}

bool operator==(
    const geometry::Point &lhs, const geometry::Point &rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

}  // namespace bfg
