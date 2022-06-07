#include <ostream>

#include "point.h"

namespace bfg {

std::ostream &operator<<(std::ostream &os, const Point &point) {
  os << "(" << point.x() << ", " << point.y() << ")";
  return os;
}

Point operator+(const Point &lhs, const Point &rhs) {
  return Point(lhs.x() + rhs.x(), lhs.y() + rhs.y());
}

Point operator-(const Point &lhs, const Point &rhs) {
  return Point(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

bool operator==(const Point &lhs, const Point &rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

}  // namespace bfg
