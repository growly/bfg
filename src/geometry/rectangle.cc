#include "rectangle.h"

#include <algorithm>
#include <optional>
#include <glog/logging.h>
#include <sstream>

#include "point.h"
#include "../physical_properties_database.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace geometry {

double Rectangle::ClosestDistanceBetween(
    const Rectangle &lhs, const Rectangle &rhs) {
  if (lhs.Overlaps(rhs))
    return 0;

  // Don't overthink it. If the two rectangles do not overlap, there are only a
  // few places they can be relative to one another:
  //
  //                |                |
  //      top       |                |     top
  //      left      |                |     right
  //                |                |
  // ---------------+----------------+----------------
  //                |                |
  //                |     'lhs'      |     right
  //                |   rectangle    |
  //                |                |
  // ---------------+----------------+----------------
  //                |                |
  //                |                |     bottom
  //                |                |     right
  //                |                |
  bool top = lhs.upper_right().y() < rhs.lower_left().y();
  bool bottom = lhs.lower_left().y() > rhs.upper_right().y();
  bool right = lhs.upper_right().x() < rhs.lower_left().x();
  bool left = lhs.lower_left().x() > rhs.upper_right().x();
  if (top && right) {
    return lhs.upper_right().L2DistanceTo(rhs.lower_left());
  } else if (bottom && right) {
    return lhs.LowerRight().L2DistanceTo(rhs.UpperLeft());
  } else if (bottom && left) {
    return lhs.lower_left().L2DistanceTo(rhs.upper_right());
  } else if (top && left) {
    return lhs.UpperLeft().L2DistanceTo(rhs.LowerRight());
  } else if (right) {
    return std::abs(lhs.upper_right().x() - rhs.lower_left().x());
  } else if (bottom) {
    return std::abs(lhs.lower_left().y() - rhs.upper_right().y());
  } else if (left) {
    return std::abs(lhs.lower_left().x() - rhs.upper_right().x());
  } else if (top) {
    return std::abs(lhs.upper_right().y() - rhs.lower_left().y());
  } else {
    LOG(FATAL) << "If " << lhs << " and " << rhs << " don't overlap, "
               << "how did we get here?";
  }
  return 0;
}

void Rectangle::ExpandBounds(const Rectangle &subsume,
                             Rectangle *bounding_box) {
  bounding_box->lower_left_.set_x(std::min(
      subsume.lower_left().x(), bounding_box->lower_left_.x()));
  bounding_box->lower_left_.set_y(std::min(
      subsume.lower_left().y(), bounding_box->lower_left_.y()));
  bounding_box->upper_right_.set_x(std::max(
      subsume.upper_right().x(), bounding_box->upper_right_.x()));
  bounding_box->upper_right_.set_y(std::max(
      subsume.upper_right().y(), bounding_box->upper_right_.y()));
}

Rectangle Rectangle::CentredAt(
    const Point &centre, uint64_t width, uint64_t height) {
  Point lower_left = centre - Point{
      static_cast<int64_t>(width) / 2,
      static_cast<int64_t>(height) / 2};
  return Rectangle(lower_left, width, height);
}

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

bool Rectangle::Intersects(const Point &point) const {
  return point.x() >= lower_left_.x() && point.x() <= upper_right_.x() &&
         point.y() >= lower_left_.y() && point.y() <= upper_right_.y();
}

bool Rectangle::Intersects(const Point &point, int64_t margin) const {
  if (margin == 0) {
    return Intersects(point);
  }
  Rectangle modified = WithPadding(margin);
  return modified.Intersects(point);
}

std::vector<PointPair> Rectangle::IntersectingPoints(const Line &line) const {
  std::vector<Line> boundary_lines = GetBoundaryLines();
  std::vector<Point> intersections;
  // Unlike for Polygon, we have a very limited number of cases to deal with. A
  // line can intersect a Rectangle at at most two points. If it intersects a
  // corner, we record that as two points, in keeping with the convention
  // defined for intersections with a Polygon.
  //
  // The only exceptional handling we need is for lines that are incident on a
  // boundary line. In that case the start and end of the boundary line are the
  // intersections and we don't consider any other boundary lines.
  //
  // FIXME(aryap): DUDE a DIAGONAL line that hits two DIAGONALLY OPPOSITE
  // corners will yield 4 intersections! Maybe we disable end hit testing.
  bool any_intersection = false;
  for (const Line &boundary : boundary_lines) {
    Point hit;
    bool incident = false;
    bool unused_is_start_or_end = false;
    bool intersects = boundary.IntersectsInBounds(line,
                                                  &incident,
                                                  &unused_is_start_or_end,
                                                  &hit,
                                                  true,     // ignore_start
                                                  false);   // ignore_end
    if (intersects) {
      any_intersection = true;
      if (incident) {
        intersections = {boundary.start(), boundary.end()};
        break;
      }
      intersections.push_back(hit);
    }
  }

  if (!any_intersection) {
    LOG_IF(FATAL, !intersections.empty())
        << "No intersections but intersections exist";
    return {};
  }
  if (intersections.size() == 1) {
    intersections.push_back(intersections.front());
  }

  LOG_IF(FATAL, intersections.size() != 2)
      << "There should be exactly two intersections of a line and a rectangle";

  std::sort(
      intersections.begin(), intersections.end(),
      [&](const Point &lhs, const Point &rhs) {
          return line.ProjectionCoefficient(lhs) <
              line.ProjectionCoefficient(rhs);
      });

  PointPair intersection = {intersections.front(), intersections.back()};
  return {intersection};
}

std::vector<Line> Rectangle::GetBoundaryLines() const {
  Point upper_left = UpperLeft();
  Point lower_right = LowerRight();
  std::vector<Line> lines = {
      Line(lower_left_, upper_left),
      Line(upper_left, upper_right_),
      Line(upper_right_, lower_right),
      Line(lower_right, lower_left_)
  };
  return lines;
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

Rectangle Rectangle::WithPadding(int64_t padding) const {
  Point lower_left = lower_left_ - Point {padding, padding};
  Point upper_right = upper_right_ + Point {padding, padding};
  if (lower_left.x() > upper_right.x()) {
    lower_left.set_x((lower_left.x() + upper_right.x())/2);
    upper_right.set_x(lower_left.x());
  }
  if (lower_left.y() > upper_right.y()) {
    lower_left.set_y((lower_left.y() + upper_right.y())/2);
    upper_right.set_y(lower_left.y());
  }
  return {lower_left, upper_right};
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

const std::string Rectangle::Describe() const {
  std::stringstream ss;
  ss << "[Rectangle " << lower_left_ << " " << upper_right_ << "]";
  return ss.str();
}

bool operator==(const Rectangle &lhs, const Rectangle &rhs) {
  return lhs.lower_left() == rhs.lower_left()
      && lhs.upper_right() == rhs.upper_right();
}

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os, const geometry::Rectangle &rectangle) {
  os << rectangle.Describe();
  return os;
}

}  // namespace bfg
