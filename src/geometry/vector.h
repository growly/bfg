#ifndef GEOMETRY_VECTOR_H_
#define GEOMETRY_VECTOR_H_

#include <ostream>
#include <cstdint>
#include <vector>

#include "point.h"
#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace geometry {

// TODO(aryap):
// A point is often treated like a vector, with the assumption that it is the
// 2-vector from the origin (0, 0) to the point given by (x_, y_). It might be
// more readable to have an explicit vector type for vector computations, so
// that it can also intuitively take part in linear algebra too.
class Vector : public Point {
 public:
  Point() = default;
  Point(const int64_t x, const int64_t y)
      : x_(x),
        y_(y) {}

  const int64_t &x() const { return x_; }
  const int64_t &y() const { return y_; }

  void set_x(const int64_t &x) { x_ = x; }
  void set_y(const int64_t &y) { y_ = y; }

  ::vlsir::raw::Point ToVLSIRPoint() const {
    ::vlsir::raw::Point point_pb;
    point_pb.set_x(x_);
    point_pb.set_y(y_);
    return point_pb;
  }

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override {}   // No-op for a point.
  void FlipVertical() override {}   // No-op for a point.
  void Translate(const Point &offset) override;
  void ResetOrigin() override {}    // No-op for a point.

  void Rotate(double theta_radians);
  void Rotate(int32_t degrees_ccw) override;

  int64_t L2SquaredDistanceTo(const Point &other) const;
  double L2DistanceTo(const Point &other) const;

  Point &operator+=(const Point &other);

 private:
  int64_t x_;
  int64_t y_;
};

// Treat the point as a vector from the origin, then add element-wise.
Point operator+(const Point &lhs, const Point &rhs);
Point operator-(const Point &lhs, const Point &rhs);
Point operator-(const Point &rhs);

bool operator==(const Point &lhs, const Point &rhs);

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Point &point);

}  // namespace bfg

#endif  // GEOMETRY_VECTOR_H_
