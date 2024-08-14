#ifndef TILES_LUT_B_H_
#define TILES_LUT_B_H_

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

class LutB : public Tile {
 public:
  // A k-LUT is made up of:
  //  - 2^k memories
  //  - k input buffers
  //  - 1 output buffer
  //  - (2^k / 8 = 2^k / 2^3 =) 2^(k - 3) 8:1 passive muxes
  //  - Enough passive or active muxes to reduce the outputs of the passive
  //  muxes
  //
  // We build the LUT by arranging memories and buffer cells in rows around a
  // central grid of active muxes, since they are large. An easy shorthand for
  // the arrangment is to consider the queue of each part and assign them to
  // rows by index, where rows are indexed from 0 (bottom). We do this once for
  // the bank of rows on the left side and once for the bank of rows on the
  // right.
  struct BankArrangement {
    // constexpr comes to std::vector in C++20 but we don't have it yet.
    std::vector<size_t> memory_rows;
    std::vector<size_t> buffer_rows;
    std::vector<size_t> active_mux2_rows;
  };

  struct LayoutConfig {
    BankArrangement left;
    BankArrangement right;
    int64_t mux_area_padding;
  };

  LutB(DesignDatabase *design_db, size_t lut_size = 4)
      : Tile(design_db),
        lut_size_(lut_size) {}

  bfg::Cell *GenerateIntoDatabase(const std::string &name) override;

 protected:
  size_t lut_size_;

  static const LayoutConfig *GetLayoutConfiguration(size_t lut_size);

  static const std::pair<size_t, LayoutConfig> kLayoutConfigurations[];
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_B_H_
