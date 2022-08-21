#include "poly_line_cell.h"

#include <utility>
#include <cstdint>
#include <vector>
#include <memory>

#include "geometry/point.h"
#include "geometry/rectangle.h"

namespace bfg {

using geometry::Point;
using geometry::PolyLine;

void PolyLineCell::AddPolyLine(const PolyLine &poly_line) {
  poly_lines_.emplace_back(new PolyLine(poly_line));
}

PolyLine *PolyLineCell::AddPolyLine() {
  PolyLine *poly_line = new PolyLine();
  poly_lines_.emplace_back(poly_line);
  return poly_line;
}

const geometry::Rectangle PolyLineCell::GetBoundingBox() const {
  if (poly_lines_.empty()) {
    return std::make_pair(Point(0, 0), Point(0, 0));
  }

  auto &first_box = poly_lines_.front()->GetBoundingBox();
  const Point &lower_left = first_box.lower_left();
  const Point &upper_right = first_box.upper_right();
  int64_t min_x = lower_left.x();
  int64_t min_y = lower_left.y();
  int64_t max_x = upper_right.x();
  int64_t max_y = upper_right.y();

  for (size_t i = 2; i < poly_lines_.size(); ++i) {
    auto &bounds = poly_lines_[i]->GetBoundingBox();
    const Point &lower_left = bounds.lower_left();
    const Point &upper_right = bounds.upper_right();
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  return std::make_pair(Point(min_x, min_y), Point(max_x, max_y));
}

}  // namespace bfg
