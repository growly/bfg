#include "poly_line.h"

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <glog/logging.h>
#include <sstream>
#include <vector>

#include "line_segment.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

std::string PolyLine::Describe() const {
  std::stringstream ss;
  ss << "<" << overhang_start_ << "| " << start_;
  for (const auto &segment : segments_) {
    ss << " |" << segment.width << "| " << segment.end;
  }
  ss << " |" << overhang_end_ << ">";
  return ss.str();
}

void PolyLine::MirrorY() {
  start_.set_x(-start_.x());
  for (LineSegment &segment : segments_) {
    segment.end.set_x(-segment.end.x());
  }
}

void PolyLine::MirrorX() {
  start_.set_y(-start_.y());
  for (LineSegment &segment : segments_) {
    segment.end.set_y(-segment.end.y());
  }
}

void PolyLine::FlipHorizontal() {
  Rectangle bounding_box = GetBoundingBox();
  int64_t left_x = bounding_box.lower_left().x();
  int64_t right_x = bounding_box.upper_right().x();
  start_.set_x(right_x - (start_.x() - left_x));
  for (LineSegment &segment : segments_) {
    segment.end.set_x(right_x - (segment.end.x() - left_x));
  }
}

void PolyLine::FlipVertical() {
  Rectangle bounding_box = GetBoundingBox();
  int64_t upper_y = bounding_box.upper_right().y();
  int64_t lower_y = bounding_box.lower_left().y();
  start_.set_y(upper_y - (start_.y() - lower_y));
  for (LineSegment &segment : segments_) {
    segment.end.set_y(upper_y - (segment.end.y() - lower_y));
  }
}

void PolyLine::Translate(const Point &offset) {
  start_ += offset;
  for (LineSegment &segment : segments_) {
    segment.end += offset;
  }
}

const Rectangle PolyLine::GetBoundingBox() const {
  int64_t min_x = start_.x();
  int64_t max_x = start_.x();
  int64_t min_y = start_.y();
  int64_t max_y = start_.y();

  for (const auto &segment : segments_) {
    min_x = std::min(segment.end.x(), min_x);
    min_y = std::min(segment.end.y(), min_y);
    max_x = std::max(segment.end.x(), max_x);
    max_y = std::max(segment.end.y(), max_y);
  }

  Rectangle bounding_box = Rectangle(Point(min_x, min_y), Point(max_x, max_y));
  bounding_box.set_layer(layer_);
  return bounding_box;
};

void PolyLine::AddSegment(const Point &to, const uint64_t width) {
  const Point &last = segments_.empty() ? start_ : segments_.back().end;
  // Skip duplicate segments.
  if (to == last) {
    LOG(WARNING) << "Skipping duplicate PolyLine segment to " << to;
    return;
  }
  if (to.x() != last.x() && to.y() != last.y()) {
    LOG(FATAL) << "PolyLine segments must be rectilinear. Make sure the new "
               << "x == last_x or y == last_y.";
  }
  segments_.push_back(LineSegment{to, width});
}

