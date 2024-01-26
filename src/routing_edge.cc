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
  VLOG_IF(13, in_use_by_net_) << *this << " now used by " << *in_use_by_net;
  in_use_by_net_ = in_use_by_net;
}

std::optional<geometry::Rectangle> RoutingEdge::AsRectangle(
    int64_t width) const {
  // There is no guaranteed order of first_ and second_, so we have to make sure
  // we have the lower left and upper right points the right way around. If we
  // put them in a vector and impose an ordering, we get this conceptually very
  // simply:
  std::vector<std::reference_wrapper<const geometry::Point>> points = {
      first_->centre(), second_->centre()
  };
  std::sort(points.begin(), points.end(),
            [](const geometry::Point &lhs, const geometry::Point &rhs) {
              if (lhs.x() == rhs.x()) {
                return lhs.y() < rhs.y();
              }
              return lhs.x() < rhs.x();
            });

  const geometry::Point &lower_left = points.front().get();
  const geometry::Point &upper_right = points.back().get();

  int64_t half_width = width / 2;
  int64_t remaining_width = width - half_width;

  if (lower_left.x() == upper_right.x()) {
    // Vertical rectangle.
    return {{{lower_left.x() - half_width, lower_left.y()},
             {upper_right.x() + remaining_width, upper_right.y()}}};
  } else if (lower_left.y() == upper_right.y()) {
    // Horizontal rectangle.
    return {{{lower_left.x(), lower_left.y() - half_width},
             {upper_right.x(), upper_right.y() + remaining_width}}};
  }

  return std::nullopt;
}

std::vector<RoutingVertex*> RoutingEdge::SpannedVertices() const {
  if (!track_) {
    return {first_, second_};
  }

  return track_->VerticesInSpan(first_->centre(), second_->centre());
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

bool RoutingEdge::IsRectilinear() const {
  return first_ && second_ && (
      first_->centre().x() == second_->centre().x() ||
      first_->centre().y() == second_->centre().y());
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
