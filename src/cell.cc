#include "cell.h"

#include "geometry/point.h"

namespace bfg {

using geometry::Point;

const std::pair<Point, Point> Cell::GetBoundingBox() const {
  Point start;
  if (!polygons_.empty()) {
    start = polygons_.front().GetBoundingBox().first;
  } else if (!instances_.empty()) {
    start = instances_.front().GetBoundingBox().first;
  } else {
    // Cell is empty.
    return std::make_pair(Point(0, 0), Point(0, 0));
  }

  int64_t min_x = start.x();
  int64_t max_x = start.x();
  int64_t min_y = start.y();
  int64_t max_y = start.y();

  for (const auto &polygon : polygons_) {
    std::pair<Point, Point> bounding_box = polygon.GetBoundingBox();
    const Point &lower_left = bounding_box.first;
    const Point &upper_right = bounding_box.second;
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  for (const auto &instance : instances_) {
    std::pair<Point, Point> bounding_box = instance.GetBoundingBox();
    const Point &lower_left = bounding_box.first;
    const Point &upper_right = bounding_box.second;
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  return std::make_pair(Point(min_x, min_y), Point(max_x, max_y));
}


}  // namespace bfg
