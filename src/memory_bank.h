#ifndef MEMORY_BANK_H_
#define MEMORY_BANK_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "design_database.h"
#include "geometry/compass.h"
#include "geometry/point.h"
#include "geometry/instance.h"
#include "cell.h"
#include "layout.h"
#include "row_guide.h"

namespace bfg {

class MemoryBank {
 public:
  MemoryBank()
      : MemoryBank(nullptr, nullptr, nullptr, true, false, std::nullopt) {}

  MemoryBank(Layout *layout,
             DesignDatabase *design_db,
             Cell *tap_cell,
             bool rotate_alternate_rows,
             bool rotate_first_row,
             std::optional<geometry::Compass> horizontal_alignment)
      : layout_(layout),
        design_db_(design_db),
        tap_cell_(tap_cell),
        grow_down_(false),
        rotate_alternate_rows_(rotate_alternate_rows),
        rotate_first_row_(rotate_first_row),
        horizontal_alignment_(horizontal_alignment) {}

  // This only moves existing rows. Rows created after this call are not moved.
  // TODO(aryap): We could fix that behaviour with an "origin_" property that
  // controls where new Rows are created in Row(size_t index).
  void MoveTo(const geometry::Point &point);

  // Translate the Layout such that the given reference point, assumed to be in
  // the coordinate space of this Layout, ends up at the target point.
  void AlignPointTo(
      const geometry::Point &reference, const geometry::Point &target);

  geometry::Point Origin() const;

  RowGuide &Row(size_t index);

  bool RowIsRotated(size_t index);

  // Instantiates on the opposite side to which the bank is aligned:
  geometry::Instance *InstantiateInside(size_t row_index,
                                        const std::string &name,
                                        Layout *template_layout);
  geometry::Instance *InstantiateRight(size_t row_index,
                                       const std::string &name,
                                       Layout *template_layout);
  geometry::Instance *InstantiateLeft(size_t row_index,
                                      const std::string &name,
                                      Layout *template_layout);

  std::optional<geometry::Rectangle> GetBoundingBox() const;
  std::optional<geometry::Rectangle> GetTilingBounds() const;

  void DisableTapInsertionOnRow(size_t index);
  void EnableTapInsertionOnRow(size_t index);
  // Uses default tap_cell_.
  void EnableTapInsertionOnRow(size_t index, Cell *tap_cell);

  std::vector<std::vector<std::string>> &instance_names() {
    return instance_names_;
  }
  std::vector<std::vector<geometry::Instance*>> &instances() {
    return instances_;
  }
  std::vector<RowGuide> &rows() { return rows_; }

  const std::vector<std::vector<std::string>> &instance_names() const {
    return instance_names_;
  }
  const std::vector<std::vector<geometry::Instance*>> &instances() const {
    return instances_;
  }
  const std::vector<RowGuide> &rows() const { return rows_; }

 private:
  void FixAlignments();
  void EnsureVerticalAbutment();
  void AlignRight();
  void AlignLeft();

  std::optional<geometry::Rectangle> CoveringBoxOverAllRows(
      const std::function<std::optional<geometry::Rectangle>(const RowGuide&)>
          &box_getter_fn) const;

  void set_grow_down(bool grow_down) { grow_down_ = grow_down; }
  void set_rotate_alternate_rows(bool rotate_alternate_rows) {
    rotate_alternate_rows_ = rotate_alternate_rows;
  }
  void set_rotate_first_row(bool rotate_first_row) {
    rotate_first_row_ = rotate_first_row;
  }

  // It turns out it is awfully convenient to be able to use this structure as a
  // grouping for objects that belong to some external layout.
  bfg::Layout *layout_;

  // Memory instance names by row and column. Major index is row, minor index
  // is column.
  std::vector<std::vector<std::string>> instance_names_;

  // Instances per row. Major index is row, minor index is column. These are
  // pointers to Instances in the main layout, not the bank-specific temporary
  // layout below.
  std::vector<std::vector<geometry::Instance*>> instances_;

  std::vector<RowGuide> rows_;

  Cell *tap_cell_;
  DesignDatabase *design_db_;

  // The MemoryBank will create rows if they don't exist. If `grow_down_` is
  // true new rows will be added geometrically below the bottom-most, otherwise
  // they will be added above the top-most.
  //
  // That is, if grow_down_ is true, then higher index rows will be _below_
  // lower index rows.
  bool grow_down_;

  bool rotate_alternate_rows_;
  bool rotate_first_row_;

  std::optional<geometry::Compass> horizontal_alignment_;
};

}  // namespace bfg

#endif  // MEMORY_BANK_H_
