#include "shape_collection.h"

#include <sstream>

namespace bfg {
namespace geometry {

std::string ShapeCollection::Describe() const {
  std::stringstream ss;

  for (const auto &rectangle : rectangles_) {
    ss << "    rect " << rectangle->lower_left().x() << " "
       << rectangle->lower_left().y() << " "
       << rectangle->upper_right().x() << " "
       << rectangle->upper_right().y() << std::endl;
  }
  for (const auto &poly : polygons_) {
    ss << "    polygon ";
    for (const geometry::Point &point : poly->vertices()) {
      ss << "(" << point.x() << ", " << point.y() << ") ";
    }
    ss << std::endl;
  }
  for (const auto &port : ports_) {
    ss << "    port " << port->lower_left().x() << " "
       << port->lower_left().y() << " "
       << port->upper_right().x() << " "
       << port->upper_right().y() << std::endl;
  }
  return ss.str();
}

bool ShapeCollection::Empty() const {
  return rectangles_.empty() && polygons_.empty() && ports_.empty();
}

void ShapeCollection::Add(const ShapeCollection &other) {
  for (const auto &rectangle : other.rectangles_) {
    Rectangle *copy = new Rectangle(*rectangle);
    rectangles_.emplace_back(copy);
  }
  for (const auto &polygon : other.polygons_) {
    Polygon *copy = new Polygon(*polygon);
    polygons_.emplace_back(copy);
  }
  for (const auto &port : other.ports_) {
    Port *copy = new Port(*port);
    ports_.emplace_back(copy);
  }
}

void ShapeCollection::MirrorY() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorY(); }
  for (const auto &polygon : polygons_) { polygon->MirrorY(); }
  for (const auto &port : ports_) { port->MirrorY(); }
}

void ShapeCollection::MirrorX() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorX(); }
  for (const auto &polygon : polygons_) { polygon->MirrorX(); }
  for (const auto &port : ports_) { port->MirrorX(); }
}

void ShapeCollection::Translate(const Point &offset) {
  for (const auto &rectangle : rectangles_) {
    rectangle->Translate(offset);
  }
  for (const auto &polygon : polygons_) { polygon->Translate(offset); }
  for (const auto &port : ports_) { port->Translate(offset); }
}

void ShapeCollection::Rotate(int32_t degrees_ccw) {
  for (const auto &rectangle : rectangles_) { rectangle->Rotate(degrees_ccw); }
  for (const auto &polygon : polygons_) { polygon->Rotate(degrees_ccw); }
  for (const auto &port : ports_) { port->Rotate(degrees_ccw); }
}

void ShapeCollection::ResetOrigin() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

void ShapeCollection::FlipHorizontal() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorY();
  Translate(Point(bounding_box.Width() * 2, 0));
}

void ShapeCollection::FlipVertical() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorX();
  Translate(Point(0, bounding_box.Height() * 2));
}

const Rectangle ShapeCollection::GetBoundingBox() const {
  std::optional<Point> start;
  if (!rectangles_.empty()) {
    start = rectangles_.front()->lower_left();
  } else if (!polygons_.empty()) {
    start = polygons_.front()->GetBoundingBox().lower_left();
  } else if (!ports_.empty()) {
    start = ports_.front()->GetBoundingBox().lower_left();
  }

  int64_t min_x = start.value().x();
  int64_t max_x = start.value().x();
  int64_t min_y = start.value().y();
  int64_t max_y = start.value().y();

  for (const auto &rectangle : rectangles_) {
    min_x = std::min(rectangle->lower_left().x(), min_x);
    min_y = std::min(rectangle->lower_left().y(), min_y);
    max_x = std::max(rectangle->upper_right().x(), max_x);
    max_y = std::max(rectangle->upper_right().y(), max_y);
  }

  for (const auto &polygon : polygons_) {
    geometry::Rectangle bounding_box = polygon->GetBoundingBox();
    const Point &lower_left = bounding_box.lower_left();
    const Point &upper_right = bounding_box.upper_right();
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  for (const auto &port : ports_) {
    min_x = std::min(port->lower_left().x(), min_x);
    min_y = std::min(port->lower_left().y(), min_y);
    max_x = std::max(port->upper_right().x(), max_x);
    max_y = std::max(port->upper_right().y(), max_y);
  }
  return Rectangle({min_x, min_y}, {max_x, max_y});
}

}   // namespace geometry
}   // namespace bfg
