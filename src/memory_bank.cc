#include "memory_bank.h"

#include "geometry/point.h"
#include "layout.h"

namespace bfg {

void MemoryBank::MoveTo(const geometry::Point &point) {
  // Treat the lower_left_ point of the first row as the origin of the
  // MemoryBank itself. Moving the whole bank moves this point to the new point,
  // and so the relative position of all other rows must be preserved.
  if (rows_.empty()) {
    return;
  }
  geometry::Point origin = rows_.front().lower_left();
  for (auto &row : rows_) {
    geometry::Point difference = row.lower_left() - origin;
    row.set_lower_left(point + difference);
  }
  layout_->MoveTo(point);
}

}  // namespace bfg
