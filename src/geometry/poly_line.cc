#include "poly_line.h"

#include <algorithm>
#include <cstdint>
#include <glog/logging.h>
#include <vector>

#include "line_segment.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

void PolyLine::FlipHorizontal() {
  start_.set_x(-start_.x());
  for (LineSegment &segment : segments_) {
    segment.end.set_x(-segment.end.x());
  }
}

void PolyLine::FlipVertical() {
  start_.set_y(-start_.y());
  for (LineSegment &segment : segments_) {
    segment.end.set_y(-segment.end.y());
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

  return Rectangle(Point(min_x, min_y), Point(max_x, max_y));
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

}  // namespace geometry
}  // namespace bfg
