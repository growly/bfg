#include "routing_track_direction.h"

#include "geometry/radian.h"

#include <set>

namespace bfg {

const std::set<RoutingTrackDirection>
RoutingTrackDirectionUtility::kAllDirections = {
    RoutingTrackDirection::kTrackHorizontal,
    RoutingTrackDirection::kTrackVertical
};

double RoutingTrackDirectionUtility::DirectionToAngle(
    const RoutingTrackDirection &direction) {
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      return 0.0;
    case RoutingTrackDirection::kTrackVertical:
      return geometry::Radian::kPi / 2;
    default:
      LOG(FATAL) << "Unknown RoutingTrackDirection: " << direction;
  }
  return 0.0;
}

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
