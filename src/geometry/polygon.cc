#include "polygon.h"

#include <algorithm>
#include <functional>
#include <ostream>
#include <optional>
#include <set>
#include <sstream>
#include <utility>

#include <glog/logging.h>

#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

void Polygon::ResolveIntersectingPointsFrom(
    const std::vector<PointOrChoice> &choices,
    const Point &reference_point,
    std::vector<std::pair<Point, Point>> *intersections) {
  std::vector<PointOrChoice> choices_copy(choices.begin(), choices.end());

  for (const auto &choice : choices) {
    if (choice.unique()) {
      LOG(INFO) << *choice.unique();
    } else {
      Point closest = choice.ClosestPointTo(reference_point);
      Point furthest = choice.FurthestPointFrom(reference_point);
      LOG(INFO) << "(" << closest << " or " << furthest << ")";
    }
  }

  std::vector<Point> sorted;
  bool outside = true;
  while (!choices_copy.empty()) {
    // We have to repeatedly choose the next closest point to the reference
    // point from the available choices. Whether we are currently inside or
    // outside the polygon affects whether we choose the closest or furthest
    // point from a set of choices in a given PointOrChoice structure.
    //
    // We do not need to maintain a sorted structure; we only need the minimum
    // in the collection at a given time.
    std::function<bool (const PointOrChoice&, const PointOrChoice&)> comparator;
    if (outside) {
      comparator = [&](const PointOrChoice &lhs, const PointOrChoice &rhs) {
        const Point left = lhs.ClosestPointTo(reference_point);
        const Point right = rhs.ClosestPointTo(reference_point);
        return reference_point.L2SquaredDistanceTo(
            left) > reference_point.L2SquaredDistanceTo(right);
      };
    } else {
      comparator = [&](const PointOrChoice &lhs, const PointOrChoice &rhs) {
        //const Point *left = lhs.FurthestPointFrom(reference_point);
        //const Point *right = rhs.FurthestPointFrom(reference_point);
        const Point left = lhs.ClosestPointTo(reference_point);
        const Point right = rhs.ClosestPointTo(reference_point);
        return reference_point.L2SquaredDistanceTo(
            left) > reference_point.L2SquaredDistanceTo(right);
      };
    }
    auto it = std::min_element(
        choices_copy.begin(), choices_copy.end(), comparator);
    LOG_IF(FATAL, it == choices_copy.end())
        << "choices_copy was not empty so at least one min element must exist";
    Point next_point = outside ?
        it->ClosestPointTo(reference_point) :
        it->FurthestPointFrom(reference_point);
    sorted.push_back(next_point);
    choices_copy.erase(it);
    outside = !outside;
  }

  LOG(INFO) << "sorted:";
  for (const auto &point : sorted) {
    LOG(INFO) << point;
  }

  for (size_t i = 0; i < sorted.size() - 1; i += 2) {
    intersections->push_back({sorted[i], sorted[i + 1]});
  }
}

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
  points->clear();
  if (vertices_.empty()) {
    LOG(WARNING) << "Polygon with no vertices!";
    return;
  }

  std::vector<PointOrChoice> intersections;
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
    // determine if the point should be de-duplicated.
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
      // for a corner and whether we ingress/egress. Because we do.
  
      // When the line is incident on a segment, we have to check the previous
      // and following segments to determine if it is an ingress/egress event.
      //
      //   |          |
      //   v          v
      //                  
      //   +<-      ->+
      //   |          |
      // <-+        <-+
      //  (a)        (b)
      //
      // (a) a single event
      // (b) two events

      const Point *next_point = &vertices_[(i + 1) % vertices_.size()];
      Line next_segment = Line(*point, *next_point);

      if (segment.IsSameInfiniteLine(next_segment)) {
        // If there are two lines in series both incident, we skip the current
        // one and hope that the next one yields the correct intersection point
        // (i.e. the end of that line).
        //
        // Set up segment to be the last_segment, so that at the end of the loop
        // it is set back to itself. This makes it like the vertical segment
        // didn't exist.
        //
        //   |
        //   v
        //
        //   +--
        //   | <- skip
        //   +
        //   |
        // --+
        // ^
        // Preserve as last_segment on the next iteration.
        segment = last_segment;
        continue;
      }

      int64_t dot_product = last_segment.DotProduct(next_segment);
      //  a . b = ||a|| ||b|| cos (theta)
      //  a . b < 0 iff cos (theta) < 0 iff pi/2 <= theta <= 3*pi/2
      if (dot_product < 0) {
        // Case (b) above; we keep the intersection from the previous segment
        // and from the end of this segment, since both are boundaries of the
        // polygon.
        intersections.emplace_back(segment.end());
      } else if (dot_product > 0) {
        // FIXME(aryap): Two potential problems here:
        // 1) what if the last segment was a vertical segment we skipped already
        //    - last_segment is not updated so this should 
        // 2) what if this is the start of the line
        // Remove the last intersection, since it also falls on this vertical
        // line and needs to be considered as a choice of intersecting point
        // depending on where we are in the overall polygon.
        if (!intersections.empty()) intersections.pop_back();
        intersections.emplace_back(std::set<Point> {
            segment.end(), last_segment.end()
        });
      }

      // Skip the next segment since the intersection with this vertical line
      // was recorded as an intersection with last_segment, and we're done (case
      // (a) above) or we added the intersection from the end of this segment
      // already (case (b) above).
      segment = next_segment;
      point = next_point;
      ++i;
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
        intersections.emplace_back(intersection);
      } else {
        VLOG(12) << "corner " << intersection << " is an ingress or egress";
      }
      continue;
    }
    if (i < vertices_.size() &&
        !intersections.empty() &&
        intersections.back().Contains(intersection) &&
        intersection == segment.start()) {
      // Do not add duplicate intersections when they occur at the start or end
      // of line segments. (If they occur at the middle of line segments they
      // must come from multiple lines.)
      //
      // NOTE(aryap): A more elegant way to do this would be to check for
      // intersection only including one of line bounds, not both.
      continue;
    }
    if (!intersections.empty() &&
        intersections.front().Contains(intersection)) {
      intersections.insert(intersections.begin(), PointOrChoice(intersection));
    } else {
      intersections.push_back(PointOrChoice(intersection));
    }
  }

  if (intersections.empty())
    return;

  LOG_IF(FATAL, intersections.size() % 2 != 0)
      << "Expected pairs of intersecting point choices, got "
      << intersections.size() << " intersecting " << Describe() << " with "
      << line.Describe();

  Point outside_point = GetBoundingBox().PointOnLineOutside(line);
  VLOG(12) << "outside point: " << outside_point;

  // Go through all points and choices among points. 
  ResolveIntersectingPointsFrom(intersections, outside_point, points);
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