void PolyLine::InsertForwardBulgePoint(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line) {

  double half_length = static_cast<double>(coaxial_length) / 2.0;
  uint64_t half_width = std::llround(static_cast<double>(coaxial_width) / 2.0);

  // - Proceed to the end of all segments in a straight line, stopping at the
  // first turn or at when the bulge end point would fall on the current
  // segment (index k).
  // - Fatten covered segments in the line to at least the width of the bulge.
  // - When the loop terminates, either we are at the end of the line, the next
  // segment turns a corner, or we have to insert a point into the current
  // segment.

  // Just after the first corner turn or at the end of the poly line we have to
  // insert a new point to maintain the bulge shape when the poly line is
  // inflated.

  double d_end = point.L2DistanceTo(segments_[intersection_index].end);
  double d_insertion = half_length;
  double overflow = d_insertion - d_end;

  Point insertion_start = point;

  size_t k = intersection_index;
  while (k < segments_.size() - 1) {
    Line next_line = Line(segments_[k].end, segments_[k + 1].end);

    if (!intersected_line.IsSameInfiniteLine(next_line))
      // Next segment turns a corner, so break.
      break;

    if (overflow <= 0) {
      // Inserted point should fall on the current segment (k), so break.
      break;
    }

    insertion_start = segments_[k].end;
    double next_line_length = next_line.Length();
    d_insertion = overflow;
    overflow -= next_line_length;

    // The current segment (k) is exceeded, so it must be at least as wide as
    // the new bulge. But as a cleaner alternative, we just delete the existing
    // segment. We can do this because the next segment is on the same line and
    // would become the same width.
    if (segments_[k].width <= coaxial_width) {
      if (segments_[k + 1].width <= coaxial_width) {
        segments_.erase(segments_.begin() + k);
        continue;
      }
      segments_[k].width = coaxial_width;
    }

    ++k;
  }

  // TODO(aryap): Close to zero good enough here?
  if (overflow < 0) {
    // The segment to alter is k, but only if it would increase the segment's
    // existing width:
    if (segments_[k].width <= coaxial_width) {
      Point point_after = intersected_line.PointOnLineAtDistance(
          insertion_start, d_insertion);
      segments_.insert(segments_.begin() + k,
                       LineSegment {
                         .end = point_after,
                         .width = coaxial_width,
                       });
    }
  } else if (overflow == 0 || k == segments_.size() - 1) {
    // Effectively at the end of the poly line or only need to change the width
    // of this line, so only need modify the last segment instead of inserting
    // a new one.
    if (segments_[k].width < coaxial_width) {
      segments_[k].end = intersected_line.PointOnLineAtDistance(
          segments_[k].end, overflow);
      segments_[k].width = coaxial_width;
    } else if (overflow > 0) {
      Point point_after =
          intersected_line.PointOnLineAtDistance(segments_[k].end, overflow);
      segments_.push_back(
          LineSegment { .end = point_after, .width = coaxial_width });
    }
  } else {
    // The next segment is around a corner.
    //
    // The intersected segment `segment` will remain and be enlarged to the
    // given width.
    segments_[k].width = coaxial_width;

    // A new segment should created along the next segment we were going to
    // embark on, with the new width. As a special case, we check for
    // segments that in the same line but in a different width:
    LineSegment &next_segment = segments_[k + 1];
    Line next_line = Line(segments_[k].end, next_segment.end);
    Point point_after = next_line.PointOnLineAtDistance(segments_[k].end, half_width);

    // TODO(aryap): Check if next segment width is already too big?
    segments_.insert(
        segments_.begin() + k + 1,
        LineSegment {
          .end = point_after,
          .width = std::max(
              static_cast<uint64_t>(2.0 * overflow), next_segment.width)
    });
  }
}

