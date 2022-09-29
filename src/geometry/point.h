#ifndef GEOMETRY_POINT_H_
#define GEOMETRY_POINT_H_

#include <ostream>
#include <cstdint>
#include <vector>

#include "manipulable.h"
#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace geometry {

class Point : public Manipulable {
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
  void Rotate(int32_t degrees_counter_clockwise);

  Point &operator+=(const Point &other);

 private:
  int64_t x_;
  int64_t y_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Point &point);

// Treat the point as a vector from the origin, then add element-wise.
geometry::Point operator+(
    const geometry::Point &lhs, const geometry::Point &rhs);
geometry::Point operator-(
    const geometry::Point &lhs, const geometry::Point &rhs);
geometry::Point operator-(const geometry::Point &rhs);

bool operator==(
    const geometry::Point &lhs, const geometry::Point &rhs);

}  // namespace bfg

#endif  // GEOMETRY_POINT_H_
