#include "point_or_choice.h"

#include <algorithm>
#include <optional>
#include <set>

#include <glog/logging.h>

#include "point.h"

namespace bfg {
namespace geometry {

bool PointOrChoice::Contains(const Point &point) {
  if (unique_) {
    return *unique_ == point;
  }
  if (choose_one_) {
    return choose_one_->find(point) != choose_one_->end();
  }
  return false;
}

const Point PointOrChoice::MinOrMaxPoint(const Point &reference, bool use_max)
  const {
  if (unique_) {
    return *unique_;
  }
  LOG_IF(FATAL, !choose_one_)
      << "PointOrChoice should have one of unique_ and choose_one_ "
      << "available.";
  auto comparator = [&](const Point &lhs, const Point &rhs) {
    return reference.L2SquaredDistanceTo(
        lhs) > reference.L2SquaredDistanceTo(rhs);
  };
  auto it = use_max ?
      std::max_element(choose_one_->begin(), choose_one_->end(), comparator) :
      std::min_element(choose_one_->begin(), choose_one_->end(), comparator);
  LOG_IF(FATAL, it == choose_one_->end())
      << "choose_one_ should not be empty, so there must be at least one "
      << "minimum element.";
  return *it;
}

}  // namespace bfg
}  // namespace geometry
