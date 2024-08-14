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

RowGuide &MemoryBank::GetRow(size_t index) {
  if (index < rows_.size()) {
    return rows_[index];
  }

  int64_t y_pos = 0;
  if (!rows_.empty()) {
    y_pos = 
  }
  
  for (size_t i = rows_.size(); i <= index; ++i) {
    rows_.push_back(
        RowGuide({0, y_pos},     // Row lower-left point.
                 layout.get(),
                 nullptr,        // FIXME
                 design_db_);
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
