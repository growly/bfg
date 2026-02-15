#ifndef GEOMETRY_ROUNDED_RECTANGLE_H_
#define GEOMETRY_ROUNDED_RECTANGLE_H_

#include <ostream>
#include <optional>

#include "arc.h"
#include "rectangle.h"
#include "shape.h"
#include "point.h"
#include "line.h"

namespace bfg {

class PhysicalPropertiesDatabase;

namespace geometry {

// A rectangle with rounded corners of a given radius.
//
// This is a more sophisticated shape for hit-testing the keep-out region of a
// regular rectilinear rectangle.
class RoundedRectangle : public Rectangle {
 public:
  struct Regions {
    Rectangle centre;
    Rectangle left;
    Rectangle upper;
    Rectangle right;
    Rectangle lower;
    Arc lower_left_arc;
    Arc upper_left_arc;
    Arc upper_right_arc;
    Arc lower_right_arc;

    // Bounding boxes for the corner regions.
    Rectangle lower_left;
    Rectangle upper_left;
    Rectangle upper_right;
    Rectangle lower_right;
  };

  //static double ClosestDistanceBetween(
  //    const RoundedRectangle &lhs, const RoundedRectangle &rhs);

  RoundedRectangle(
      const Point &lower_left, const Point &upper_right, int64_t corner_radius)
      : corner_radius_(corner_radius),
        Rectangle(lower_left, upper_right) {}

  // These methods do NOT consider the layers of shapes involved. They assume
  // all shapes are on the same plane.
  //
  // We ONLY support rectilinearly-aligned RoundedRectangles, nothing at a
  // weird angle!
  bool Overlaps(const Rectangle &other) const;
  bool Overlaps(const RoundedRectangle &other) const;
  bool Intersects(const Point &point) const;
  bool Intersects(const Point &point, int64_t margin) const;

  std::tuple<Point, Point, Point, Point> GetInnerCoordinates() const;
  Regions GetRegions() const;

  uint64_t Width() const { return upper_right_.x() - lower_left_.x(); }
  uint64_t Height() const { return upper_right_.y() - lower_left_.y(); }

  //void MirrorY() override;
  //void MirrorX() override;
  //void Translate(const Point &offset) override;
  //void ResetOrigin() override;
  //void FlipHorizontal() override {}   // No-op for a rectangle.
  //void FlipVertical() override {}   // No-op for rectangle.
  //void MoveLowerLeftTo(const Point &point) override {
  //  ResetOrigin();
  //  Translate(point);
  //}
  //void Rotate(int32_t degrees_ccw) override;

  const std::string Describe() const;

  Point Centre() const {
    Point centre = Point
        ((lower_left_.x() + upper_right_.x()) / 2,
         (lower_left_.y() + upper_right_.y()) / 2);
    centre.set_layer(layer_);
    return centre;
  }

  const Rectangle GetBoundingBox() const override {
    return Rectangle(lower_left_, upper_right_);
  }

  Point &lower_left() { return lower_left_; }
  const Point &lower_left() const { return lower_left_; }
  void set_lower_left(const Point &lower_left) { lower_left_ = lower_left; }

  Point &upper_right() { return upper_right_; }
  const Point &upper_right() const { return upper_right_; }
  void set_upper_right(const Point &upper_right) { upper_right_ = upper_right; }

  int64_t corner_radius() const { return corner_radius_; }

  // TODO(aryap): What meaning do these have for a RoundedRectangle?
  const Point UpperLeft() const {
    return Point(lower_left_.x(), upper_right_.y());
  }
  const Point LowerRight() const {
    return Point(upper_right_.x(), lower_left_.y());
  }

 protected:
  int64_t corner_radius_;

 private:
  bool WithinRadius(const Point &centre, const Point &test_point) const;
};

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os,
    const geometry::RoundedRectangle &rounded_rectangle);

}  // namespace bfg

#endif  // GEOMETRY_ROUNDED_RECTANGLE_H_
