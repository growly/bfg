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

std::vector<PointPair> Polygon::ResolveIntersectingPointsFrom(
    const std::vector<PointOrChoice> &choices,
    const Point &reference_point) {
  std::vector<PointPair> intersections;
  std::vector<PointOrChoice> choices_copy(choices);

  for (const auto &choice : choices) {
    VLOG(13) << choice;
  }

  std::set<Point> poisoned;
  std::vector<Point> sorted;
  bool outside = true;
  bool check_for_dupes = false;
  while (!choices_copy.empty()) {
    // We have to repeatedly choose the next closest point to the reference
    // point from the available choices. Whether we are currently inside or
    // outside the polygon affects whether we choose the closest or furthest
    // point from a set of choices in a given PointOrChoice structure.
    //
    // We do not need to maintain a sorted structure; we only need the minimum
    // in the collection at a given time.
    std::function<bool (const PointOrChoice&, const PointOrChoice&)>
        comparator = [&](const PointOrChoice &lhs, const PointOrChoice &rhs) {
      const Point left = lhs.ClosestPointTo(reference_point);
      const Point right = rhs.ClosestPointTo(reference_point);
      return reference_point.L2SquaredDistanceTo(
          left) < reference_point.L2SquaredDistanceTo(right);
    };
    auto it = std::min_element(
        choices_copy.begin(), choices_copy.end(), comparator);
    LOG_IF(FATAL, it == choices_copy.end())
        << "choices_copy was not empty so at least one min element must exist";
    const PointOrChoice &choice = *it;
    // If the PointOrChoice contains a single point then these just yield the
    // same point.
    Point next_point = outside ?
        choice.ClosestPointTo(reference_point) :
        choice.FurthestPointFrom(reference_point);

    if (poisoned.find(next_point) != poisoned.end()) {
      choices_copy.erase(it);
      continue;
    }

    // Any time we hit a span we should make sure to check for dupes on the next
    // iteration (since we could be duplicating one end of the span in the
    // points list).
    if (choice.choose_one()) {
      check_for_dupes = true;
    }

    // By the strict order of distances from the reference point, we wouldn't
    // normally have to go back and make sure we're not adding a duplicate point
    // somewhere well back in the sorted list. But since we add both ends of an
    // entire span if we hit a choice.choose_one() (i.e. a line on which we are
    // entirely incident), we can break the ordering by inserting one point out
    // of order.
    bool already_exists = check_for_dupes && (
        std::find(sorted.begin(), sorted.end(), next_point) != sorted.end());

    // Reset the check for subsequent iterations if this choice is not a choice
    // between two points.
    if (!choice.choose_one()) {
      check_for_dupes = false;
    }

    if (choice.choose_one() && !choice.crosses_boundary()) {
      // Incident on an edge of the polygon but do not cross the edge.

      if (outside) {
        // We're outside, so add the span of choices (both closest and
        // furthest).
        //
        // If there are multiple choices, we will take the extremes and add them
        // to the list. We have to make sure that this not yield any accidental
        // duplicates. This happens when a line is incident on a line in a
        // polygon - in that case, the start point of the previous or next line
        // segment in the polygon will also yield an intersection. We can rely
        // on that but only in one direction (clockwise or anti-clockwise around
        // the segments), so instead we don't rely on it. We always add both the
        // closest and furthest points on the segment and then de-dupe them
        // later.
        Point other_next_point = choice.FurthestPointFrom(reference_point);
        if (!already_exists) {
          sorted.push_back(next_point);
        }
        sorted.push_back(other_next_point);
        // choices_copy.erase(it);
        // check_for_dupes = true;
      } else {
        // We're still inside, so ignore both ends of the span and make sure
        // they don't get included again:
        poisoned.insert(next_point);

        Point other_end = outside ?
            choice.FurthestPointFrom(reference_point) :
            choice.ClosestPointTo(reference_point);
        poisoned.insert(other_end);
      }

      choices_copy.erase(it);
      continue;
    }

    bool poison_other_end = choice.choose_one() && choice.crosses_boundary();
    if (poison_other_end) {
      Point other_end = outside ?
          choice.FurthestPointFrom(reference_point) :
          choice.ClosestPointTo(reference_point);
      poisoned.insert(other_end);
    }

    // If we're outside and encounter a non-boundary-crossing corner, we have to
    // repeat the intersection point to indicate that the line entered and
    // exited the polygon at that single point:
    bool outside_corner =
        outside && choice.is_corner() && !choice.crosses_boundary();
    if (outside_corner && !already_exists) {
      sorted.push_back(next_point);
      sorted.push_back(next_point);
      choices_copy.erase(it);
      continue;
    }

    // Inside corners are not considered a departure from the polygon. In all
    // other cases, the considered point is a reported intersection.
    bool inside_corner =
        !outside && choice.is_corner() && !choice.crosses_boundary();
    if (inside_corner) {
      choices_copy.erase(it);
      continue;
    }

    // The base case.
    if (!already_exists) {
      sorted.push_back(next_point);
    }

    choices_copy.erase(it);

    if (!already_exists && choice.crosses_boundary()) {
      outside = !outside;
    }
  }

  VLOG(13) << "sorted: ";
  for (const auto &point : sorted) {
    VLOG(13) << point;
  }

  LOG_IF(WARNING, sorted.size() % 2 != 0)
      << "Undefined behaviour: "
      << "Expected pairs of intersecting point choices, got " << sorted.size();

  for (size_t i = 0; i < sorted.size() - 1; i += 2) {
    intersections.push_back({sorted[i], sorted[i + 1]});
  }
  return intersections;
}

