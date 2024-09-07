#ifndef ROUTING_TRACK_DIRECTION_H_
#define ROUTING_TRACK_DIRECTION_H_

#include <ostream>

namespace bfg {

enum class RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

std::ostream &operator<<(
    std::ostream &os, const RoutingTrackDirection &direction);

}  // namespace bfg

#endif  // ROUTING_TRACK_DIRECTION_H_