void PolyLine::InsertBackwardBulgePoint(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line,
    uint64_t intersected_previous_width) {

  const Point &start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;

  double half_length = static_cast<double>(coaxial_length) / 2.0;
  uint64_t half_width = std::llround(static_cast<double>(coaxial_width) / 2.0);

  double d_start = point.L2DistanceTo(start);
  double d_insertion = half_length;
  double overflow = d_insertion - d_start;

  Point insertion_start = point;

  size_t k = intersection_index;
  while (k > 0) {
    Line previous_line = Line(
        k == 1 ? start_ : segments_[k - 2].end,
        segments_[k - 1].end);

    if (!intersected_line.IsSameInfiniteLine(previous_line)) {
      break;
    }

    if (overflow <= 0) {
      break;
    }

    insertion_start = k == 1 ? start_ : segments_[k - 2].end;
    double previous_line_length = previous_line.Length();
    d_insertion = overflow;
    overflow -= previous_line_length;

    --k;

    if (segments_[k].width <= coaxial_width) {
      if (segments_[k + 1].width <= coaxial_width) {
        segments_.erase(segments_.begin() + k);
      } else {
        segments_[k].width = coaxial_width;
      }
    }
  }

  if (overflow < 0) {
    if (segments_[k].width <= coaxial_width) {
      Point point_before = intersected_line.PointOnLineAtDistance(
          point, -d_insertion);
      segments_.insert(segments_.begin() + k,
                       LineSegment {
                         .end = point_before,
                         .width = (k == intersection_index ?
                            intersected_previous_width : segments_[k].width)
                       });
    }
  } else if (overflow == 0) {
    segments_[k].width = std::max(segments_[k].width, coaxial_width);
  } else if (k == 0) {
    start_ = intersected_line.PointOnLineAtDistance(insertion_start, -overflow);
  } else {
    // In this case we have overflow and a corner turn.
    LineSegment &last_segment = segments_[k - 1];
    Point last_line_start = k == 1 ?  start_ : segments_[k - 2].end;
    // The end of the last line is the start of this line. The start of the
    // last line is the end of the line before it.
    Line last_line = Line(last_line_start, last_segment.end);
    Point point_before = last_line.PointOnLineAtDistance(
        last_segment.end, -static_cast<int64_t>(half_width));

    segments_.insert(
        segments_.begin() + k - 1,
        LineSegment {
          .end = point_before,
          .width = std::max(
              static_cast<uint64_t>(2.0 * overflow), last_segment.width)
        });
  }
}

//           _
//           /|
//          o <- want this point after
//         /
//        x
//       /
//      o <- want this point before
//     /
void PolyLine::InsertBulge(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length) {
  size_t intersection_index = 0;
  if (!Intersects(point, &intersection_index)) {
    return;
  }

  LOG(INFO) << Describe();
  LOG(INFO) << "point = " << point << " w x l " << coaxial_width << " x " << coaxial_length;

  const Point &start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;
  Line line = Line(start, segments_[intersection_index].end);

  // InsertForwardBulgePoint might modify this so we save the previous width of
  // the segment at intersection_index.
  uint64_t previous_width = segments_[intersection_index].width;

  InsertForwardBulgePoint(
      point, coaxial_width, coaxial_length, intersection_index, line);
  InsertBackwardBulgePoint(
      point, coaxial_width, coaxial_length, intersection_index, line,
      previous_width);

  LOG(INFO) << Describe();

  EnforceInvariants();
}

void PolyLine::SetWidth(const uint64_t width) {
  for (LineSegment &segment : segments_) {
    segment.width = width;
  }
}

const std::vector<Point> PolyLine::Vertices() const {
  std::vector<Point> points = {start_};
  for (const auto &segment : segments_) {
    points.push_back(segment.end);
  }
  return points;
}

bool PolyLine::Intersects(const Point &point, size_t *segment_index) const {
  Point start = start_;
  int64_t k = -1;
  for (size_t i = 0; i < segments_.size(); ++i) {
    const LineSegment &segment = segments_[i];
    Line line = Line(start, segment.end);
    if (line.IntersectsInBounds(point)) {
      k = i;
      break;
    }
    start = segment.end;
  }
  if (segment_index)
    *segment_index = k;
  return k >= 0;
}

void PolyLine::EnforceInvariants() {
  // Remove duplicate segment end-points, enlarging the remaining to the max of
  // all the segment widths which had the same end-point.
  Point last = start_;
  auto last_segment = segments_.end();
  for (auto it = segments_.begin(); it != segments_.end();) {
    if (it->end == last) {
      if (last_segment != segments_.end()) {
        last_segment->width = std::max(last_segment->width, it->width);
      }
      it = segments_.erase(it);
    } else {
      last = it->end;
      last_segment = it;
      ++it;
    }
  }
}

}  // namespace geometry
}  // namespace bfg
