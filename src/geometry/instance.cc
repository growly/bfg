#include "instance.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>

#include "../cell.h"
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

void Instance::AlignPoints(const Point &our_point, const Point &align_to) {
  Translate(align_to - our_point);
}

uint64_t Instance::TilingHeight() const {
  return template_layout()->GetTilingBounds().Height();
}

uint64_t Instance::TilingWidth() const {
  return template_layout()->GetTilingBounds().Width();
}

Point Instance::TilingLowerLeft() const {
  Rectangle tiling_bounds = GetTilingBounds();
  return tiling_bounds.lower_left();
}

void Instance::RotatePreservingLowerLeft(int32_t rotation_degrees_ccw) {
  //geometry::Rectangle bounding_box = template_layout_->GetBoundingBox();
  //bounding_box.ResetOrigin();
  //bounding_box.Translate(lower_left_);
}

// +-----------------------+
// |                       |
// |                       |
// |      O                |
// |     /                 |
// |    /                  |
// |   /                   |              O', 'lower_left_'
// |  / B                  |             /
// | /                     |            /
// |L                      |           /
// +-----------------------+          / B, 'relative_to_origin'
// A, 'tiling_bounds.lower_left()'   /
//                                  L
//                                 X, 'new_lower_left'
//
// Find the new origin O' so that the lower left point of the tiling bounds
// appears at X
void Instance::MoveTilingLowerLeft(const Point &new_lower_left) {
  Rectangle tiling_bounds = GetTilingBounds();
  Point relative_to_origin = tiling_bounds.lower_left() - lower_left_;
  lower_left_ = new_lower_left - relative_to_origin;
  ports_generated_ = false;
}

const Rectangle Instance::GetTilingBounds() const {
  Rectangle transformed = template_layout_->GetTilingBounds();
  if (reflect_vertical_) {
    transformed.FlipVertical();
  }
  transformed.Rotate(rotation_degrees_ccw_);
  transformed.MoveLowerLeftTo(lower_left_ + transformed.lower_left());
  return transformed;
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
  int32_t rotation_ccw_degrees = (360 - (rotation_degrees_ccw_ % 360)) % 360;
  instance_ports_.clear();
  for (const auto &port : template_layout_->Ports()) {
    const std::string &net = port->net();
    Port *instance_port = new Port(*port);
    // In the template layout the implicit origin is always (0, 0).
    Rectangle rotated_bounds =
        instance_port->BoundingBoxIfRotated(Point(0, 0), rotation_ccw_degrees);
    instance_port->set_lower_left(rotated_bounds.lower_left());
    instance_port->set_upper_right(rotated_bounds.upper_right());
    // Now move translate the instance port according to the translation of the
    // instance, relative to the template layout origin (0, 0).
    geometry::Point translation = lower_left_ - Point {0, 0};
    instance_port->Translate(translation);
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
  instance_shapes.PrefixNetNames(name_, ".");

  shapes->Add(instance_shapes);
}

Port *Instance::GetNearestPortNamed(
    const Port &to_port, const std::string &name) {
  std::vector<Port*> matching_ports;
  GetInstancePorts(name, &matching_ports);
  if (matching_ports.empty()) {
    return nullptr;
  }

  auto comp = [&](Port *lhs, Port *rhs) {
    int64_t lhs_distance = lhs->centre().L2SquaredDistanceTo(to_port.centre());
    int64_t rhs_distance = rhs->centre().L2SquaredDistanceTo(to_port.centre());
    return lhs_distance < rhs_distance;
  };
  std::sort(matching_ports.begin(), matching_ports.end(), comp);
  return *matching_ports.begin();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Instance &instance) {
  os << "[Instance " << instance.name() << " of "
     << instance.template_layout()->parent_cell()->name() << "]";
  return os;
}

}  // namespace bfg
