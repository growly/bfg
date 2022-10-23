#include "routing_edge.h"

#include <set>
#include <vector>

#include "routing_edge.h"
#include "routing_vertex.h"
#include "routing_track.h"

namespace bfg {

void RoutingEdge::set_track(RoutingTrack *track) {
  track_ = track;
  if (track_ != nullptr) set_layer(track_->layer());
}

const geometry::Layer &RoutingEdge::ExplicitOrTrackLayer() const {
  if (track_ != nullptr)
    return track_->layer();
  return layer_;
}

void RoutingEdge::PrepareForRemoval() {
  if (first_)
    first_->RemoveEdge(this);
  if (second_)
    second_->RemoveEdge(this);
  track_ = nullptr;
}

// NOTE(aryap): This is a pretty interesting problem to solve:
void RoutingEdge::ApproximateCost() {
  // Proportional to the square of the distance.
  int64_t distance = first_->centre().L2DistanceTo(second_->centre());
  if (distance == 0) {
    cost_ = 0;
    return;
  }
  cost_ = std::log(distance);
  // LOG(INFO) << "edge " << first_->centre() << " to " << second_->centre()
  //           << " cost is " << cost_;
}

}  // namespace bfg
