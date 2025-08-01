#ifndef GEOMETRY_POINT_H_
#define GEOMETRY_POINT_H_

#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <absl/strings/str_format.h>

#include "../physical_properties_database.h"
#include "abstract_shape.h"
#include "manipulable.h"
#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace geometry {

class Point : public AbstractShape, public Manipulable {
 public:
  static bool CompareX(const Point &lhs, const Point &rhs);
  static bool CompareY(const Point &lhs, const Point &rhs);
  static bool CompareXThenY(const Point &lhs, const Point &rhs);
  static bool CompareYThenX(const Point &lhs, const Point &rhs);
  static bool ShareHorizontalOrVerticalAxis(
      const Point &lhs, const Point &rhs);

  static std::vector<Point*> SortAscendingX(
      const std::vector<Point*> &input);
  static std::vector<Point*> SortDescendingX(
      const std::vector<Point*> &input);

  static Point MidpointOf(const Point &lhs, const Point &rhs);
  static Point PickMaxY(const Point &lhs, const Point &rhs);
  static Point PickMinY(const Point &lhs, const Point &rhs);

  static Point ClosestTo(const std::vector<Point> &points,
                         const Point &target);

  // NOTE(aryap): It doesn't make sense to use this with angles that aren't
  // multiples of pi/4, since our Point has integer units:
  //
  //
  //          +     + (1, 1)
  //          |
  //          |       (1, 0)
  //    +-----+-----+
  //          |(0, 0)
  //          |
  //          +
  //
  // The only unit-length
  // lines we can represent in this format are those with angles at multiples of
  // pi/2 to the horizon.
  static Point UnitVector(double angle_to_horizon_radians);

  Point()
      : x_(0), y_(0) {}
  Point(const int64_t x, const int64_t y)
      : x_(x), y_(y) {}

  const int64_t &x() const { return x_; }
  const int64_t &y() const { return y_; }

  void set_x(const int64_t &x) { x_ = x; }
  void set_y(const int64_t &y) { y_ = y; }

  ::vlsir::raw::Point ToVLSIRPoint(const PhysicalPropertiesDatabase &db) const {
    ::vlsir::raw::Point point_pb;
    point_pb.set_x(db.ToExternalUnits(x_));
    point_pb.set_y(db.ToExternalUnits(y_));
    return point_pb;
  }

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override {}   // No-op for a point.
  void FlipVertical() override {}   // No-op for a point.
  void Translate(const Point &offset) override;
  void ResetOrigin() override {}    // No-op for a point.

  void Scale(double scalar);

  void Rotate(double theta_radians);
  void Rotate(int32_t degrees_ccw) override;

  // This is the same as creating a unit vector with the given angle and adding
  // it to this Point (treated as a Vector).
  void AddComponents(double amount, double angle_rads);
  int64_t Component(double angle_rads) const;

  // This isn't hard to implement with other interface methods, but it sure is
  // convenient. (Using std::swap hides the irksome inelegance of having a
  // temporary and doesn't require making the x_ and y_ fields visible to
  // other classes.)
  void SwapX(Point *other) {
    std::swap(x_, other->x_);
  }
  void SwapY(Point *other) {
    std::swap(y_, other->y_);
  }

  // The Length of a point is the length of the Vector from (0, 0) to the Point.
  double Length() const { return L2DistanceTo(Point(0, 0)); }

  double ProjectionCoefficient(const Point &other) const;
  // Treating this point as a vector from (0, 0) to (x_, y_), and likewise
  // treating the other point as a vector from (0, 0) to its (x_, y_), return
  // the vector projection of the other onto this. The return value is likewise
  // a point representing a vector from (0, 0).
  Point Project(const Point &other) const;

  bool IsStrictlyLeftOf(const Point &other) const {
    return x_ < other.x_;
  }
  bool IsLeftOf(const Point &other) const {
    return x_ <= other.x_;
  }
  bool IsStrictlyRightOf(const Point &other) const {
    return x_ > other.x_;
  }
  bool IsRightOf(const Point &other) const {
    return x_ >= other.x_;
  }

  // Treating this point as a vector from (0, 0) to (x_, y_), and likewise
  // treating the other point as a vector from (0, 0) to (x_, y_), return the
  // dot product of the vectors.
  int64_t DotProduct(const Point &other) const;

  std::string Describe() const;

  int64_t L2SquaredDistanceTo(const Point &other) const;
  double L2DistanceTo(const Point &other) const;
  int64_t L1DistanceTo(const Point &point) const;

  Point &operator+=(const Point &other);
  Point &operator-=(const Point &other);

 private:
  // https://google.github.io/googletest/advanced.html#teaching-googletest-how-to-print-your-values
  friend void PrintTo(const Point &point, std::ostream *os) {
    *os << point.Describe();
  }

  int64_t x_;
  int64_t y_;
};

// Treat the point as a vector from the origin, then add element-wise.
Point operator*(const Point &lhs, double &rhs);
Point operator*(double &lhs, const Point &rhs);
Point operator/(const Point &lhs, int divisor);
Point operator+(const Point &lhs, const Point &rhs);
Point operator-(const Point &lhs, const Point &rhs);
Point operator-(const Point &rhs);

bool operator<(const Point &lhs, const Point &rhs);
bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);

typedef std::pair<Point, Point> PointPair;

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Point &point);

}  // namespace bfg

#endif  // GEOMETRY_POINT_H_
