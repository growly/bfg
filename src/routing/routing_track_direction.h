#ifndef ROUTING_TRACK_DIRECTION_H_
#define ROUTING_TRACK_DIRECTION_H_

#include <ostream>
#include <set>

namespace bfg {
namespace routing {

enum class RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

class RoutingTrackDirectionUtility {
 public:
  static const std::set<RoutingTrackDirection> kAllDirections;
  static double DirectionToAngle(
      const RoutingTrackDirection &direction);
};

std::ostream &operator<<(
    std::ostream &os, const RoutingTrackDirection &direction);

}  // namespace routing
}  // namespace bfg

#endif  // ROUTING_TRACK_DIRECTION_H_
