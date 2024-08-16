#include "checkerboard_guide.h"

#include <optional>
#include <algorithm>
#include <absl/cleanup/cleanup.h>

#include "layout.h"
#include "circuit.h"
#include "design_database.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"

namespace bfg {

CheckerboardGuide::CheckerboardGuide(
      const geometry::Point &origin,
      const std::string &name_prefix,
      size_t num_rows,
      size_t num_columns,
      bfg::Layout *layout,
      bfg::Circuit *circuit,
      DesignDatabase *design_db)
      : origin_(origin),
        layout_(layout),
        circuit_(circuit),
        num_rows_(num_rows),
        num_columns_(num_columns),
        vertical_overlap_(0),
        horizontal_overlap_(0),
        flip_horizontal_(false),
        name_prefix_(name_prefix),
        template_cells_(nullptr),
        instance_count_(0) {}

const std::vector<geometry::Instance*> &CheckerboardGuide::InstantiateAll() {
  LOG_IF(FATAL, !template_cells_) << "You must set_template_cells() first!";
  size_t column = 0;

  // Do one pass to see what the maximum widths per column are:
  std::vector<uint64_t> max_column_widths =
      std::vector<uint64_t>(num_columns_, 0);
  size_t k = 0;
  for (size_t i = 0; i < num_rows_; ++i) {
    for (size_t j = 0; j < num_columns_; ++j) {
      bfg::Cell *cell = (*template_cells_)[k % template_cells_->size()];
      const geometry::Rectangle cell_bb = cell->layout()->GetBoundingBox();
      max_column_widths[j] = std::max(max_column_widths[j], cell_bb.Width());
      ++k;
    }
  }

  layout_instances_.clear();
  int64_t y_pos = origin_.y();
  bool flip = flip_horizontal_;
  k = 0;
  for (size_t i = 0; i < num_rows_; ++i) {
    uint64_t max_row_height = 0;
    int64_t x_pos = origin_.x();

    for (size_t j = 0; j < num_columns_; ++j) {
      absl::Cleanup increment_x_pos = [&]() {
        // You can put this in the post-iteration expression for the loop but
        // then you have to rely on the order of evaluation to get the right j,
        // which seems like a gotcha. This is cleaner:
        x_pos += max_column_widths[j] - horizontal_overlap_;
      };
      // Check if this cell gets an instance.
      if (((i * num_columns_ + j) % 2 == 0) == flip) {
        LOG(INFO) << "Checkerboard (" << i << ", " << j << ") cell empty";
        continue;
      }

      bfg::Cell *cell = (*template_cells_)[k % template_cells_->size()];
      const geometry::Rectangle cell_bb = cell->layout()->GetBoundingBox();

      std::string name = absl::StrCat(name_prefix_, "_", k);

      // Add instance to circuit.
      if (circuit_) {
        circuit_->AddInstance(name, cell->circuit());
      }

      // Add instance to layout.
      geometry::Point position = {x_pos, y_pos};
      geometry::Instance layout_instance_template(cell->layout(), position);
      layout_instance_template.set_name(name);
      geometry::Instance *layout_instance =
          layout_->AddInstance(layout_instance_template);
      layout_instances_.push_back(layout_instance);
      LOG(INFO) << "Put " << *layout_instance << " at " << position;

      max_row_height = std::max(max_row_height, cell_bb.Height());

      ++k;
    }
    y_pos += max_row_height - vertical_overlap_;
    flip = !flip;
  }
  instance_count_ = k;
  return layout_instances_;
}

std::optional<geometry::Rectangle> CheckerboardGuide::GetBoundingBox() const {
  // TODO(aryap): This code is duplicated enough that it could be simplified and
  // consolidated. The basic task is, given a vector of things that can have
  // GetBoundingBox() called on them, to create the rectangle that covers them
  // all.
  std::optional<geometry::Rectangle> bounding_box;
  for (geometry::Instance *instance : layout_instances_) {
    auto instance_box = instance->GetBoundingBox();
    if (!bounding_box) {
      bounding_box = instance_box;
      continue;
    }
    bounding_box->ExpandToCover(instance_box);
  }
  return bounding_box;
}

}   // namespace bfg
