#include "instance.h"

#include <cmath>
#include <glog/logging.h>

#include "../layout.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {

namespace geometry {

void Instance::MirrorY() {
  rotation_degrees_ccw_ = (rotation_degrees_ccw_ + 180) % 360;
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

uint64_t Instance::TilingHeight() const {
  return template_layout()->GetTilingBounds().Height();
}

uint64_t Instance::TilingWidth() const {
  return template_layout()->GetTilingBounds().Width();
}

// We compute the bounding box assuming the instance has been rotated by
// rotation_degrees_ccw_ about the origin in the frame of the template
// cell; that is, about the lower left point of the instance.
const Rectangle Instance::GetBoundingBox() const {
  LOG_IF(FATAL, template_layout_ == nullptr)
      << "Why does this Instance object have no template_layout set?";
  Rectangle template_bb = template_layout_->GetBoundingBox();

  // We used to store it cw:
  //int32_t degrees_ccw = (
  //    360 - (rotation_degrees_ccw_ % 360)) % 360;

  Rectangle rotated = template_bb.BoundingBoxIfRotated(
      Point(0, 0), rotation_degrees_ccw_);
  rotated.Translate(lower_left_);
  return rotated;
}

void Instance::GeneratePorts() {
  int32_t rotation_ccw_degrees = (
      360 - (rotation_degrees_ccw_ % 360)) % 360;
  instance_ports_.clear();
  for (const auto &port : template_layout_->Ports()) {
    const std::string &net = port->net();
    Port *instance_port = new Port(*port);
    Rectangle rotated_bounds =
        instance_port->BoundingBoxIfRotated(Point(0, 0), rotation_ccw_degrees);
    instance_port->set_lower_left(rotated_bounds.lower_left());
    instance_port->set_upper_right(rotated_bounds.upper_right());
    // Move to where the instance is supposed to sit:
    instance_port->MoveLowerLeftTo(lower_left_);
    instance_ports_[net].insert(std::unique_ptr<Port>(instance_port));
  }
  ports_generated_ = true;
}

void Instance::GetShapesOnLayer(const geometry::Layer &layer,
                                ShapeCollection *shapes) const {
  ShapeCollection *master_shapes = template_layout_->GetShapeCollection(layer);
  if (!master_shapes)
    return;

  ShapeCollection instance_shapes;
  instance_shapes.Add(*master_shapes);
  instance_shapes.Rotate(rotation_degrees_ccw_);
  instance_shapes.Translate(lower_left_);
  shapes->Add(instance_shapes);
}

}  // namespace geometry
}  // namespace bfg
