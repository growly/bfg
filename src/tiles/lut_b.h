#ifndef TILES_LUT_B_H_
#define TILES_LUT_B_H_

#include <utility>

#include "tile.h"

#include "../geometry/compass.h"
#include "../design_database.h"
#include "../row_guide.h"
#include "../memory_bank.h"
#include "../layout.h"

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
  //
  // TODO(aryap): Imagine a shorthand that was like this:
  //  bank:
  //    row: memory2, memory3, buf1, buf2
  //    row: memory1, buf0
  //    row: memory0
  // Would that be good?
  struct BankArrangement {
    // constexpr comes to std::vector in C++20 but we don't have it yet.
    std::vector<size_t> memory_rows;
    std::vector<size_t> buffer_rows;
    std::vector<size_t> active_mux2_rows;
    std::vector<size_t> clk_buf_rows;
    geometry::Compass horizontal_alignment;
  };

  struct LayoutConfig {
    BankArrangement left;
    BankArrangement right;
    int64_t mux_area_padding;
    size_t mux_area_rows;
    size_t mux_area_columns;
  };

  LutB(DesignDatabase *design_db, size_t lut_size = 4)
      : Tile(design_db),
        lut_size_(lut_size) {}

  bfg::Cell *GenerateIntoDatabase(const std::string &name) override;

 protected:
  void Route(Layout *layout) const;
  void AddClockAndPowerStraps(Layout *layout) const;

  // TODO(aryap): This feels like a nice general feature of the Layout class.
  void AddVerticalSpineWithFingers(
      const std::string &spine_layer_name,
      const std::string &via_layer_name,
      const std::string &finger_layer_name,
      const std::string &net,
      const std::vector<geometry::Point> &connections,
      int64_t spine_x,
      Layout *layout) const;

  size_t lut_size_;

  static const LayoutConfig *GetLayoutConfiguration(size_t lut_size);

  static const std::pair<size_t, LayoutConfig> kLayoutConfigurations[];

  std::vector<MemoryBank> banks_;
  std::vector<geometry::Instance*> buf_order_;
  std::vector<geometry::Instance*> mux_order_;
  std::vector<geometry::Instance*> active_mux2s_;
  std::vector<geometry::Instance*> clk_buf_order_;
  std::vector<geometry::Instance*> memories_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_B_H_
