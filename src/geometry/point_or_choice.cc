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

std::string PointOrChoice::Describe() const {
  std::stringstream ss;
  if (unique_) {
    ss << *unique_;
  } else if (choose_one_) {
    std::vector<std::string> point_descriptions;
    for (const geometry::Point &point : *choose_one_) {
      std::stringstream ss;
      ss << point;
      point_descriptions.push_back(ss.str());
    }
    ss << "(" << absl::StrJoin(point_descriptions, ", ") << ")";
  }
  if (maybe_internal_) {
    ss << " maybe_internal";
  }
  if (is_corner_) {
    ss << " is_corner";
  }
  if (crosses_boundary_) {
    ss << " crosses_boundary";
  }
  return ss.str();
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
        lhs) < reference.L2SquaredDistanceTo(rhs);
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
  os << choice.Describe();
  return os;
}

}  // namespace geometry
