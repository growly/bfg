#include "routing_track_direction.h"

#include <set>

namespace bfg {

const std::set<RoutingTrackDirection>
RoutingTrackDirectionUtility::kAllDirections = {
    RoutingTrackDirection::kTrackHorizontal,
    RoutingTrackDirection::kTrackVertical
};

std::ostream &operator<<(
    std::ostream &os, const RoutingTrackDirection &direction) {
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      os << "horizontal";
      break;
    case RoutingTrackDirection::kTrackVertical:
      os << "vertical";
      break;
    default:
      os << "unknown";
      break;
  }
  return os;
}

}   // namespace bfg
