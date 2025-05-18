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
  geometry::Point first_row_origin = Origin();
  for (auto &row : rows_) {
    geometry::Point difference = row.origin() - first_row_origin;
    row.MoveTo(point + difference);
  }
}

geometry::Point MemoryBank::Origin() const {
  if (rows_.empty()) {
    return {0, 0};
  }
  return rows_.front().origin();
}

void MemoryBank::AlignPointTo(
    const geometry::Point &reference, const geometry::Point &target) {
  geometry::Point diff = target - reference;
  geometry::Point first_row_origin = Origin();
  MoveTo(first_row_origin + diff);
}

bool MemoryBank::RowIsRotated(size_t index) {
  return rotate_first_row_ ? index % 2 == 0 : index % 2 != 0;
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

    // There is also a corresponding vector of instances for each row.
    instances_.emplace_back();
    instance_names_.emplace_back();

    if (rotate_alternate_rows_) {
      row.set_rotate_instances(RowIsRotated(i));
    }
    if (tap_cell_) {
      row.set_tap_cell(*tap_cell_);
    }
  }
  return rows_[index];
}

// Makes sure the y position of each row's origin sits on top (or below, if
// grow_down_ is true) the row below (or above). This is necessary because when
// empty rows are created they have unknown height, but as soon as an instance
// is assigned to those rows they gain a height and rows above them need to be
// shifted up.
void MemoryBank::EnsureVerticalAbutment() {
  int64_t last_y = Origin().y();
  int64_t last_row_height = 0;
  for (size_t i = 0; i < rows_.size(); ++i) {
    RowGuide &row = Row(i);

    int64_t expected_y = grow_down_ ?
        last_y - row.Height() : last_y + last_row_height;

    row.MoveTo({row.origin().x(), expected_y});

    last_y = row.LowerLeft().y();
    last_row_height = row.Height();
  }
}

// Aligns the origins of all rows such that the x position of each row's origin
// aligns with the previous row's left or right-most x position, based on
// horizontal_alignment_.
void MemoryBank::AlignRight() {
  int64_t right_x = 0;
  for (size_t i = 0; i < rows_.size(); ++i) {
    right_x = std::max(right_x, rows_[i].LowerRight().x());
  }
  for (size_t i = 0; i < rows_.size(); ++i) {
    RowGuide &row = rows_[i];
    int64_t current_y = row.LowerRight().y();
    row.MoveLowerRight({right_x, current_y});
  }
}
void MemoryBank::AlignLeft() {
  int64_t left_x = 0;
  for (size_t i = 0; i < rows_.size(); ++i) {
    left_x = std::min(left_x, rows_[i].LowerLeft().x());
  }
  for (size_t i = 0; i < rows_.size(); ++i) {
    RowGuide &row = rows_[i];
    int64_t current_y = row.LowerLeft().y();
    row.MoveLowerLeft({left_x, current_y});
  }
}

void MemoryBank::FixAlignments() {
  if (rows_.empty()) {
    return;
  }
  EnsureVerticalAbutment();

  if (horizontal_alignment_) {
    switch (*horizontal_alignment_) {
      case geometry::Compass::LEFT:
        AlignLeft();
        break;
      case geometry::Compass::RIGHT:
        AlignRight();
        break;
      default:
        LOG(FATAL) << "Unsupported horizontal_alignment in MemoryBank: "
                   << *horizontal_alignment_;
    }
  }
}

geometry::Instance *MemoryBank::InstantiateInside(size_t row_index,
                                                  const std::string &name,
                                                  Layout *template_layout) {
  if (!horizontal_alignment_) {
    return InstantiateRight(row_index, name, template_layout);
  }
  switch (*horizontal_alignment_) {
    case geometry::Compass::LEFT:
      return InstantiateRight(row_index, name, template_layout);
      break;
    case geometry::Compass::RIGHT:
      return InstantiateLeft(row_index, name, template_layout);
      break;
    default:
      LOG(FATAL) << "Unsupported horizontal_alignment in MemoryBank: "
                 << *horizontal_alignment_;
  }
  return nullptr;
}

geometry::Instance *MemoryBank::InstantiateLeft(size_t row_index,
                                                const std::string &name,
                                                Layout *template_layout) {
  RowGuide &row = Row(row_index);
  std::vector<geometry::Instance*> &instances = instances_[row_index];
  std::vector<std::string> &instance_names = instance_names_[row_index];

  geometry::Instance *installed = nullptr;
  installed = row.InstantiateAndInsertFront(name, template_layout);
  instances.push_back(installed);
  instance_names.push_back(name);
  FixAlignments();
  return installed;
}

geometry::Instance *MemoryBank::InstantiateRight(size_t row_index,
                                                 const std::string &name,
                                                 Layout *template_layout) {
  RowGuide &row = Row(row_index);
  std::vector<geometry::Instance*> &instances = instances_[row_index];
  std::vector<std::string> &instance_names = instance_names_[row_index];

  geometry::Instance *installed = nullptr;
  installed = row.InstantiateBack(name, template_layout);
  instances.push_back(installed);
  instance_names.push_back(name);
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
