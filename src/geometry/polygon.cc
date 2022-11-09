#include "polygon.h"

#include <ostream>
#include <sstream>
#include <utility>

#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

// Compute the points at which a line intersects with a polygon, returning the
// pairs of points at which the line enters and then exits the polygon. Uses
// the same principle as polygon filling algorithms: Lines start at infinity
// and so much enter a polygon on the first intersection, and leave on the next.
//
// Intersections at a corner are trickier: they can be thought of as entering
// and exiting the polygon at the same point. We discover this "for free" since
// we'll compute an intersection with both lines anyway..
//
// If the polygon has two identical-gradient line segments in a row, we must
// discard one or merge them. Otherwise we will get a duplicate point.
void Polygon::IntersectingPoints(
    const Line &line,
    std::vector<std::pair<Point, Point>> *points) const {
  if (vertices_.empty()) {
    LOG(WARNING) << "Polygon with no vertices!";
    return;
  }

  std::vector<Point> intersections;
  std::vector<Line> segments;
  const Point *point = nullptr;
  const Point *last_point = &*vertices_.begin();
  Line segment;
  Line last_segment = Line(vertices_.back(), vertices_.front());
  for (size_t i = 1; i < vertices_.size() + 1;
       ++i, last_point = point, last_segment = segment) {
    // Get next point, wrapping around to the front if we're at the end.
    point = &vertices_[i % vertices_.size()];
    segment = Line(*last_point, *point);
    VLOG(12) << "Checking " << segment;
    // NOTE(aryap): We don't need to check this because we check for
    // intersections in the line's bounds and our test for ingress/egress will
    // determine determine if the point should be de-duplicated.
    // if (segment.IsSameInfiniteLine(last_segment)) {
    //   // Skip segment entirely.
    //   continue;
    // }
    segments.push_back(segment);
    Point intersection;
    bool incident;
    if (!segment.IntersectsInBounds(line, &incident, &intersection)) {
      continue;
    }
    if (incident) {
      VLOG(12) << segment << " is incident on " << line;
      // The line falls on a edge of the polygon directly, so skip the check
      // for a corner and whether we ingress/egress. Because do.
      intersections.push_back(segment.end());
  
      // When the line is incident on a segment, we have to check the previous
      // and following segments to determine if it is an ingress/egress event.
      //
      //   |          |
      //   v          v
      //                  
      //   +--      --+
      //   |          |
      // --+        --+
      //  (a)        (b)
      //
      // (a) a single event
      // (b) two events

      const Point *next_point = &vertices_[(i + 1) % vertices_.size()];
      Line next_segment = Line(*point, *next_point);
      int64_t dot_product = last_segment.DotProduct(next_segment);
      //  a . b = ||a|| ||b|| cos (theta)
      //  a . b < 0 iff cos (theta) < 0 iff pi/2 <= theta <= 3*pi/2
      if (dot_product < 0) {
        // Skip the next segment.
        segment = next_segment;
        point = next_point;
        ++i;
      }
      continue;
    }
    VLOG(12) << segment << " intersects " << line << " at " << intersection;
    if (intersection == segment.start()) {
      // Since the line intersected with a segment boundary, we have to check
      // if the line has gone through the Polygon's hull. We do that by
      // completing the triangle between the two segments and seeing if the
      // line intersects that too.
      Line completion_line = Line(segment.end(), last_segment.start());
      Point completion_intersection;
      if (!completion_line.IntersectsInBounds(
              line, &incident, &completion_intersection)) {
        // The line has not entered nor exited the polygon and should be
        // recorded twice (here and when the other segment encounters it),
        // since that indicates ingress or egress.
        intersections.push_back(intersection);
      } else {
        VLOG(12) << "corner " << intersection << " is an ingress or egress";
      }
      continue;
    }
    if (i < vertices_.size() &&
        !intersections.empty() && intersections.back() == intersection) {
      continue;
    }
    if (!intersections.empty() && intersections.front() == intersection) {
      intersections.insert(intersections.begin(), intersection);
    } else {
      intersections.push_back(intersection);
    }
  }

  for (const auto &point : intersections) {
    VLOG(12) << point;
  }

  LOG_IF(FATAL, intersections.size() % 2 != 0)
      << "Expected pairs of intersecting points.";

  Point outside = GetBoundingBox().PointOnLineOutside(line);
  VLOG(12) << "outside point: " << outside;
  std::sort(intersections.begin(), intersections.end(),
            [&](const Point &lhs, const Point &rhs) {
    return outside.L2SquaredDistanceTo(lhs) > outside.L2SquaredDistanceTo(rhs);
  });

  if (intersections.empty())
    return;

  for (size_t i = 0; i < intersections.size() - 1; i += 2) {
    points->push_back({intersections[i], intersections[i + 1]});
  }
}

void Polygon::MirrorY() {
  for (Point &point : vertices_) {
    point.MirrorY();
  }
}

void Polygon::MirrorX() {
  for (Point &point : vertices_) {
    point.MirrorX();
  }
}

void Polygon::FlipHorizontal() {
  Point lower_left = GetBoundingBox().lower_left();
  MirrorY();
  MoveLowerLeftTo(lower_left);
}

void Polygon::FlipVertical() {
  Point lower_left = GetBoundingBox().lower_left();
  MirrorX();
  MoveLowerLeftTo(lower_left);
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

void Polygon::Rotate(int32_t degrees_ccw) {
  for (auto &vertex : vertices_) {
    vertex.Rotate(degrees_ccw);
  }
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

  Rectangle bounding_box = Rectangle(lower_left, upper_right);
  bounding_box.set_layer(layer_);
  return bounding_box;
}

const std::string Polygon::Describe() const {
  std::stringstream ss;
  for (const auto &point : vertices_) {
    ss << point << " ";
  }
  return ss.str();
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
