#include <ostream>

#include "point.h"

namespace bfg {
namespace geometry {

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

bool operator==(
    const geometry::Point &lhs, const geometry::Point &rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

}  // namespace bfg
