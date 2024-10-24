#ifndef TILES_LUT_H_
#define TILES_LUT_H_

#include <utility>

#include "tile.h"

#include "../design_database.h"
#include "../row_guide.h"

namespace bfg {

class Cell;
class Layout;
class Circuit;

namespace geometry {

class Instance;

}  // namespace geometry

namespace tiles {

class Lut : public Tile {
 public:
  struct LayoutConfig {
    size_t num_banks;
    size_t bank_rows;
    size_t bank_columns;
    size_t mux_area_rows;
    size_t mux_area_columns;
    int64_t mux_area_padding;
    bool rotate_first_row;
  };

  Lut(DesignDatabase *design_db, size_t lut_size = 4)
      : Tile(design_db),
        lut_size_(lut_size) {}

  bfg::Cell *GenerateIntoDatabase(const std::string &name) override;

 protected:
  size_t lut_size_;

  static const LayoutConfig *GetLayoutConfiguration(size_t lut_size);

  // TODO(aryap): Figure out how to express mux arrangement statically.
  static constexpr std::pair<size_t, LayoutConfig> kLayoutConfigurations[] = {
      {4, LayoutConfig {
          .num_banks = 2,
          .bank_rows = 4,
          .bank_columns = 2,
          .mux_area_rows = 2,
          .mux_area_columns = 2,
          .mux_area_padding = 2500,
          .rotate_first_row = true
      }},
      {5, LayoutConfig {
          .num_banks = 2,
          .bank_rows = 8,
          .bank_columns = 2,
          .mux_area_rows = 4,
          .mux_area_columns = 2,
          .mux_area_padding = 2500,
          .rotate_first_row = true
      }},
      {6, LayoutConfig {
          .num_banks = 2,
          .bank_rows = 8,
          .bank_columns = 4,
          .mux_area_rows = 4,
          .mux_area_columns = 4,
          .mux_area_padding = 2500,
          .rotate_first_row = true
      }}
  };
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_H_
