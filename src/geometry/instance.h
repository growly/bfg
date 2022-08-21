#ifndef GEOMETRY_INSTANCE_H_
#define GEOMETRY_INSTANCE_H_

#include <string>

#include "point.h"
#include "rectangle.h"

namespace bfg {

class Layout;

namespace geometry {

class Instance {
 public:
  Instance(bfg::Layout *template_layout,
           const Point &lower_left)
      : template_layout_(template_layout),
        lower_left_(lower_left),
        reflect_vertical_(false),
        rotation_clockwise_degrees_(0) {}

  const Rectangle GetBoundingBox() const;

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  bfg::Layout *template_layout() const { return template_layout_; }

  const Point &lower_left() const { return lower_left_; }

  void set_reflect_vertical(bool reflect) {
    reflect_vertical_ = reflect;
  }
  const bool reflect_vertical() const { return reflect_vertical_; }

  void set_rotation_clockwise_degrees(int32_t degrees) {
    rotation_clockwise_degrees_ = degrees;
  }
  const int32_t rotation_clockwise_degrees() const { return
    rotation_clockwise_degrees_;
  }

 private:
  std::string name_;

  // This is the template cell.
  bfg::Layout *template_layout_;

  Point lower_left_;
  bool reflect_vertical_;
  int32_t rotation_clockwise_degrees_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_INSTANCE_H_
