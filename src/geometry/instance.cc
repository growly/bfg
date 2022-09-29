#include "instance.h"

#include <cmath>
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

// We compute the bounding box assuming the instance has been rotated by
// rotation_clockwise_degrees_ about the origin in the frame of the template
// cell; that is, about the lower left point of the instance.
const Rectangle Instance::GetBoundingBox() const {
  LOG_IF(FATAL, template_layout_ == nullptr)
      << "Why does this Instance object have no template_layout set?";
  Rectangle template_bb = template_layout_->GetBoundingBox();
  Rectangle unrotated = Rectangle(
      template_bb.lower_left() + lower_left_,
      template_bb.upper_right() + lower_left_);
  
  Point rotation_about = lower_left_;

  // LOG(INFO) << "unrotated bounding box: " << unrotated;

  Point lower_left = unrotated.lower_left() - rotation_about;
  Point upper_left = unrotated.UpperLeft() - rotation_about;
  Point upper_right = unrotated.upper_right() - rotation_about;
  Point lower_right = unrotated.LowerRight() - rotation_about;

  int32_t rotation_ccw_degrees = (
      360 - (rotation_clockwise_degrees_ % 360)) % 360;

  lower_left.Rotate(rotation_ccw_degrees);
  upper_left.Rotate(rotation_ccw_degrees);
  upper_right.Rotate(rotation_ccw_degrees);
  lower_right.Rotate(rotation_ccw_degrees);

  std::vector x_points = {
      lower_left.x(), upper_left.x(), upper_right.x(), lower_right.x() };
  std::vector y_points = {
      lower_left.y(), upper_left.y(), upper_right.y(), lower_right.y() };

  int64_t min_x = *std::min_element(x_points.begin(), x_points.end());
  int64_t max_x = *std::max_element(x_points.begin(), x_points.end());
  int64_t min_y = *std::min_element(y_points.begin(), y_points.end());
  int64_t max_y = *std::max_element(y_points.begin(), y_points.end());

  Rectangle rotated = Rectangle(Point(min_x, min_y) + rotation_about,
                                Point(max_x, max_y) + rotation_about);
  // LOG(INFO) << "rotated bounding box: " << rotated;
  return rotated;
}

}  // namespace geometry
}  // namespace bfg
