#ifndef ROW_GUIDE_H_
#define ROW_GUIDE_H_

#include <ostream>
#include <optional>

#include "vlsir/layout/raw.pb.h"

#include "geometry/instance.h"
#include "geometry/point.h"

namespace bfg {

class Cell;
class Layout;
class Circuit;
class DesignDatabase;

// A helper structure for managing the placement of geometry::Instances in
// something like a row, but without the requirement that all instances have the
// same height. Provides helpers for successive placement in the forward and
// reverse (rotated) directions and automatically inserting tap cells, as would
// be necessary for a row of standard cells.
class RowGuide {
 public:
  RowGuide(
      const geometry::Point &lower_left,
      bfg::Layout *layout,
      bfg::Circuit *circuit,
      DesignDatabase *design_db)
      : lower_left_(lower_left),
        layout_(layout),
        circuit_(circuit),
        design_db(design_db) {}

  // Caller maintains ownership of instance. Instance's position and
  // orientation may be changed. It is assumed that the given instance is
  // already installed in a bfg::Layout (and its circuit representation exists
  // in a bfg::Circuit).
  void PushBack(geometry::Instance *instance);
  void PushFront(geometry::Instance *instance);

  geometry::Instance *InstantiateBack(
      const std::string &name, Layout *template_layout);
  geometry::Instance *InstantiateFront(
      const std::string &name, Layout *template_layout);

  // Assign positions & orientations of instances in the stack according to
  // configuration. Tap cells, if enabled and needed, are inserted into the
  // layout automatically.
  void Place();

  uint64_t Width() const;
  uint64_t Height() const;

  geometry::Point UpperRight() const;
  geometry::Point LowerRight() const;
  geometry::Point UpperLeft() const;
  geometry::Point LowerLeft() const;

  const geometry::Point &lower_left() const { return lower_left_; }

  void set_tap_cell(const bfg::Cell &tap_cell) {
    tap_cell_ = tap_cell;
  }

  void set_rotate_instances(bool rotate_instances) {
    rotate_instances_ = rotate_instances;
  }

  bool rotate_instances() const {
    return rotate_instances_;
  }

  int32_t RotationDegreesCCW() const {
    return rotate_instances_ ? 180 : 0;
  }

 private:
  void Place(
      geometry::Instance *instance,
      int64_t *x_pos,
      int64_t *y_pos,
      int64_t *distance_to_tap) const;

  // If set, automatically insert taps according to the rules of the physical
  // database.
  std::optional<std::reference_wrapper<const bfg::Cell>> tap_cell_;

  std::optional<std::reference_wrapper<const bfg::Cell>> start_cell_;
  std::optional<std::reference_wrapper<const bfg::Cell>> end_cell_;

  geometry::Point lower_left_;

  // If true, rotate all instances 180 degrees.
  bool rotate_instances_;

  bfg::Layout *layout_;
  bfg::Circuit *circuit_;
  bfg::DesignDatabase *design_db;


  std::vector<geometry::Instance*> instances_;
};

}  // namespace bfg

#endif  // ROW_GUIDE_H_
