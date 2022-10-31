#include "poly_line.h"

#include <algorithm>
#include <cstdint>
#include <glog/logging.h>
#include <vector>

#include "line_segment.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

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
    const Point &point, uint64_t width, uint64_t length) {
  size_t segment_index = 0;
  if (!Intersects(point, &segment_index)) {
    return;
  }

  Point start;
  const LineSegment &segment = segments_[segment_index];
  if (segment_index > 0) {
    start = segments_[segment_index - 1].end;
  } else {
    start = start_;
  }

  Line line = Line(start, segment.end);
  uint64_t previous_width = segment.width;
  double d_end = point.L2DistanceTo(segment.end);
  double d_start = point.L2DistanceTo(start);

  if (point == segment.end) {
    // TODO(aryap): Special treatment if the intersection was at the end of the
    // LineSegment.
    return;
    // TODO(aryap): There is also the case where we are within `length` of the
    // end of the segment.
  }

  //           /
  //          o <- want this point after
  //         /
  //        x
  //       /
  //      o <- want this point before
  //     /

  Point point_before = line.PointOnLineAtDistance(point, -length);
  Point point_after = line.PointOnLineAtDistance(point, length);
  LineSegment bulge_segment = {
    .end = point_after,
    .width = width,
    .growth_anchor = AnchorPosition::kCenterAutomatic
  };

  segments_.insert(segments_.begin() + segment_index, bulge_segment);
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
