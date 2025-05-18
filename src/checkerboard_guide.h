#ifndef CHECKERBOARD_GUIDE_H_
#define CHECKERBOARD_GUIDE_H_

#include <string>

#include "layout.h"
#include "circuit.h"
#include "design_database.h"
#include "geometry/point.h"

namespace bfg {

// This guide automates placement of instances of cells as follows:
//                      <-------------- num_columns_ ------------>
//
//                                    ->  horizontal overlap
//                      +------------+--+----------+--+----------+  ^
//                      |            |  |          |  |          |  |
//                      |  INST 3    |  | BLANK    |  | INST 4   |  |
//                      |   (0, 2)   |  |  (1, 2)  |  |  (2, 2)  |  |
//                      +------------+--+----------+--+----------+  |
//                      +------------+--+----------+--+----------+  |
//                      |            |  |          |  |          |  |
//                      |  BLANK     |  | INST 2   |  | BLANK    |  | num_rows_
//                      |   (0, 1)   |  |  (1, 1)  |  |  (2, 1)  |  |
//                    | +------------+--+----------+--+----------+  |
//   vertical overlap v +------------+--+----------+--+----------+  |
//                      |            |  |          |  |          |  |
//                      |  INST 0    |  | BLANK    |  | INST 1   |  |
//                      |   (0, 0)   |  |  (1, 0)  |  |  (2, 0)  |  |
//            origin, O +------------+--+----------+--+----------+  v
//
// NOTE(aryap): The algorithm originally used for this allowed the grid to grow
// in one dimension, namely in the number of rows, by fixing the number of
// instances to be made and the number of columns. This could be the basis of a
// non-fixed CheckerboardGrid:
//  {
//    size_t column_select = 0;
//    int64_t column_spacing = 300;
//    int64_t row_spacing = 750;
//  
//    // Height of the first mux we put down:
//    int64_t mux_height = base_mux_cell->layout()->GetBoundingBox().Height();
//  
//    int64_t x_pos = static_cast<int64_t>(
//        left_bounds.Width()) + layout_config.mux_area_padding;
//    int64_t y_offset = -(mux_height - 2 * max_row_height) / 2;
//    int64_t y_pos = y_offset;
//  
//    size_t p = 0;
//    for (size_t i = 0; i < num_muxes; ++i) {
//      bfg::Cell *mux_cell = p % 2 == 0 ? base_mux_cell : alt_mux_cell;
//      mux_height = mux_cell->layout()->GetBoundingBox().Height();
//      int64_t mux_width = mux_cell->layout()->GetBoundingBox().Width();
//      int64_t effective_mux_height = mux_height + row_spacing;
//      int64_t effective_mux_width = mux_width + column_spacing;
//  
//      std::string mux_name = absl::StrCat("mux_", i);
//      circuit->AddInstance(mux_name, mux_cell->circuit());
//      int64_t x_shift = column_select * effective_mux_width;
//      geometry::Instance geo_instance(
//          mux_cell->layout(), geometry::Point {
//              x_pos + static_cast<int64_t>(column_select * effective_mux_width),
//              y_pos
//          });
//      geo_instance.set_name(mux_name);
//      geometry::Instance *instance = layout->AddInstance(geo_instance);
//      mux_order.push_back(instance);
//      if (p < layout_config.mux_area_rows - 1) {
//        y_pos += effective_mux_height;
//        ++p;
//      } else {
//        column_select += 1;   // Alternate the alternation.
//        x_pos += 2 * (effective_mux_width);
//        y_pos = y_offset;
//        p = 0;
//      }
//  
//      // Alternates between 0 and 1.
//      column_select = (column_select + 1) % 2;
//    }
//  }
// TODO(aryap): Don't leave whole code chunks in comments ^.

class CheckerboardGuide {
 public:
  CheckerboardGuide(
      const geometry::Point &origin,
      const std::string &name_prefix,
      size_t num_rows,
      size_t num_columns,
      bfg::Layout *layout,
      bfg::Circuit *circuit,
      DesignDatabase *design_db);

  void set_vertical_overlap(const int64_t &vertical_overlap) {
    vertical_overlap_ = vertical_overlap;
  }
  void set_horizontal_overlap(const int64_t &horizontal_overlap) {
    horizontal_overlap_ = horizontal_overlap;
  }
  void set_flip_horizontal(bool flip_horizontal) {
    flip_horizontal_ = flip_horizontal;
  }
  void set_template_cells(std::vector<bfg::Cell*> *template_cells) {
    template_cells_ = template_cells;
  }

  const std::vector<geometry::Instance*> &InstantiateAll();
  std::optional<geometry::Rectangle> GetBoundingBox() const;

 private:
  bfg::Layout *layout_;
  bfg::Circuit *circuit_;

  geometry::Point origin_;
  size_t num_rows_;
  size_t num_columns_;
  int64_t vertical_overlap_;
  int64_t horizontal_overlap_;
  bool flip_horizontal_;
  std::string name_prefix_;

  std::vector<bfg::Cell*> *template_cells_;
  std::vector<geometry::Instance*> layout_instances_;

  size_t instance_count_;
};

std::ostream &operator<<(std::ostream &os, const CheckerboardGuide &guide);

}  // namespace bfg

#endif  // CHECKERBOARD_GUIDE_H_
