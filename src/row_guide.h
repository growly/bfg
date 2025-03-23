#ifndef ROW_GUIDE_H_
#define ROW_GUIDE_H_

#include <ostream>

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
      const geometry::Point &origin,
      bfg::Layout *layout,
      bfg::Circuit *circuit,
      DesignDatabase *design_db)
      : origin_(origin),
        layout_(layout),
        circuit_(circuit),
        design_db(design_db),
        start_with_tap_(true),
        num_taps_(0),
        distance_to_tap_left_(0),
        distance_to_tap_right_(0) {
    max_tap_distance_ = 15000;
  }

  // Methods to insert new instances of a given template layout, with a given
  // name, at either:
  //  - the back of the row, extending the row past where it currently is;
  //
  //          +------+------+ - - -+
  //          |  A   |  B   | new  |
  //          +------+------+ - - -+
  //                        ^
  //                        appended
  //
  //  - the front of the row, shifting the position of all other instances back;
  //    and
  //
  //                                 existing instances
  //                   ---->| ---->| moved right
  //          + - - -+------+------+
  //          | new  |  A   |  B   |
  //          + - - -+------+------+
  //          ^
  //          inserted
  //
  //  - in front of the front of the row, leaving existing placements
  //    unchanged.
  //
  //          existing instances do not move
  //          v
  //   + - - -+------+------+
  //   | new  |  A   |  B   |
  //   + - - -+------+------+
  //   ^
  //   prefixed
  //
  // Caller takes ownership of instance. Instance's position and orientation may
  // be changed. It is assumed that the given instance is already installed in a
  // bfg::Layout (and its circuit representation exists in a bfg::Circuit).
  geometry::Instance *InstantiateBack(
      const std::string &name, Layout *template_layout);
  geometry::Instance *InstantiateAndInsertFront(
      const std::string &name, Layout *template_layout);
  geometry::Instance *InstantiateFront(
      const std::string &name, Layout *template_layout);

  uint64_t Width() const;
  uint64_t Height() const;

  std::optional<geometry::Rectangle> GetBoundingBox() const;

  geometry::Point UpperRight() const;
  geometry::Point LowerRight() const;
  geometry::Point UpperLeft() const;
  geometry::Point LowerLeft() const;

  // Move the origin of this RowGuide and all instances such that the relative
  // position is conserved.
  void MoveTo(const geometry::Point &point);
  void MoveLowerLeft(const geometry::Point &point);
  void MoveLowerRight(const geometry::Point &point);

  void set_origin(const geometry::Point &point) { origin_ = point; }
  const geometry::Point &origin() const { return origin_; }

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
  void Place(const geometry::Point &point,
             geometry::Instance *instance,
             int64_t *distance_to_tap);
  void MaybeAddTapLeftFor(const geometry::Instance &added_instance);
  void MaybeAddTapRightFor(const geometry::Instance &added_instance);

  bool NeedsTapLeft(const geometry::Instance &added_instance) const;
  bool NeedsTapRight(const geometry::Instance &added_instance) const;
  bool NeedsTap(const int64_t &current_distance,
                int64_t additional_distance) const;

  void ShiftAllRight(int64_t x);

  geometry::Point NextPointLeft(const geometry::Instance &to_add) const;
  geometry::Point NextPointRight(const geometry::Instance &to_add) const;

  geometry::Instance *AddTap();

  // If set, automatically insert taps according to the rules of the physical
  // database.
  std::optional<std::reference_wrapper<const bfg::Cell>> tap_cell_;

  std::optional<std::reference_wrapper<const bfg::Cell>> start_cell_;
  std::optional<std::reference_wrapper<const bfg::Cell>> end_cell_;

  geometry::Point origin_;

  // If true, rotate all instances 180 degrees.
  bool rotate_instances_;

  bfg::Layout *layout_;
  bfg::Circuit *circuit_;
  bfg::DesignDatabase *design_db;

  // Stores all instances in the row in order from left to right with increasing
  // index. If the row's cells are rotated (rotate_instances_ is true) then each
  // instance is rotated, but the order is still left-to-right.
  std::vector<geometry::Instance*> instances_;

  bool start_with_tap_;
  int64_t num_taps_;
  int64_t distance_to_tap_right_;
  int64_t distance_to_tap_left_;

  int64_t max_tap_distance_;
};

}  // namespace bfg

#endif  // ROW_GUIDE_H_
