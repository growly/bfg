#include "poly_line.h"

#include <algorithm>
#include <cstdint>
#include <glog/logging.h>
#include <vector>

namespace bfg {
namespace geometry {

const std::pair<Point, Point> PolyLine::GetBoundingBox() const {
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

  return std::make_pair(Point(min_x, min_y), Point(max_x, max_y));
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

}  // namespace geometry
}  // namespace bfg
