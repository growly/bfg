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
  ss << start_;
  for (const auto &segment : segments_) {
    ss << " |" << segment.width << "| " << segment.end;
  }
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
  int64_t last_x = segments_.empty() ? start_.x() : segments_.back().end.x();
  int64_t last_y = segments_.empty() ? start_.y() : segments_.back().end.y();
  if (to.x() != last_x && to.y() != last_y) {
    LOG(FATAL) << "PolyLine segments must be rectilinear. Make sure the new "
               << "x == last_x or y == last_y.";
  }
  segments_.push_back(LineSegment{to, width});
}

void PolyLine::InsertBulge(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length) {
  size_t segment_index = 0;
  if (!Intersects(point, &segment_index)) {
    return;
  }

  LineSegment &segment = segments_[segment_index];

  double half_length = static_cast<double>(coaxial_length) / 2.0;
  uint64_t half_width = std::llround(static_cast<double>(coaxial_width) / 2.0);
  uint64_t previous_width = segment.width;

  const Point &start =
      segment_index == 0 ? start_ : segments_[segment_index - 1].end;
  Line line = Line(start, segment.end);

  //           _
  //           /|
  //          o <- want this point after
  //         /
  //        x
  //       /
  //      o <- want this point before
  //     /

  double d_start = point.L2DistanceTo(start);
  double d_end = point.L2DistanceTo(segment.end);

  LOG(INFO) << "d_end: " << d_end << ", half_length: " << half_length;
  if (d_end < half_length) {
    double overflow = half_length - d_end;
    if (segment_index == segments_.size() - 1) {
      // Effectively at the end of the poly line, so only need modify the last
      // segment instead of inserting a new one.
      segment.end =
          line.PointOnLineAtDistance(segment.end, overflow);
      segment.width = coaxial_width;
    } else {
      // The intersected segment `segment` will remain and be enlarged to the
      // given width.
      segment.width = coaxial_width;

      // A new segment should created along the next segment we were going to
      // embark on with the new width:
      LineSegment &next_segment = segments_[segment_index + 1];
      Line next_line = Line(segment.end, next_segment.end);
      Point point_after = next_line.PointOnLineAtDistance(segment.end, half_width);
      segments_.insert(
          segments_.begin() + segment_index + 1,
          LineSegment {
            .end = point_after,
            .width = std::max(half_width, next_segment.width)
          });

    }
  } else {
    Point point_after = line.PointOnLineAtDistance(point, half_length);
    segments_.insert(segments_.begin() + segment_index,
                     LineSegment {
                       .end = point_after,
                       .width = coaxial_width,
                     });
  }

  // If the intersection is with the first segment we have to check the
  // start, otherwise, we don't, since the start of the next segment is equal
  // to the end of the last.
  //
  // If the bulge is going to overlap the start point, we have to push the
  // start point back by the difference in lengths.
  if (segment_index == 0 && d_start < half_length) {
    double overflow = half_length - d_start;
    // Effectively at the start, so instead of inserting a new segment before
    // modifying the start point:
    start_ = line.PointOnLineAtDistance(start_, -overflow);
  } else {
    Point point_before = line.PointOnLineAtDistance(point, -half_length);
    segments_.insert(segments_.begin() + segment_index,
                     LineSegment {
                       .end = point_before,
                       .width = previous_width
                     });
  }
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

}  // namespace geometry
}  // namespace bfg
