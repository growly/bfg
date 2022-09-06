#include "polygon.h"

#include <ostream>

#include "rectangle.h"

namespace bfg {
namespace geometry {

void Polygon::FlipHorizontal() {
  for (Point &point : vertices_) {
    point.FlipHorizontal();
  }
}

void Polygon::FlipVertical() {
  for (Point &point : vertices_) {
    point.FlipVertical();
  }
}

void Polygon::Translate(const Point &offset) {
  Rectangle bounding_box = GetBoundingBox();
  for (Point &point : vertices_) {
    point += offset;
  }
}

void Polygon::ResetOrigin() {
  Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

const Rectangle Polygon::GetBoundingBox() const {
  Point lower_left;
  Point upper_right;

  if (!vertices_.empty()) {
    lower_left = vertices_.front();
    upper_right = lower_left;
    for (const auto &point : vertices_) {
      lower_left.set_x(std::min(lower_left.x(), point.x()));
      lower_left.set_y(std::min(lower_left.y(), point.y()));
      upper_right.set_x(std::max(upper_right.x(), point.x()));
      upper_right.set_y(std::max(upper_right.y(), point.y()));
    }
  }

  return Rectangle(lower_left, upper_right);
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Polygon &polygon) {
  for (size_t i = 0; i < polygon.vertices().size(); ++i) {
    os << polygon.vertices().at(i);
    if (i != polygon.vertices().size() - 1) {
      os << ", ";
    }
  }
  return os;
}

}  // namespace bfg
