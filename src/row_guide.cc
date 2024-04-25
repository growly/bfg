#include <optional>

#include "geometry/instance.h"
#include "geometry/point.h"
#include "row_guide.h"
#include "design_database.h"

namespace bfg {

geometry::Instance *RowGuide::InstantiateBack(
    const std::string &name, Layout *template_layout) {
  // Instances start out at whatever lower_left_ is, before being moved in
  // Place().
  geometry::Instance instance = geometry::Instance(
      template_layout, lower_left_);
  instance.set_name(name);
  geometry::Instance *installed = layout_->AddInstance(instance);

  int64_t width = installed->TilingWidth();
  if (tap_cell_.has_value() &&
      (right_most_tap_x_

  geometry::Point next = instances_.empty() ? lower_left_ : instances_.back()->

  instances_.push_back(installed);
  return installed;
}

geometry::Instance *RowGuide::InstantiateAndInsertFront(
    const std::string &name, Layout *template_layout) {
  geometry::Instance instance(template_layout, lower_left_);
  instance.set_name(name);
  geometry::Instance *installed = layout_->AddInstance(instance);

  instances_.insert(instances_.begin(), installed);

  return installed;
}

geometry::Instance *RowGuide::InstantiateFront(
    const std::string &name, Layout *template_layout) {
  geometry::Instance instance(template_layout, lower_left_);
  instance.set_name(name);
  geometry::Instance *installed = layout_->AddInstance(instance);

  instances_.insert(instances_.begin(), installed);

  lower_left_.set_x(lower_left_.x() - instance.TilingWidth());

  return installed;
}

// TODO(aryap): Add taps to circuit.
void RowGuide::PlaceTap(const geometry::Point &point) {
  if (!tap_cell_) {
    return;
  }
  geometry::Instance tap_instance(tap_cell_.value().get().layout());
  Place(&tap_instance, &x_pos, &y_pos, &distance_to_tap);
  geometry::Instance *installed = layout_->AddInstance(tap_instance);

  distance_to_tap = 0;
}

// FIXME: ok the thing to do is to figure out the role of "RowGuide". is it an
// adapter for layout? does it own meaningful stuff? If I call
// "InstantiateFront" am I expecting the lower_left_ to shift or for the new
// instance to be added and the remaining instances pushed back?
// Since it has a pointer to instances in the row and a pointer to a (shared)
// layout, this is ok?
// Maybe MemoryBank should similarly be an adapter, with a handle to shapes
// added through it, so that in effect it manages a grouping of instances. Do we
// need an InstanceGroup()?

// Sets the position of the given instance according to x_pos, y_pos and
// advances the x_pos value according to the instances' tiling bounds. If
// rotation is needed, that is done too.
void RowGuide::Place(
    geometry::Instance *instance,
    int64_t *x_pos,
    int64_t *y_pos,
    int64_t *distance_to_tap) const {
  instance->set_lower_left({*x_pos, *y_pos});
  geometry::Rectangle tiling_bounds =
      instance->template_layout()->GetTilingBounds();
  if (rotate_instances_) {
    // Rotation is effective about the origin, which after 180 degrees means
    // we've set the upper right point on the instance. To make its effective
    // lower left point the one we intend _after rotation_ we have to shift
    // the whole instance up and right its own bounding box.
    instance->set_rotation_degrees_ccw(180);
    instance->Translate({
        static_cast<int64_t>(tiling_bounds.Width()),
        static_cast<int64_t>(tiling_bounds.Height())
    });
  }
  (*x_pos) += tiling_bounds.Width();
  (*distance_to_tap) += tiling_bounds.Width();
}

void RowGuide::Place() {
  // Walk through instances, assigning positions in order. If rotation is
  // enabled, instances must be rotated and moved too. Tap cells, once
  // generated, are owned by the given layout.
  int64_t x_pos = lower_left_.x();
  int64_t y_pos = lower_left_.y();

  int64_t distance_to_tap = 0;

  for (auto it = instances_.begin(); it != instances_.end(); ++it) {
    geometry::Instance *instance = *it;
    int64_t next_width = instance->TilingWidth();

    // TODO(aryap): Generalise this.
    // TODO(aryap): Is this the right model? Maybe a RowGuide is more of a
    // sub-tile? RowGenerator? Now it needs to seem more inline...
    if (tap_cell_.has_value() &&
        (distance_to_tap + next_width) >= 10000) {
      geometry::Instance tap_instance(tap_cell_.value().get().layout());
      Place(&tap_instance, &x_pos, &y_pos, &distance_to_tap);
      // FIXME(aryap): Add taps to circuit.
      geometry::Instance *installed = layout_->AddInstance(tap_instance);
      it = instances_.insert(it, installed);
      // 'it' now points to the inserted tap, which was inserted just before the
      // current instance (previous pointee of 'it'). We advance it again to
      // have it point back at where 'instance' now is.
      ++it;

      distance_to_tap = 0;
    }

    // Effectively "MoveTo".
    Place(instance, &x_pos, &y_pos, &distance_to_tap);

    // Instance should already be placed.
    // layout()->AddInstance(...);
  }
}

uint64_t RowGuide::Width() const {
  uint64_t width = 0;
  for (geometry::Instance *const instance : instances_) {
    width += instance->TilingWidth();
  }
  return width;
}

uint64_t RowGuide::Height() const {
  uint64_t height = 0;
  for (geometry::Instance *const instance : instances_) {
    height = std::max(instance->TilingHeight(), height);
  }
  return height;
}

geometry::Point RowGuide::UpperRight() const {
  if (instances_.empty()) {
    return lower_left_;
  }
  int64_t x = instances_.back()->lower_left().x() +
      instances_.back()->TilingWidth();
  int64_t y = lower_left_.y() + Height();
  return {x, y};
}

geometry::Point RowGuide::LowerRight() const {
  if (instances_.empty()) {
    return lower_left_;
  }
  int64_t x = instances_.back()->lower_left().x() +
      instances_.back()->TilingWidth();
  int64_t y = lower_left_.y();
  return {x, y};
}

geometry::Point RowGuide::UpperLeft() const {
  int64_t x = lower_left_.x();
  int64_t y = lower_left_.y() + Height();
  return {x, y};
}

geometry::Point RowGuide::LowerLeft() const {
  return lower_left_;
}

}   // namespace bfg
