#include "rectangle.h"
#include <algorithm>
#include <glog/logging.h>

#include "point.h"
#include "../physical_properties_database.h"

#include "vlsir/layout/raw.pb.h"

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

void Rectangle::Rotate(int32_t degrees_ccw) {
  LOG_IF(WARNING, degrees_ccw % 90 != 0)
      << "Rectangle only supports rectilinear shapes and will be "
      << "massaged as such.";
  // This is the slow but more general way to do this:
  Rectangle bb = BoundingBoxIfRotated(Point(0, 0), degrees_ccw);
  lower_left_ = bb.lower_left_;
  upper_right_ = bb.upper_right_;
}

Point Rectangle::PointOnLineOutside(const Line &line) const {
  if (line.IsVertical()) {
    return Point(line.start().x(), lower_left_.y() - 1);
  }
  double y = line.Gradient() * static_cast<double>(
      lower_left_.x() - 1) + line.Offset();
  return Point(lower_left_.x() - 1, static_cast<int64_t>(y));
}

Rectangle Rectangle::BoundingBoxIfRotated(
    const Point &about, int32_t degrees_ccw) const {
  Point lower_left = lower_left_ - about;
  Point upper_left = UpperLeft() - about;
  Point upper_right = upper_right_ - about;
  Point lower_right = LowerRight() - about;

  lower_left.Rotate(degrees_ccw);
  upper_left.Rotate(degrees_ccw);
  upper_right.Rotate(degrees_ccw);
  lower_right.Rotate(degrees_ccw);

  std::vector x_points = {
      lower_left.x(), upper_left.x(), upper_right.x(), lower_right.x() };
  std::vector y_points = {
      lower_left.y(), upper_left.y(), upper_right.y(), lower_right.y() };

  int64_t min_x = *std::min_element(x_points.begin(), x_points.end());
  int64_t max_x = *std::max_element(x_points.begin(), x_points.end());
  int64_t min_y = *std::min_element(y_points.begin(), y_points.end());
  int64_t max_y = *std::max_element(y_points.begin(), y_points.end());

  Rectangle rotated = Rectangle(Point(min_x, min_y) + about,
                                Point(max_x, max_y) + about);
  return rotated;
}

::vlsir::raw::Rectangle Rectangle::ToVLSIRRectangle(
    const PhysicalPropertiesDatabase &db) const {
  ::vlsir::raw::Rectangle rect_pb;
  rect_pb.mutable_lower_left()->set_x(db.ToExternalUnits(lower_left_.x()));
  rect_pb.mutable_lower_left()->set_y(db.ToExternalUnits(lower_left_.y()));
  rect_pb.set_width(db.ToExternalUnits(Width()));
  rect_pb.set_height(db.ToExternalUnits(Height()));
  return rect_pb;
}

::vlsir::raw::Polygon Rectangle::ToVLSIRPolygon(
    const PhysicalPropertiesDatabase &db) const {
  ::vlsir::raw::Polygon polygon_pb;
  std::vector<Point> outline_points = {
      lower_left_,
      UpperLeft(),
      upper_right_,
      LowerRight()
  };
  for (const auto &point : outline_points) {
    ::vlsir::raw::Point *point_pb = polygon_pb.add_vertices();
    point_pb->set_x(db.ToExternalUnits(point.x()));
    point_pb->set_y(db.ToExternalUnits(point.y()));
  }
  return polygon_pb;
}

bool operator==(const Rectangle &lhs, const Rectangle &rhs) {
  return lhs.lower_left() == rhs.lower_left()
      && lhs.upper_right() == rhs.upper_right();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Rectangle &rectangle) {
  os << "[Rectangle " << rectangle.lower_left()
     << " " << rectangle.upper_right() << "]";
  return os;
}

}  // namespace bfg
