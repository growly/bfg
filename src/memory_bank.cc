#include "memory_bank.h"

#include <optional>

#include "geometry/rectangle.h"
#include "geometry/point.h"
#include "layout.h"

namespace bfg {

void MemoryBank::MoveTo(const geometry::Point &point) {
  // Treat the origin point of the first row as the origin of the
  // MemoryBank itself. Moving the whole bank moves this point to the new point,
  // and so the relative position of all other rows must be preserved.
  if (rows_.empty()) {
    return;
  }
  geometry::Point first_row_origin = rows_.front().origin();
  for (auto &row : rows_) {
    geometry::Point difference = row.origin() - first_row_origin;
    row.MoveTo(point + difference);
  }
}

std::optional<geometry::Rectangle> MemoryBank::GetBoundingBox() const {
  std::optional<geometry::Rectangle> bounding_box;
  for (const auto &row : rows_) {
    auto row_box = row.GetBoundingBox();
    if (!row_box) {
      continue;
    }
    if (!bounding_box) {
      bounding_box = row_box;
      continue;
    }
    bounding_box->ExpandToCover(*row_box);
  }
  return bounding_box;
}

}  // namespace bfg
