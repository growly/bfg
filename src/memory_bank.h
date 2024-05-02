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
  void MoveTo(const geometry::Point &point);

  std::vector<std::vector<std::string>> &memory_names() {
    return memory_names_;
  }
  std::vector<std::vector<geometry::Instance*>> &memories() {
    return memories_;
  }
  std::vector<RowGuide> &rows() { return rows_; }

  std::optional<geometry::Rectangle> GetBoundingBox() const;

 private:
  // It turns out it is awfully convenient to be able to use this structure as a
  // grouping of other objects that belong to some external layout.
  bfg::Layout *layout_;

  // Memory instance names by row and column. Major index is row, minor index
  // is column.
  std::vector<std::vector<std::string>> memory_names_;

  // Instances per row. Major index is row, minor index is column. These are
  // pointers to Instances in the main layout, not the bank-specific temporary
  // layout below.
  std::vector<std::vector<geometry::Instance*>> memories_;

  std::vector<RowGuide> rows_;
};

}  // namespace bfg

#endif  // MEMORY_BANK_H_
