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
  PointOrChoice(const Point &point) {
    unique_ = point;
  }

  PointOrChoice(const std::set<Point> &&points) {
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
  const std::optional<std::set<Point>> &choose_one() const {
    return choose_one_;
  }

 private:
  const Point MinOrMaxPoint(const Point &reference, bool use_max) const;

  std::optional<Point> unique_;
  // NOTE(aryap): I have wrapped the set in std::optional because I hope that
  // will save space, since in the vast majority of cases the set is unused.
  std::optional<std::set<Point>> choose_one_;
};

}  // namespace bfg
}  // namespace geometry

#endif  // GEOMETRY_POINT_OR_CHOICE_H_
