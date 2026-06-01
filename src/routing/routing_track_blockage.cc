#include "routing_track_blockage.h"

namespace bfg {

bool RoutingTrackBlockage::Contains(int64_t position) const {
  return position >= start_ && position <= end_;
}

bool RoutingTrackBlockage::IsAfter(int64_t position) const {
  return position <= start_;
}

bool RoutingTrackBlockage::IsBefore(int64_t position) const {
  return position >= end_;
}

// Whether the given span [low, high] overlaps with this blockage.
bool RoutingTrackBlockage::Blocks(int64_t low, int64_t high) const {
  return Contains(low) || Contains(high) || (low <= start_ && high >= end_);
}

}  // namespace bfg
