#include "layout.h"

#include <sstream>

#include "geometry/point.h"

namespace bfg {

using geometry::Point;

const std::pair<Point, Point> Layout::GetBoundingBox() const {
  Point start;
  if (!polygons_.empty()) {
    start = polygons_.front().GetBoundingBox().first;
  } else if (!instances_.empty()) {
    start = instances_.front().GetBoundingBox().first;
  } else {
    // Layout is empty.
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

std::string Layout::Describe() const {
  std::stringstream ss;

  ss << "layout: " << rectangles_.size() << " rectangles, "
     << polygons_.size() << " polygons, "
     << std::endl;
  for (const geometry::Rectangle &rectangle : rectangles_) {
    ss << "rect " << rectangle.lower_left().x() << " "
       << rectangle.lower_left().y() << " "
       << rectangle.upper_right().x() << " "
       << rectangle.upper_right().y() << std::endl;
  }

  for (const geometry::Polygon &poly : polygons_) {
    ss << "polygon ";
    for (const geometry::Point &point : poly.vertices()) {
      ss << "(" << point.x() << ", " << point.y() << ") ";
    }
    ss << std::endl;
  }

  return ss.str();
}

void Layout::SetActiveLayerByName(const std::string &name) {
  set_active_layer(physical_db_.GetLayerInfo(name).internal_layer);
}

}  // namespace bfg
