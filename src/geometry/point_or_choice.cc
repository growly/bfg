#include "point_or_choice.h"

#include <algorithm>
#include <optional>
#include <set>
#include <sstream>

#include <absl/strings/str_join.h>
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

std::ostream &operator<<(std::ostream &os,
                         const geometry::PointOrChoice &choice) {
  if (choice.unique()) {
    os << *choice.unique();
  } else if (choice.choose_one()) {
    std::vector<std::string> point_descriptions;
    for (const geometry::Point &point : *choice.choose_one()) {
      std::stringstream ss;
      ss << point;
      point_descriptions.push_back(ss.str());
    }
    os << "(" << absl::StrJoin(point_descriptions, ", ") << ")";
  }
  if (choice.maybe_internal()) {
    os << " maybe_internal";
  }
  if (choice.is_corner()) {
    os << " is_corner";
  }
  if (choice.crosses_boundary()) {
    os << " crosses_boundary";
  }
  return os;
}

}  // namespace geometry
