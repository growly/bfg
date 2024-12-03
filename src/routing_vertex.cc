#include "routing_vertex.h"

#include <map>
#include <optional>
#include <set>
#include <variant>
#include <vector>

#include "equivalent_nets.h"
#include "geometry/compass.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"
#include "routing_edge.h"
#include "routing_track.h"

#include <absl/cleanup/cleanup.h>
#include <absl/strings/str_join.h>

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

void RoutingVertex::AddEdges(RoutingEdge *in, RoutingEdge *out) {
  if (in == nullptr && out == nullptr) {
    return;
  }
  in_out_edges_.insert({in, out});
}

bool RoutingVertex::ChangesEdge() const {
  for (const auto &pair : in_out_edges_) {
    if (pair.first != pair.second) {
      return true;
    }
  }
  return false;
}

void RoutingVertex::UpdateCachedStatus() {
  totally_available_ = !forced_blocked_ && !temporarily_forced_blocked_ &&
                       in_use_by_nets_.empty() &&
                       blocked_by_nearby_nets_.empty();
}

void RoutingVertex::AddUsingNet(
    const std::string &net,
    bool temporary,
    std::optional<geometry::Layer> layer,
    std::optional<const geometry::Rectangle*> blocking_rectangle,
    std::optional<const geometry::Polygon*> blocking_polygon) {
  if (net == "") {
    return;
  }
  // This mutates blocking state and should call UpdateCachedStatus() before it
  // exits.
  absl::Cleanup update_cached_status = [&]() { UpdateCachedStatus(); };

  auto it = in_use_by_nets_.find(net);
  if (it == in_use_by_nets_.end()) {
    auto new_entry = in_use_by_nets_.insert(
        {net, std::vector<NetHazardInfo>()});
    // Ignore a failed insertion because then we'd have bigger problems.
    it = new_entry.first;
  }

  NetHazardInfo hazard = {
    .is_temporary = temporary,
    .layer = layer
  };
  if (blocking_rectangle) {
    hazard.blockage = blocking_rectangle;
  } else if (blocking_polygon) {
    hazard.blockage = blocking_polygon;
  }

  it->second.push_back(hazard);
}

void RoutingVertex::AddBlockingNet(
    const std::string &net,
    bool temporary,
    std::optional<geometry::Layer> layer,
    std::optional<const geometry::Rectangle*> blocking_rectangle,
    std::optional<const geometry::Polygon*> blocking_polygon) {
  if (net == "") {
    return;
  }
  // This mutates blocking state and should call UpdateCachedStatus() before it
  // exits.
  absl::Cleanup update_cached_status = [&]() { UpdateCachedStatus(); };

  auto it = blocked_by_nearby_nets_.find(net);
  if (it == blocked_by_nearby_nets_.end()) {
    auto new_entry = blocked_by_nearby_nets_.insert(
        {net, std::vector<NetHazardInfo>()});
    // Ignore a failed insertion because then we'd have bigger problems.
    it = new_entry.first;
  }

  NetHazardInfo hazard = {
    .is_temporary = temporary,
    .layer = layer
  };
  if (blocking_rectangle) {
    hazard.blockage = blocking_rectangle;
  } else if (blocking_polygon) {
    hazard.blockage = blocking_polygon;
  }

  it->second.push_back(hazard);
}

// This mutates blocking state and should call UpdateCachedStatus() before it
// exits.
void RoutingVertex::SetForcedBlocked(bool blocked, bool temporary) {
  bool &forced_blocked =
      temporary ? temporarily_forced_blocked_ : forced_blocked_;
  forced_blocked = blocked;
  UpdateCachedStatus();
}

void RoutingVertex::RemoveTemporaryHazardsFrom(
    std::map<std::string, std::vector<NetHazardInfo>> *container) {
  for (auto outer_it = container->begin(); outer_it != container->end();) {
    std::vector<NetHazardInfo> &inner = outer_it->second;
    for (auto inner_it = inner.begin(); inner_it != inner.end();) {
      if (inner_it->is_temporary) {
        inner_it = inner.erase(inner_it);
        continue;
      }
      ++inner_it;
    }
    if (inner.empty()) {
      outer_it = container->erase(outer_it);
      continue;
    }
    ++outer_it;
  }
}

// This mutates blocking state and should call UpdateCachedStatus() before it
// exits.
void RoutingVertex::ResetTemporaryStatus() {
  temporarily_forced_blocked_ = false;

  RemoveTemporaryHazardsFrom(&in_use_by_nets_);
  RemoveTemporaryHazardsFrom(&blocked_by_nearby_nets_);

  UpdateCachedStatus();
}

std::optional<std::set<geometry::Layer>> RoutingVertex::GetNetLayers(
    const std::map<std::string, std::vector<NetHazardInfo>> &container,
    const std::string &net) const {
  auto it = container.find(net);
  if (it == container.end()) {
    return std::nullopt;
  }
  std::set<geometry::Layer> layers;
  const auto &inner = it->second;
  for (auto it = inner.begin(); it != inner.end(); ++it) {
    if (it->layer) {
      layers.insert(*it->layer);
    }
  }
  return layers;
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

  if (blocked_by_nearby_nets_.size() > 1) {
    return false;
  }
  std::optional<std::string> blocked_by_nearby_net = BlockedBySingleNearbyNet();

  // TODO(aryap): We should be tracking the EquivalentNets which use the vertex,
  // for convenience. It's possible that this will fail erroneously if two
  // equivalent nets are added as using nets. A shortcut for this is to reduce
  // the known-using-nets in in_use_by_nets_ according to the equivalence class
  // given by the 'nets' argument in this function.
  std::optional<std::string> in_use_by_net = InUseBySingleNet();

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

// On-grid neighbours.
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

std::optional<RoutingTrackDirection> RoutingVertex::GetEncapDirection(
    const geometry::Layer &layer) const {
  auto forced = GetForcedEncapDirection(layer);
  if (forced) {
    return *forced;
  }
  RoutingEdge *edge = GetFirstEdgeOnLayer(layer);
  if (edge) {
    return edge->Direction();
  }
  return std::nullopt;
}

RoutingEdge *RoutingVertex::GetFirstEdgeOnLayer(
    const geometry::Layer &layer) const {
  // FIXME(aryap): Why do we have in_edge_, out_edge_ and edges_? Since this is
  // added for RoutingGrid::InstallVertexInPath we will use the in_ and
  // out_edge_ fields:
  for (const auto &pair : in_out_edges_) {
    for (RoutingEdge *edge : {pair.first, pair.second}) {
      if (!edge) {
        continue;
      }
      if (edge->EffectiveLayer() == layer) {
        return edge;
      }
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
