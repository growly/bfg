#include "routing_edge.h"

#include <cmath>
#include <optional>
#include <set>
#include <vector>

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_vertex.h"
#include "routing_track.h"

namespace bfg {

std::optional<geometry::Rectangle> RoutingEdge::AsRectangle(
    int64_t width) const {
  const geometry::Point &one_end = first_->centre();
  const geometry::Point &other_end = second_->centre();

  int64_t half_width = width / 2;

  if (one_end.x() == other_end.x()) {
    // Vertical rectangle.
    return {{{one_end.x() - half_width, one_end.y()},
             {other_end.x() + (1 - half_width), other_end.y()}}};
  } else if (one_end.y() == other_end.y()) {
    // Horizontal rectangle.
    return {{{one_end.x(), one_end.y() - half_width},
             {other_end.x(), other_end.y() - (1 - half_width)}}};
  }
  return std::nullopt;
}

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
