#ifndef GEOMETRY_RECTANGLE_H_
#define GEOMETRY_RECTANGLE_H_

#include <ostream>

#include "shape.h"
#include "point.h"

namespace bfg {
namespace geometry {

// TODO(aryap): Do we need to have separate classes for a "rectangle", the
// object on a layer with connected nets and such, and a generic notion of a
// "rectangular region", which we use to do math? I mean, maybe. But
// laziness...
class Rectangle : public Shape {
 public:
  Rectangle() : Shape(0, "") {}
  Rectangle(const Point &lower_left, uint64_t width, uint64_t height)
      : lower_left_(lower_left),
        upper_right_(lower_left + Point(width, height)),
        Shape(0, "") {}

  Rectangle(const Point &lower_left, const Point &upper_right)
      : lower_left_(lower_left),
        upper_right_(upper_right),
        Shape(0, "") {}

  Rectangle(const Point &lower_left, const Point &upper_right,
            const Layer &layer, const std::string &net)
      : lower_left_(lower_left),
        upper_right_(upper_right),
        Shape(layer, net) {}

  Rectangle(const std::pair<Point, Point> &ll_ur)
      : lower_left_(ll_ur.first),
        upper_right_(ll_ur.second),
        Shape(0, "") {}

  bool Overlaps(const Rectangle &other) const;
  const Rectangle OverlapWith(const Rectangle &other) const;

  uint64_t Width() const { return upper_right_.x() - lower_left_.x(); }
  uint64_t Height() const { return upper_right_.y() - lower_left_.y(); }

  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;

  // TODO(aryap): Hmmm. Not a double. Truncating. Hmmm.
  // TODO(aryap): Rename Centre().
  Point centre() const {
    return Point((lower_left_.x() + upper_right_.x()) / 2,
                 (lower_left_.y() + upper_right_.y()) / 2);
  }

  const Rectangle GetBoundingBox() const override {
    return std::make_pair(lower_left_, upper_right_);
  }

  const Point &lower_left() const { return lower_left_; }
  void set_lower_left(const Point &lower_left) { lower_left_ = lower_left; }

  const Point &upper_right() const { return upper_right_; }
  void set_upper_right(const Point &upper_right) { upper_right_ = upper_right; }

  const Point UpperLeft() const {
    return Point(lower_left_.x(), upper_right_.y());
  }
  const Point LowerRight() const {
    return Point(upper_right_.x(), lower_left_.y());
  }

 protected:
  Point lower_left_;
  Point upper_right_;
};

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os,
    const geometry::Rectangle &rectangle);

}  // namespace bfg

#endif  // GEOMETRY_RECTANGLE_H_
