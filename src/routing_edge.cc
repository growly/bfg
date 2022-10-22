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
  // TODO(aryap): Am not thinking about this hard enough but:
  for (RoutingVertex *vertex : spans_) {
    vertex->RemoveSpanningEdge(this);
  }
  spans_.clear();
  track_ = nullptr;
}

// NOTE(aryap): This is a pretty interesting problem to solve:
void RoutingEdge::ApproximateCost() {
  // Proportional to the square of the distance.
  cost_ = std::log(first_->centre().L2DistanceTo(second_->centre()));
  // LOG(INFO) << "edge " << first_->centre() << " to " << second_->centre()
  //           << " cost is " << cost_;
}

}  // namespace bfg
