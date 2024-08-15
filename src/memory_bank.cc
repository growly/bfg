#include "memory_bank.h"

#include <string>
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

RowGuide &MemoryBank::Row(size_t index) {
  if (index < rows_.size()) {
    return rows_[index];
  }

  int64_t y_pos = 0;
  if (!rows_.empty()) {
    y_pos = grow_down_ ?
        rows_.front().LowerLeft().y() : rows_.back().UpperLeft().y();
  }
  
  // We now need to insert rows up to index so that rows are contiguous:
  for (size_t i = rows_.size(); i <= index; ++i) {
    RowGuide &row = rows_.emplace_back(
        geometry::Point {0, y_pos},   // Row lower-left point.
        layout_,
        nullptr,                      // FIXME
        design_db_);
    if (rotate_alternate_rows_) {
      bool rotate_this_row = rotate_first_row_ ?  i % 2 == 0 : i % 2 != 0;
      row.set_rotate_instances(rotate_this_row);
    }
    if (tap_cell_) {
      row.set_tap_cell(*tap_cell_);
    }
  }
  return rows_[index];
}

// Aligns the origins of all rows such that:
// - The y position of each row's origin sits on top (or below, if grow_down_
// is true) the row below (or above).
// - The x position of each row's origin aligns with the previous row's left or
// right-most x position, based on horizontal_alignment_.
void MemoryBank::FixAlignments() {
  // Make sure rows abut each other vertically at the tiling boundary:
  int64_t last_y = 0;
  int64_t last_x = 0;
  int64_t last_row_height = 0;
  for (size_t i = 0; i < rows_.size(); ++i) {
    RowGuide &row = Row(i);

    int64_t expected_y = grow_down_ ?
        last_y - row.Height() : last_y + last_row_height;

    if (!horizontal_alignment_) {
      row.MoveTo({row.origin().x(), expected_y});
    } else {
      switch (*horizontal_alignment_) {
        case geometry::Compass::LEFT:
          row.MoveLowerLeft({last_x, expected_y});
          break;
        case geometry::Compass::RIGHT:
          row.MoveLowerRight({last_x, expected_y});
          break;
        default:
          LOG(FATAL) << "Unsupported horizontal_alignment in MemoryBank: "
                     << *horizontal_alignment_;
      }
    }

    last_y = row.LowerLeft().y();
    last_row_height = row.Height();

    if (horizontal_alignment_) {
      switch (*horizontal_alignment_) {
        case geometry::Compass::LEFT:
          last_x = row.LowerLeft().x();
          break;
        case geometry::Compass::RIGHT:
          last_x = row.UpperRight().x();
          break;
        default:
          LOG(FATAL) << "Unsupported horizontal_alignment in MemoryBank: "
                     << *horizontal_alignment_;
      }
    }
  }
}

geometry::Instance *MemoryBank::InstantiateRight(size_t row_index,
                                                 const std::string &name,
                                                 Layout *template_layout) {
  RowGuide &row = Row(row_index);
  geometry::Instance *installed = nullptr;
  if (row.rotate_instances()) {
    installed = row.InstantiateAndInsertFront(name, template_layout);
  } else {
    installed = row.InstantiateBack(name, template_layout);
  }
  // FIXME: Put installed in memories_.
  FixAlignments();
  return installed;
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
