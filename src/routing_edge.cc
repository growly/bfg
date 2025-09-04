#include "routing_edge.h"

#include <sstream>
#include <cmath>
#include <optional>
#include <set>
#include <vector>

#include <absl/strings/str_cat.h>

#include "equivalent_nets.h"
#include "geometry/point.h"
#include "geometry/line.h"
#include "geometry/rectangle.h"
#include "physical_properties_database.h"
#include "routing_edge.h"
#include "routing_vertex.h"
#include "routing_track.h"

namespace bfg {

bool RoutingEdge::Blocked() const {
  return blocked_ || temporarily_blocked_;
}

const std::optional<std::string> &RoutingEdge::EffectiveNet() const {
  LOG_IF(FATAL, temporarily_in_use_by_net_ && in_use_by_net_)
      << "RoutingEdge should not be assigned both in_use_by_net_ ("
      << *in_use_by_net_ << ") and temporarily_in_use_by_net_ ("
      << *temporarily_in_use_by_net_
      << ") simultaneously";
  if (temporarily_in_use_by_net_) {
    return temporarily_in_use_by_net_;
  }
  return PermanentNet();
}

const std::optional<std::string> &RoutingEdge::PermanentNet() const {
  return in_use_by_net_;
}

void RoutingEdge::SetNet(
    const std::optional<std::string> &in_use_by_net, bool temporary) {
  if (temporary) {
    // LOG_IF(FATAL, in_use_by_net_) << "in_use_by_net_ already set";
    temporarily_in_use_by_net_ = in_use_by_net;
  } else {
    // LOG_IF(FATAL, temporarily_in_use_by_net_)
    //     << "temporarily_in_use_by_net_ already set";
    in_use_by_net_ = in_use_by_net;
  }
}

RoutingVertex *RoutingEdge::OtherVertexThan(RoutingVertex *given) const {
  if (given == first_) {
    return second_;
  } else if (given == second_) {
    return first_;
  }
  return nullptr;
}

std::pair<int64_t, int64_t> RoutingEdge::ProjectOntoAxis() const {
  return RoutingTrack::ProjectOntoAxis(
      first_->centre(), second_->centre(), Direction());
}

void RoutingEdge::SetBlocked(bool blocked, bool temporary) {
  if (temporary) {
    temporarily_blocked_ = blocked;
  } else {
    blocked_ = blocked;
  }
}

std::optional<geometry::Rectangle> RoutingEdge::AsRectangle(
    int64_t width) const {
  auto maybe_rectangle = geometry::Rectangle::FromCentralAxis(
      first_->centre(), second_->centre(), width);
  if (!maybe_rectangle) {
    return std::nullopt;
  }
  if (layer_) {
    maybe_rectangle->set_layer(*layer_);
  }
  return *maybe_rectangle;
}

std::optional<geometry::Line> RoutingEdge::AsLine() const {
  if (!first_ || !second_) {
    return std::nullopt;
  }
  return geometry::Line(first_->centre(), second_->centre());
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

const geometry::Layer RoutingEdge::EffectiveLayer() const {
  if (layer_)
    return *layer_;
  if (track_ != nullptr)
    return track_->layer();
  LOG(FATAL) << "Edge has no explicit layer and no parent track";
  return -1;
}

RoutingTrackDirection RoutingEdge::Direction() const {
  if (track_) {
    return track_->direction();
  }
  // Off-grid, so attempt to determine what our direction is:
  if (first_->centre().x() == second_->centre().x()) {
    return RoutingTrackDirection::kTrackVertical;
  }
  LOG_IF(FATAL, first_->centre().y() != second_->centre().y())
      << "Track " << *this << " is not horizontal or vertical";
  return RoutingTrackDirection::kTrackHorizontal;
}

double RoutingEdge::Length() const {
  return first_->centre().L2DistanceTo(second_->centre());
}

bool RoutingEdge::TerminatesAt(const geometry::Point &point) const {
  return (first_ && first_->centre() == point) ||
      (second_ && second_->centre() == point);
}

bool RoutingEdge::IsBetween(
    const geometry::Point &lhs, const geometry::Point &rhs) const {
  LOG_IF(FATAL, !first_ || !second_)
      << "Edge is missing either the first (" << first_
      << ") or second (" << second_ << ") vertex";
  return (first_->centre() == lhs && second_->centre() == rhs) ||
         (first_->centre() == rhs && second_->centre() == lhs);
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
  cost_ = distance; //std::log(distance);
  // LOG(INFO) << "edge " << first_->centre() << " to " << second_->centre()
  //           << " cost is " << cost_;
}

bool RoutingEdge::IsRectilinear() const {
  return first_ && second_ && geometry::Line::PointsFormRectilinearLine(
      first_->centre(), second_->centre());
}

bool RoutingEdge::AvailableForNets(
    const EquivalentNets &ok_nets) const {
  if (Available()) {
    return true;
  }

  if (Blocked()) {
    VLOG(16) << "Edge " << *this << " is blocked";
    return false;
  }

  if (EffectiveNet() && ok_nets.Contains(*EffectiveNet())) {
    return true;
  } else {
    VLOG(16) << "Cannot use edge " << *this << " for net "
             << ok_nets.primary();
  }
  return false;
}

std::string RoutingEdge::Describe() const {
  std::stringstream ss;
  if (first_) {
    ss << first_->centre();
  } else {
    ss << "nullptr";
  }
  ss << " to ";
  if (second_) {
    ss << second_->centre();
  } else {
    ss << "nullptr";
  }
  if (in_use_by_net_) {
    ss << " used by net: " << *in_use_by_net_;
  }
  if (blocked_) {
    ss << " blocked";
  }
  return ss.str();
}

std::ostream &operator<<(std::ostream &os, const RoutingEdge &edge) {
  os << edge.Describe();
  return os;
}

}  // namespace bfg
