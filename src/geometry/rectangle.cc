#include "rectangle.h"

#include <algorithm>
#include <glog/logging.h>

namespace bfg {
namespace geometry {

bool Rectangle::Overlaps(const Rectangle &other) const {
  if (other.upper_right().x() < lower_left_.x() ||
      other.upper_right().y() < lower_left_.y() ||
      upper_right_.x() < other.lower_left().x() ||
      upper_right_.y() < other.lower_left().y()) {
    return false;
  }
  return true;
}

const Rectangle Rectangle::OverlapWith(const Rectangle &other) const {
  if (!Overlaps(other))
    return Rectangle(Point(0, 0), Point(0, 0));
  int64_t min_x = std::max(lower_left_.x(), other.lower_left().x());
  int64_t min_y = std::max(lower_left_.y(), other.lower_left().y());
  int64_t max_x = std::min(upper_right_.x(), other.upper_right().x());
  int64_t max_y = std::min(upper_right_.y(), other.upper_right().y());
  return Rectangle(Point(min_x, min_y), Point(max_x, max_y));
}

void Rectangle::MirrorY() {
  Point new_upper_right(-lower_left_.x(), upper_right_.y());
  Point new_lower_left(-upper_right_.x(), lower_left_.y());
  lower_left_ = new_lower_left;
  upper_right_ = new_upper_right;
}

void Rectangle::MirrorX() {
  Point new_upper_right(upper_right_.x(), -lower_left_.y());
  Point new_lower_left(lower_left_.x(), -upper_right_.y());
  lower_left_ = new_lower_left;
  upper_right_ = new_upper_right;
}

void Rectangle::Translate(const Point &offset) {
  lower_left_ = lower_left_ + offset;
  upper_right_ = upper_right_ + offset;
}

void Rectangle::ResetOrigin() {
  Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os,
    const geometry::Rectangle &rectangle) {
  os << "[Rectangle " << rectangle.lower_left()
     << " " << rectangle.upper_right() << "]";
  return os;
}

}  // namespace bfg
