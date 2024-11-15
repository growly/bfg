#include "routing_vertex.h"

#include "equivalent_nets.h"
#include "geometry/compass.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "routing_edge.h"
#include "routing_track.h"

namespace bfg {

bool RoutingVertex::Compare(
    const RoutingVertex &lhs, const RoutingVertex &rhs) {
  return geometry::Point::CompareXThenY(lhs.centre(), rhs.centre());
}

bool RoutingVertex::Compare(RoutingVertex *lhs, RoutingVertex *rhs) {
  // nullptrs should be ordered first.
  // nullptr < x == true
  // x < nullptr == false
  // nullptr < nullptr == true
  if (!lhs) return true;
  if (!rhs) return false;
  return Compare(*lhs, *rhs);
}

void RoutingVertex::UpdateCachedStatus() {
  totally_available_ = !forced_blocked_ && !temporarily_forced_blocked_ &&
                       in_use_by_nets_.empty() &&
                       blocked_by_nearby_nets_.empty();
}

// This mutates blocking state and should call UpdateCachedStatus().
void RoutingVertex::AddUsingNet(const std::string &net, bool temporary) {
  auto it = in_use_by_nets_.find(net);
  if (it == in_use_by_nets_.end()) {
    in_use_by_nets_[net] = temporary;
    return;
  }
  // If the blockage was previously temporary but is now not, override it as permanent.
  // We trade one check of the existing value for what might be an unnecessary write.
  if (!temporary) {
    it->second = temporary;
  }
  UpdateCachedStatus();
}

// This mutates blocking state and should call UpdateCachedStatus().
void RoutingVertex::AddBlockingNet(const std::string &net, bool temporary) {
  auto it = blocked_by_nearby_nets_.find(net);
  if (it == blocked_by_nearby_nets_.end()) {
    blocked_by_nearby_nets_[net] = temporary;
    return;
  }
  // If the blockage was previously temporary but is now not, override it as permanent.
  // We trade one check of the existing value for what might be an unnecessary write.
  if (!temporary) {
    it->second = temporary;
  }
  UpdateCachedStatus();
}

// This mutates blocking state and should call UpdateCachedStatus().
void RoutingVertex::SetForcedBlocked(bool blocked, bool temporary) {
  bool &forced_blocked =
      temporary ? temporarily_forced_blocked_ : forced_blocked_;
  forced_blocked = blocked;
  UpdateCachedStatus();
}

// This mutates blocking state and should call UpdateCachedStatus().
void RoutingVertex::ResetTemporaryStatus() {
  temporarily_forced_blocked_ = false;

  auto remove_temporaries_fn = [](std::map<std::string, bool> *container) {
    for (auto it = container->begin(); it != container->end();) {
      if (it->second) {
        it = container->erase(it);
        continue;
      }
      ++it;
    }
  };
  remove_temporaries_fn(&in_use_by_nets_);
  remove_temporaries_fn(&blocked_by_nearby_nets_);

  UpdateCachedStatus();
}

std::optional<std::string> RoutingVertex::InUseBySingleNet() const {
  if (in_use_by_nets_.size() != 1) {
    return std::nullopt;
  }
  return in_use_by_nets_.begin()->first;
}

std::optional<std::string> RoutingVertex::BlockedBySingleNearbyNet() const {
  if (blocked_by_nearby_nets_.size() != 1) {
    return std::nullopt;
  }
  return blocked_by_nearby_nets_.begin()->first;
}

bool RoutingVertex::AvailableForNets(const EquivalentNets &nets) const {
  if (Available()) {
    return true;
  }
  std::optional<std::string> in_use_by_net = InUseBySingleNet();
  std::optional<std::string> blocked_by_nearby_net = BlockedBySingleNearbyNet();
  if (in_use_by_net && blocked_by_nearby_net &&
      *in_use_by_net != *blocked_by_nearby_net) {
    return false;
  }
  return (in_use_by_net && nets.Contains(*in_use_by_net)) ||
         (blocked_by_nearby_net && nets.Contains(*blocked_by_nearby_net));
}

std::optional<geometry::Layer> RoutingVertex::ConnectedLayerOtherThan(
    const geometry::Layer &layer) const {
  std::set<geometry::Layer> layers = connected_layers_;
  layers.erase(layer);
  if (layers.empty()) {
    return std::nullopt;
  }
  return *layers.begin();
}

std::set<geometry::Layer> RoutingVertex::CommonLayers(
      const RoutingVertex &lhs, const RoutingVertex &rhs) {
  std::set<geometry::Layer> shared_layers;
  std::set_intersection(
      lhs.connected_layers().begin(), lhs.connected_layers().end(),
      rhs.connected_layers().begin(), rhs.connected_layers().end(),
      std::inserter(shared_layers, shared_layers.begin()));
  return shared_layers;
}

bool RoutingVertex::RemoveEdge(RoutingEdge *edge) {
  size_t erased = edges_.erase(edge) > 0;
  return erased > 0;
}

int64_t RoutingVertex::L1DistanceTo(const geometry::Point &point) {
  return centre_.L1DistanceTo(point);
}

void RoutingVertex::AddNeighbour(const geometry::Compass &position,
                                 RoutingVertex *vertex) {
  if (!vertex) {
    return;
  }
  neighbours_.push_back({
      .position = position,
      .vertex = vertex
  });
}

std::set<RoutingVertex*> RoutingVertex::GetNeighbours(
    const geometry::Compass &position) const {
  std::set<RoutingVertex*> neighbours;
  for (const auto &neighbour : neighbours_) {
    if (neighbour.position == position) {
      neighbours.insert(neighbour.vertex);
    }
  }
  return neighbours;
}

RoutingEdge *RoutingVertex::GetEdgeOnLayer(const geometry::Layer &layer) const {
  // FIXME(aryap): Why do we have in_edge_, out_edge_ and edges_? Since this is
  // added for RoutingGrid::InstallVertexInPath we will use the in_ and
  // out_edge_ fields:
  for (RoutingEdge *edge : {in_edge_, out_edge_}) {
    if (!edge) {
      continue;
    }
    if (edge->EffectiveLayer() == layer) {
      return edge;
    }
  }
  return nullptr;
}

std::set<RoutingVertex*> RoutingVertex::GetNeighbours() const {
  std::set<RoutingVertex*> neighbours;
  for (const auto &neighbour : neighbours_) {
    neighbours.insert(neighbour.vertex);
  }
  return neighbours;
}

std::vector<RoutingTrack*> RoutingVertex::Tracks() const {
  std::vector<RoutingTrack*> tracks;
  if (horizontal_track_) {
    tracks.push_back(horizontal_track_);
  }
  if (vertical_track_) {
    tracks.push_back(vertical_track_);
  }
  return tracks;
}

std::vector<RoutingTrack*> RoutingVertex::TracksOnLayer(
    const geometry::Layer &layer) const {
  std::vector<RoutingTrack*> tracks;
  if (horizontal_track_ && horizontal_track_->layer() == layer) {
    tracks.push_back(horizontal_track_);
  }
  if (vertical_track_ && vertical_track_->layer() == layer) {
    tracks.push_back(vertical_track_);
  }
  return tracks;
}

std::vector<RoutingTrack*> RoutingVertex::TracksInDirection(
    const RoutingTrackDirection &direction) const {
  std::vector<RoutingTrack*> tracks;
  if (horizontal_track_ && horizontal_track_->direction() == direction) {
    tracks.push_back(horizontal_track_);
  }
  if (vertical_track_ && vertical_track_->direction() == direction) {
    tracks.push_back(vertical_track_);
  }
  return tracks;
}

std::ostream &operator<<(std::ostream &os, const RoutingVertex &vertex) {
  os << vertex.centre();
  os << (vertex.Available() ? " available" : " not_available");

  std::optional<std::string> in_use_by_net = vertex.InUseBySingleNet();
  if (in_use_by_net) {
    os << " in_use_by_net:\"" << *in_use_by_net << "\"";
  }
  std::optional<std::string> blocked_by_nearby_net =
      vertex.BlockedBySingleNearbyNet();
  if (blocked_by_nearby_net) {
    os << " blocked_by_nearby_net:\"" << *blocked_by_nearby_net
       << "\"";
  }
  return os;
}

}  // namespace bfg
