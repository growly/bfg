#ifndef GEOMETRY_POINT_OR_CHOICE_H_
#define GEOMETRY_POINT_OR_CHOICE_H_

#include <algorithm>
#include <optional>
#include <set>

#include "point.h"

namespace bfg {
namespace geometry {

// When a line is incident on an edge of a polygon, it is not known which point
// on that edge should be counted as the intersection, since the polygon
// includes the edges themselves (closed). In those cases we have to store
// alternate points and leave it to a final process to decide between them,
// depending on the other intersecting points found.
class PointOrChoice {
 public:
  PointOrChoice()
      : maybe_internal_(false),
        is_corner_(false),
        crosses_boundary_(false) {}

  PointOrChoice(const Point &point)
      : maybe_internal_(false),
        is_corner_(false),
        crosses_boundary_(false) {
    unique_ = point;
  }

  PointOrChoice(const std::set<Point> &&points)
      : maybe_internal_(false),
        is_corner_(false),
        crosses_boundary_(false) {
    choose_one_ = points;
  }

  bool Contains(const Point &point);

  const Point ClosestPointTo(const Point &reference) const {
      return MinOrMaxPoint(reference, false);
  }
  const Point FurthestPointFrom(const Point &reference) const {
      return MinOrMaxPoint(reference, true);
  }

  const std::optional<Point> &unique() const { return unique_; }
  void set_unique(const Point &point) { unique_ = point; }

  const std::optional<std::set<Point>> &choose_one() const {
    return choose_one_;
  }
  void set_choose_one(const std::set<Point> &&points) {
    choose_one_ = points;
  }

  void set_maybe_internal(bool maybe_internal) {
    maybe_internal_ = maybe_internal;
  }
  bool maybe_internal() const { return maybe_internal_; }

  void set_is_corner(bool is_corner) {
    is_corner_ = is_corner;
  }
  bool is_corner() const { return is_corner_; }

  void set_crosses_boundary(bool crosses_boundary) {
    crosses_boundary_ = crosses_boundary;
  }
  bool crosses_boundary() const { return crosses_boundary_; }

 private:
  const Point MinOrMaxPoint(const Point &reference, bool use_max) const;

  // Whether this point choice represents a crossing of the polygon boundary.
  // A line intersecting with a polygon will need to use the points contained
  // herein or not depending on whether it is arriving from the inside or
  // outside and whether this is a crossing or not. These points might be on an
  // entirely-internal boundary.
  bool maybe_internal_;

  bool is_corner_;

  bool crosses_boundary_;

  std::optional<Point> unique_;
  // NOTE(aryap): I have wrapped the set in std::optional because I hope that
  // will save space, since in the vast majority of cases the set is unused.
  std::optional<std::set<Point>> choose_one_;
};

}  // namespace bfg

std::ostream &operator<<(std::ostream &os,
                         const geometry::PointOrChoice &choice);

}  // namespace geometry

#endif  // GEOMETRY_POINT_OR_CHOICE_H_
