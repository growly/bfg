#ifndef MEMORY_BANK_H_
#define MEMORY_BANK_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "geometry/point.h"
#include "layout.h"
#include "row_guide.h"

namespace bfg {

class MemoryBank {
 public:
  MemoryBank()
      : grow_down_(false) {}

  void MoveTo(const geometry::Point &point);

  RowGuide &GetRow(size_t index);

  std::vector<std::vector<std::string>> &memory_names() {
    return memory_names_;
  }
  std::vector<std::vector<geometry::Instance*>> &memories() {
    return memories_;
  }
  std::vector<RowGuide> &rows() { return rows_; }

  std::optional<geometry::Rectangle> GetBoundingBox() const;

  void set_grow_down(bool grow_down) { grow_down_ = grow_down; }

 private:
  // It turns out it is awfully convenient to be able to use this structure as a
  // grouping for objects that belong to some external layout.
  bfg::Layout *layout_;

  // Memory instance names by row and column. Major index is row, minor index
  // is column.
  std::vector<std::vector<std::string>> memory_names_;

  // Instances per row. Major index is row, minor index is column. These are
  // pointers to Instances in the main layout, not the bank-specific temporary
  // layout below.
  std::vector<std::vector<geometry::Instance*>> memories_;

  std::vector<RowGuide> rows_;

  // The MemoryBank will create rows if they don't exist. If `grow_down_` is
  // true new rows will be added geometrically below the bottom-most, otherwise
  // they will be added above the top-most.
  bool grow_down_;
};

}  // namespace bfg

#endif  // MEMORY_BANK_H_
