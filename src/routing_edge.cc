#include "routing_edge.h"

#include <cmath>
#include <optional>
#include <set>
#include <vector>

#include <absl/strings/str_cat.h>

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_vertex.h"
#include "routing_track.h"

namespace bfg {

void RoutingEdge::set_in_use_by_net(
    const std::optional<std::string> &in_use_by_net) {
  if (in_use_by_net_) {
    LOG(INFO) << *this << " now used by " << *in_use_by_net;
  }
  in_use_by_net_ = in_use_by_net;
}

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

std::ostream &operator<<(std::ostream &os, const RoutingEdge &edge) {
  if (edge.first()) {
    os << edge.first()->centre();
  } else {
    os << "nullptr";
  }
  os << " to ";
  if (edge.second()) {
    os << edge.second()->centre();
  } else {
    os << "nullptr";
  }
  if (edge.in_use_by_net()) {
    os << " used by net: " << *edge.in_use_by_net();
  }
  if (edge.blocked()) {
    os << " blocked";
  }
  return os;
}

}  // namespace bfg
