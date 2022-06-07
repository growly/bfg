#ifndef POINT_H_
#define POINT_H_

#include <ostream>
#include <cstdint>
#include <vector>

namespace bfg {
namespace geometry {

class Point {
 public:
  Point() = default;
  Point(const int64_t x, const int64_t y)
      : x_(x),
        y_(y) {}

  const int64_t &x() const { return x_; }
  const int64_t &y() const { return y_; }

  void set_x(const int64_t &x) { x_ = x; }
  void set_y(const int64_t &y) { y_ = y; }

 private:
  int64_t x_;
  int64_t y_;
};

std::ostream &operator<<(std::ostream &os, const Point &point);

// Treat the point as a vector from the origin, then add element-wise.
Point operator+(const Point &lhs, const Point &rhs);
Point operator-(const Point &lhs, const Point &rhs);

bool operator==(const Point &lhs, const Point &rhs);

}  // namespace geometry
}  // namespace bfg

#endif  // POINT_H_
