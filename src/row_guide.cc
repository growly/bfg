#include <optional>

#include "geometry/vector.h"
#include "geometry/instance.h"
#include "geometry/point.h"
#include "row_guide.h"
#include "design_database.h"

namespace bfg {

geometry::Instance *RowGuide::InstantiateBack(
    const std::string &name, Layout *template_layout) {
  geometry::Instance *installed = layout_->AddInstance(
      geometry::Instance(template_layout, {0, 0}));
  installed->set_name(name);

  MaybeAddTapRightFor(installed->TilingWidth());

  geometry::Point point = NextPointRight(*installed);
  Place(point, installed, &distance_to_tap_right_, &blank_space_right_);

  instances_.push_back(installed);
  return installed;
}

geometry::Instance *RowGuide::InstantiateAndInsertFront(
    const std::string &name, Layout *template_layout) {
  geometry::Instance *installed = layout_->AddInstance(
      geometry::Instance(template_layout, {0, 0}));
  installed->set_name(name);

  geometry::Point starting_lower_left = (instances_.empty() ?
      origin_ : instances_.front()->TilingLowerLeft()) -
      geometry::Vector(blank_space_left_, 0);

  MaybeAddTapLeftFor(installed->TilingWidth());

  geometry::Point point = NextPointLeft(*installed);
  Place(point, installed, &distance_to_tap_left_, nullptr);

  instances_.insert(instances_.begin(), installed);

  geometry::Point ending_lower_left =
      instances_.front()->TilingLowerLeft() -
      geometry::Vector(blank_space_left_, 0);


  ShiftAllRight(starting_lower_left.x() - ending_lower_left.x());

  return installed;
}

geometry::Instance *RowGuide::InstantiateFront(
    const std::string &name, Layout *template_layout) {
  geometry::Instance *installed = layout_->AddInstance(
      geometry::Instance(template_layout, origin_));
  installed->set_name(name);

  MaybeAddTapLeftFor(installed->TilingWidth());

  geometry::Point point = NextPointLeft(*installed);
  Place(point, installed, &distance_to_tap_left_, &blank_space_left_);

  instances_.insert(instances_.begin(), installed);

  return installed;
}

void RowGuide::AddBlankSpaceBack(uint64_t span) {
  MaybeAddTapRightFor(span);
  AccountForPlacement(span, &distance_to_tap_right_);
  blank_space_right_ += span;
}

void RowGuide::AddBlankSpaceAndInsertFront(uint64_t span) {
  geometry::Point starting_lower_left =
      (instances_.empty() ?  origin_ : instances_.front()->TilingLowerLeft()) -
      geometry::Vector(blank_space_left_, 0);

  MaybeAddTapLeftFor(span);
  AccountForPlacement(span, &distance_to_tap_left_);

  geometry::Point ending_lower_left =
      (instances_.empty() ? origin_ : instances_.front()->TilingLowerLeft()) -
      geometry::Vector(blank_space_left_ + span, 0);

  if (!instances_.empty()) {
    ShiftAllRight(starting_lower_left.x() - ending_lower_left.x());
  } else {
    blank_space_right_ += span;
  }
}

void RowGuide::AddBlankSpaceFront(uint64_t span) {
  MaybeAddTapLeftFor(span);
  AccountForPlacement(span, &distance_to_tap_left_);
  blank_space_left_ += span;
}

// TODO(aryap): Add taps to circuit.
geometry::Instance *RowGuide::AddTap() {
  return layout_->AddInstance(
      geometry::Instance(tap_cell_.value().get().layout()));
}

void RowGuide::MaybeAddTapLeftFor(uint64_t additional_span) {
  if (!NeedsTapLeft(additional_span)) {
    return;
  }
  geometry::Instance *tap = AddTap();
  geometry::Point point = NextPointLeft(*tap);
  Place(point, tap, nullptr, nullptr);
  num_taps_++;
  distance_to_tap_left_ = 0;
  instances_.insert(instances_.begin(), tap);
}

void RowGuide::MaybeAddTapRightFor(uint64_t additional_span) {
  if (!NeedsTapRight(additional_span)) {
    return;
  }
  geometry::Instance *tap = AddTap();
  geometry::Point point = NextPointRight(*tap);
  Place(point, tap, nullptr, nullptr);
  num_taps_++;
  distance_to_tap_right_ = 0;
  instances_.push_back(tap);
}

// FIXME: ok the thing to do is to figure out the role of "RowGuide". is it an
// adapter for layout? does it own meaningful stuff? If I call
// "InstantiateFront" am I expecting the origin_ to shift or for the new
// instance to be added and the remaining instances pushed back?
// Since it has a pointer to instances in the row and a pointer to a (shared)
// layout, this is ok?
// Maybe MemoryBank should similarly be an adapter, with a handle to shapes
// added through it, so that in effect it manages a grouping of instances. Do we
// need an InstanceGroup()?

geometry::Point RowGuide::NextPointLeft(const geometry::Instance &to_add)
    const {
  geometry::Vector blank = geometry::Vector(blank_space_left_, 0);
  if (instances_.empty()) {
    return origin_ - blank;
  }
  const geometry::Instance &existing = *instances_.front();

  geometry::Point existing_lower_left = existing.TilingLowerLeft();
  int64_t existing_tiling_width = existing.TilingWidth();
  int64_t existing_tiling_height = existing.TilingHeight();

  // Assuming vertically-centred alignment, compute the y-position of the new
  // instance given the old one:
  int64_t mid_y = existing_lower_left.y() + existing_tiling_height / 2;
  int64_t new_y = mid_y - to_add.TilingHeight() / 2;

  int64_t new_x = existing_lower_left.x() - to_add.TilingWidth() - blank.x();
  return {new_x, new_y};
}

geometry::Point RowGuide::NextPointRight(const geometry::Instance &to_add)
    const {
  geometry::Vector blank = geometry::Vector(blank_space_right_, 0);
  if (instances_.empty()) {
    return origin_ + blank;
  }
  const geometry::Instance &existing = *instances_.back();

  geometry::Point existing_lower_left = existing.TilingLowerLeft();
  int64_t existing_tiling_width = existing.TilingWidth();
  int64_t existing_tiling_height = existing.TilingHeight();

  // Assuming vertically-centred alignment, compute the y-position of the new
  // instance given the old one:
  int64_t mid_y = existing_lower_left.y() + existing_tiling_height / 2;
  int64_t new_y = mid_y - to_add.TilingHeight() / 2;

  int64_t new_x = existing_lower_left.x() + existing_tiling_width + blank.x();
  return {new_x, new_y};
}

void RowGuide::ShiftAllRight(int64_t x) {
  for (geometry::Instance *installed : instances_) {
    installed->Translate({x, 0});
  }
}

void RowGuide::MoveTo(const geometry::Point &new_origin) {
  geometry::Point old_origin = origin_;
  for (geometry::Instance *installed : instances_) {
    geometry::Point relative_position = installed->lower_left() - old_origin;
    installed->set_lower_left(relative_position + new_origin);
  }
  origin_ = new_origin;
}

void RowGuide::MoveLowerLeft(const geometry::Point &new_lower_left) {
  geometry::Point diff = origin_ - LowerLeft();
  geometry::Point new_origin = new_lower_left + diff;
  MoveTo(new_origin);
}

void RowGuide::MoveLowerRight(const geometry::Point &new_lower_right) {
  geometry::Point diff = origin_ - LowerRight();
  geometry::Point new_origin = new_lower_right + diff;
  MoveTo(new_origin);
}

// Sets the position of the given instance according to x_pos, y_pos and
// advances the x_pos value according to the instances' tiling bounds. If
// rotation is needed, that is done too.
void RowGuide::Place(
    const geometry::Point &point,
    geometry::Instance *instance,
    int64_t *distance_to_tap,
    int64_t *blank_counter) {
  //instance->MoveTilingLowerLeft(point);

  //int64_t tiling_width = instance->TilingWidth();
  //int64_t tiling_height = instance->TilingHeight();

  //if (rotate_instances_) {
  //  // Rotation is effective about the origin, which after 180 degrees means
  //  // we've set the upper right point on the instance. To make its effective
  //  // lower left point the one we intend _after rotation_ we have to shift
  //  // the whole instance up and right its own bounding box.
  //  instance->set_rotation_degrees_ccw(180);
  //  instance->Translate({tiling_width, tiling_height});
  //}
  if (rotate_instances_) {
    instance->set_rotation_degrees_ccw(180);
  }
  instance->MoveTilingLowerLeft(point);

  if (blank_counter) {
    *blank_counter = 0;
  }

  int64_t tiling_width = instance->TilingWidth();
  AccountForPlacement(tiling_width, distance_to_tap);
}

void RowGuide::AccountForPlacement(uint64_t span,
                                   int64_t *distance_to_tap) {
  if (distance_to_tap) {
    if (num_taps_ == 0) {
      distance_to_tap_right_ += span;
      distance_to_tap_left_ += span;
    } else {
      *distance_to_tap += span;
    }
  }
}

bool RowGuide::NeedsTapLeft(const geometry::Instance &added_instance) const {
  return NeedsTapLeft(added_instance.TilingWidth());
}

bool RowGuide::NeedsTapRight(const geometry::Instance &added_instance) const {
  return NeedsTapRight(added_instance.TilingWidth());
}

bool RowGuide::NeedsTapLeft(uint64_t additional_span) const {
  if (!tap_cell_) return false;
  return NeedsTap(distance_to_tap_left_, additional_span);
}

bool RowGuide::NeedsTapRight(uint64_t additional_span) const {
  if (!tap_cell_) return false;
  return NeedsTap(distance_to_tap_right_, additional_span);
}

bool RowGuide::NeedsTap(const int64_t &current_distance,
                        int64_t additional_span) const {
  if (start_with_tap_ && num_taps_ == 0) {
    return true;
  }
  if (!tap_cell_) {
    return false;
  }
  return (current_distance + additional_span) > max_tap_distance_;
}

uint64_t RowGuide::Width() const {
  uint64_t width = blank_space_left_;
  for (geometry::Instance *const instance : instances_) {
    width += instance->TilingWidth();
  }
  width += blank_space_right_;
  return width;
}

uint64_t RowGuide::Height() const {
  uint64_t height = 0;
  for (geometry::Instance *const instance : instances_) {
    height = std::max(instance->TilingHeight(), height);
  }
  return height;
}

std::optional<geometry::Rectangle> RowGuide::GetBoundingBox() const {
  if (instances_.empty()) {
    return std::nullopt;
  }
  return {{
      instances_.front()->GetBoundingBox().lower_left(),
      instances_.back()->GetBoundingBox().upper_right()}};
}

std::optional<geometry::Rectangle> RowGuide::GetTilingBounds() const {
  if (instances_.empty()) {
    return std::nullopt;
  }
  geometry::Rectangle bounding_box = instances_.front()->GetTilingBounds();
  for (auto it = instances_.begin() + 1; it != instances_.end(); ++it) {
    bounding_box.ExpandToCover((*it)->GetTilingBounds());
  }
  return bounding_box;
}

geometry::Point RowGuide::UpperRight() const {
  geometry::Vector blank = geometry::Vector(blank_space_right_, 0);
  if (instances_.empty()) {
    return origin_ + blank;
  }
  geometry::Rectangle front_tiling_bounds =
      instances_.back()->GetTilingBounds();
  return geometry::Point(
      std::max(front_tiling_bounds.upper_right().x(), origin_.x()) + blank.x(),
      front_tiling_bounds.upper_right().y());
}

geometry::Point RowGuide::LowerRight() const {
  geometry::Vector blank = geometry::Vector(blank_space_right_, 0);
  if (instances_.empty()) {
    return origin_ + blank;
  }
  geometry::Rectangle front_tiling_bounds =
      instances_.back()->GetTilingBounds();
  return geometry::Point(
      std::max(front_tiling_bounds.LowerRight().x(), origin_.x()) + blank.x(),
      front_tiling_bounds.LowerRight().y());
}

geometry::Point RowGuide::UpperLeft() const {
  geometry::Vector blank = geometry::Vector(blank_space_left_, 0);
  if (instances_.empty()) {
    return origin_ - blank;
  }
  geometry::Rectangle front_tiling_bounds =
      instances_.front()->GetTilingBounds();
  return geometry::Point(
      std::min(front_tiling_bounds.UpperLeft().x(), origin_.x()) - blank.x(),
      front_tiling_bounds.UpperLeft().y());
}

geometry::Point RowGuide::LowerLeft() const {
  geometry::Vector blank = geometry::Vector(blank_space_left_, 0);
  if (instances_.empty()) {
    return origin_ - blank;
  }
  geometry::Rectangle front_tiling_bounds =
      instances_.front()->GetTilingBounds();
  return geometry::Point(
      std::min(front_tiling_bounds.lower_left().x(), origin_.x()) - blank.x(),
      front_tiling_bounds.lower_left().y());
}

}   // namespace bfg
