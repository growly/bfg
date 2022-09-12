#include "instance.h"

#include <glog/logging.h>

#include "../layout.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {

namespace geometry {

void Instance::MirrorY() {
  rotation_clockwise_degrees_ = (rotation_clockwise_degrees_ + 180) % 360;
  FlipVertical();
}

void Instance::MirrorX() {
  reflect_vertical_ = !reflect_vertical_;
}

void Instance::FlipHorizontal() {
  MirrorY();
  lower_left_ = -lower_left_ - Point(
      template_layout_->GetBoundingBox().Width(), 0);
}

void Instance::FlipVertical() {
  MirrorX();
  lower_left_ = -lower_left_ - Point(
      0, template_layout_->GetBoundingBox().Height());
}

void Instance::Translate(const Point &offset) {
  lower_left_ += offset;
}

void Instance::ResetOrigin() {
  lower_left_ = Point(0, 0);
}

const Rectangle Instance::GetBoundingBox() const {
  LOG_IF(FATAL, template_layout_ == nullptr)
      << "Why does this Instance object have no template_layout set?";
  Rectangle template_bb = template_layout_->GetBoundingBox();
  
  return Rectangle(template_bb.lower_left() + lower_left_,
                   template_bb.upper_right() + lower_left_);
}

}  // namespace geometry
}  // namespace bfg