std::vector<PointPair> Polygon::IntersectingPoints(const Line &line) const {
  if (vertices_.empty()) {
    LOG(WARNING) << "Polygon with no vertices!";
    return std::vector<PointPair>();
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
    VLOG(16) << "Checking " << segment;
    segments.push_back(segment);
    Point intersection;
    bool incident;
    bool at_start;
    if (!segment.IntersectsInBounds(
            line,
            &incident,
            &at_start,
            &intersection,
            true)) {  // Ignore the ending point of the line segment.
      continue;
    }

    PointOrChoice choice;

    if (incident) {
      VLOG(12) << segment << " is incident on " << line;

      const Point *next_point = &vertices_[(i + 1) % vertices_.size()];
      Line next_segment = Line(*point, *next_point);

      if (segment.IsSameInfiniteLine(next_segment)) {
        segment = last_segment;
        continue;
      }

      //  a . b = ||a|| ||b|| cos (theta)
      //  a . b < 0 iff cos (theta) < 0 iff pi/2 <= theta <= 3*pi/2
      int64_t dot_product = last_segment.DotProduct(next_segment);
      // Use of last_segment is important because it includes the span of any
      // other line segments that are on the same infinite line as this one
      // and were thus skipped:
      choice.set_choose_one(
          std::set<Point> { segment.end(), last_segment.end() });

      if (dot_product < 0) {
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
        // Case (b); we keep the intersection from the previous segment
        // and from the end of this segment, since both are boundaries of the
        // polygon.
        choice.set_maybe_internal(true);
      } else if (dot_product > 0) {
        // Case (a): we crossed a line.
        choice.set_crosses_boundary(true);
      }
    } else if (at_start) {
      // Since the line intersected with a segment boundary, we have to check
      // if the line has gone through the Polygon's hull. We do that by
      // completing the triangle between the two segments and seeing if the
      // line intersects that too.
      Line completion_line = Line(segment.end(), last_segment.start());
      Point ignored_point;
      bool ignored;
      choice.set_crosses_boundary(
          completion_line.IntersectsInBounds(
              line, &incident, &ignored, &ignored_point));
      choice.set_is_corner(true);
      choice.set_unique(intersection);
    } else {
      // The base case. A single intersecting point somewhere in the middle of
      // the segment span.
      choice.set_crosses_boundary(true);
      choice.set_unique(intersection);
    }

    VLOG(12) << segment << " intersects " << line << " at " << intersection;
    intersections.push_back(choice);
  }

  if (intersections.empty())
    return std::vector<PointPair>();

  Point outside_point = GetBoundingBox().PointOnLineOutside(line);
  VLOG(12) << "outside point: " << outside_point;
  VLOG(13) << *this;

  // Go through all points and choices among points. 
  return ResolveIntersectingPointsFrom(intersections, outside_point);
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
//void IntersectingPoints2(
//    const Line &line,
//    std::vector<std::pair<Point, Point>> *points) const {
//  points->clear();
//  if (vertices_.empty()) {
//    LOG(WARNING) << "Polygon with no vertices!";
//    return;
//  }
//
//  std::vector<PointOrChoice> intersections;
//  std::vector<Line> segments;
//  const Point *point = nullptr;
//  const Point *last_point = &*vertices_.begin();
//  Line segment;
//  Line last_segment = Line(vertices_.back(), vertices_.front());
//  for (size_t i = 1; i < vertices_.size() + 1;
//       ++i, last_point = point, last_segment = segment) {
//    // Get next point, wrapping around to the front if we're at the end.
//    point = &vertices_[i % vertices_.size()];
//    segment = Line(*last_point, *point);
//    VLOG(12) << "Checking " << segment;
//    // NOTE(aryap): We don't need to check this because we check for
//    // intersections in the line's bounds and our test for ingress/egress will
//    // determine if the point should be de-duplicated.
//    // if (segment.IsSameInfiniteLine(last_segment)) {
//    //   // Skip segment entirely.
//    //   continue;
//    // }
//    segments.push_back(segment);
//    Point intersection;
//    bool incident;
//    if (!segment.IntersectsInBounds(line, false, &incident, &intersection)) {
//      continue;
//    }
//    if (incident) {
//      VLOG(12) << segment << " is incident on " << line;
//      // The line falls on a edge of the polygon directly, so skip the check
//      // for a corner and whether we ingress/egress. Because we do.
//  
//      // When the line is incident on a segment, we have to check the previous
//      // and following segments to determine if it is an ingress/egress event.
//      //
//      //   |          |
//      //   v          v
//      //                  
//      //   +<-      ->+
//      //   |          |
//      // <-+        <-+
//      //  (a)        (b)
//      //
//      // (a) a single event
//      // (b) two events
//
//      const Point *next_point = &vertices_[(i + 1) % vertices_.size()];
//      Line next_segment = Line(*point, *next_point);
//
//      if (segment.IsSameInfiniteLine(next_segment)) {
//        // If there are two lines in series both incident, we skip the current
//        // one and hope that the next one yields the correct intersection point
//        // (i.e. the end of that line).
//        //
//        // Set up segment to be the last_segment, so that at the end of the loop
//        // it is set back to itself. This makes it like the vertical segment
//        // didn't exist.
//        //
//        //   |
//        //   v
//        //
//        //   +--
//        //   | <- skip
//        //   +
//        //   |
//        // --+
//        // ^
//        // Preserve as last_segment on the next iteration.
//        segment = last_segment;
//        continue;
//      }
//
//      int64_t dot_product = last_segment.DotProduct(next_segment);
//      //  a . b = ||a|| ||b|| cos (theta)
//      //  a . b < 0 iff cos (theta) < 0 iff pi/2 <= theta <= 3*pi/2
//      if (dot_product < 0) {
//        // Case (b) above; we keep the intersection from the previous segment
//        // and from the end of this segment, since both are boundaries of the
//        // polygon.
//        intersections.emplace_back(segment.end());
//      } else if (dot_product > 0) {
//        // FIXME(aryap): Two potential problems here:
//        // 1) what if the last segment was a vertical segment we skipped already
//        //    - last_segment is not updated so this should 
//        // 2) what if this is the start of the line
//        // Remove the last intersection, since it also falls on this vertical
//        // line and needs to be considered as a choice of intersecting point
//        // depending on where we are in the overall polygon.
//        if (!intersections.empty()) intersections.pop_back();
//        intersections.emplace_back(std::set<Point> {
//            segment.end(), last_segment.end()
//        });
//      }
//
//      // Skip the next segment since the intersection with this vertical line
//      // was recorded as an intersection with last_segment, and we're done (case
//      // (a) above) or we added the intersection from the end of this segment
//      // already (case (b) above).
//      segment = next_segment;
//      point = next_point;
//      ++i;
//      continue;
//    }
//    VLOG(12) << segment << " intersects " << line << " at " << intersection;
//    if (intersection == segment.start()) {
//      // Since the line intersected with a segment boundary, we have to check
//      // if the line has gone through the Polygon's hull. We do that by
//      // completing the triangle between the two segments and seeing if the
//      // line intersects that too.
//      Line completion_line = Line(segment.end(), last_segment.start());
//      Point completion_intersection;
//      if (!completion_line.IntersectsInBounds(
//              line, &incident, &completion_intersection)) {
//        // The line has not entered nor exited the polygon and should be
//        // recorded twice (here and when the other segment encounters it),
//        // since that indicates ingress or egress.
//        intersections.emplace_back(intersection);
//      } else {
//        VLOG(12) << "corner " << intersection << " is an ingress or egress";
//      }
//      continue;
//    }
//    if (i < vertices_.size() &&
//        !intersections.empty() &&
//        intersections.back().Contains(intersection) &&
//        intersection == segment.start()) {
//      // Do not add duplicate intersections when they occur at the start or end
//      // of line segments. (If they occur at the middle of line segments they
//      // must come from multiple lines.)
//      //
//      // NOTE(aryap): A more elegant way to do this would be to check for
//      // intersection only including one of line bounds, not both.
//      continue;
//    }
//    if (!intersections.empty() &&
//        intersections.front().Contains(intersection)) {
//      intersections.insert(intersections.begin(), PointOrChoice(intersection));
//    } else {
//      intersections.push_back(PointOrChoice(intersection));
//    }
//  }
//
//  if (intersections.empty())
//    return;
//
//  LOG_IF(FATAL, intersections.size() % 2 != 0)
//      << "Expected pairs of intersecting point choices, got "
//      << intersections.size() << " intersecting " << Describe() << " with "
//      << line.Describe();
//
//  Point outside_point = GetBoundingBox().PointOnLineOutside(line);
//  VLOG(12) << "outside point: " << outside_point;
//
//  // Go through all points and choices among points. 
//  ResolveIntersectingPointsFrom(intersections, outside_point, points);
//}

bool Polygon::Overlaps(const Rectangle &rectangle) const {
  Rectangle bounding_box = GetBoundingBox();
  // If there rectangle doesn't overlap the Polygon's bounding box there is no
  // chance of an intersection and we can quit early:
  if (!rectangle.Overlaps(bounding_box)) {
    return false;
  }

  // Now the possibility of an overlap is open.
  //
  // There are two special cases. The first is if the polygon is contained
  // entirely within the rectangle:
  if (rectangle.lower_left().x() < bounding_box.lower_left().x() &&
      rectangle.lower_left().y() < bounding_box.lower_left().y() &&
      rectangle.upper_right().x() > bounding_box.upper_right().x() &&
      rectangle.upper_right().y() > bounding_box.upper_right().y()) {
    return true;
  }

  // The regular case is is that some line on the rectangle intersects some line
  // on the polygon:
  std::vector<Line> rectangle_perimeter;
  rectangle.GetBoundaryLines(&rectangle_perimeter);

  for (size_t i = 0; i < vertices_.size(); ++i) {
    // Check for intersects of any of the lines of the polygon with any of the
    // lines of the Rectangle.
    Line boundary_line = Line(
        vertices_[i], vertices_[(i + 1) % vertices_.size()]);
    for (Line &line : rectangle_perimeter) {
      bool incident_unused;
      Point point_unused;
      if (line.IntersectsInMutualBounds(
              boundary_line,
              &incident_unused,
              &point_unused)) {
        return true;
      }
    }
  }

  // The second special case is if the rectangle is contained entirely within
  // the polygon. At this point we know that the bounding box of the polygon and
  // the rectangle overlap, and that none of their lines intersect, but we need
  // to check if the rectangle is entirely nestled in some crevice not contained
  // by the polygon proper. For example:
  //
  //  rectangle in bounding box of polygon but not contained by it:
  //   |
  //   v
  //  +--+       +---+
  //  |  |       |   |  <- polygon
  //  +--+       |   |
  //             |   |
  //  +----------+   |
  //  |              |
  //  +--------------+

  // Intersect the infinite line defined by the diagonal with the polygon. Since
  // the polygon is closed we should always get pairs of intersections denoting
  // entry/exit into the polygon. Sort these along the line from some point, and
  // then iterate over them until we get to the points on the rectangle we're
  // testing. If in the space between the two points of the rectangle we ever
  // enter the polygon, we have a colision.
  std::vector<Line> test_lines = {
      Line(rectangle.lower_left(), rectangle.upper_right()),  // diagonal
      Line(rectangle.UpperLeft(), rectangle.LowerRight())     // other diagonal
  };

  for (const auto &test : test_lines) {
    // The projection back onto diagonal is measurable as a scalar coefficient
    // multiplied by the original vector diagonal defines. As shorthand we just
    // call these the 'coefficients'.
    double start_coefficient = 0.0;  // Distance from start to start along the
                                     // line is always zero.
    double end_coefficient = test.ProjectionCoefficient(test.end());

    std::vector<PointPair> points = IntersectingPoints(test);

    for (const auto &point_pair : points) {
      const Point &entry = point_pair.first;
      const Point &exit = point_pair.second;

      double entry_coefficient = test.ProjectionCoefficient(entry);
      double exit_coefficient = test.ProjectionCoefficient(exit);

      if (entry_coefficient <= start_coefficient &&
              exit_coefficient >= start_coefficient) {
        return true;
      }
      if (entry_coefficient <= end_coefficient &&
              exit_coefficient >= end_coefficient) {
        return true;
      }
    }
  }

  return false;
}

bool Polygon::HasVertex(const Point &point) const {
  return std::find(
      vertices_.begin(), vertices_.end(), point) != vertices_.end();
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
