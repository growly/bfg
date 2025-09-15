#include "routing_blockage_cache.h"

#include <map>
#include <set>
#include <optional>

#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_track_direction.h"
#include "routing_grid_blockage.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"

namespace bfg {

// TODO(aryap): We have to factor out of the ApplyBlockage et. al. functions
// all the stuff that needs to be checked to determine:
//  - if the blockage overlaps a vertex (so the vertex can be used to
//  connect to it directly)
//  - if the blockage inhibits a vertex, meaning it comes too close for the
//  vertex to accomodate a via, and in what directions.
template<typename T>
void RoutingBlockageCache::ApplyBlockageToOneVertex(
    const RoutingGridBlockage<T> &blockage,
    const RoutingVertex *vertex,
    std::optional<RoutingTrackDirection> access_direction) {
  const geometry::Layer &layer = blockage.shape().layer();

  // Check if the blockage overlaps the vertex completely:
  if (blockage.IntersectsPoint(vertex->centre(), 0)) {
    blocked_vertices_[vertex].AddUser(blockage);
    //VLOG(16) << "Blockage: " << blockage.shape()
    //         << " intersects " << vertex->centre()
    //         << " with margin " << 0;
  }

  // If it doesn't, check if there are viable directions the vertex can
  // still be used in:
  static const std::set<RoutingTrackDirection> kAllDirections = {
      RoutingTrackDirection::kTrackHorizontal,
      RoutingTrackDirection::kTrackVertical};

  // TODO(aryap): There's gotta be a more elegant way to either use a set with
  // one element or a static const set. Maybe a reference_wrapper?
  std::set<RoutingTrackDirection> test_directions = access_direction ?
      std::set<RoutingTrackDirection>{*access_direction} : kAllDirections;

  const std::string &net = blockage.shape().net();
  for (const auto &direction : test_directions) {
    // We use the RoutingGridBlockage to do a hit test; set
    // exceptional_nets = nullopt so that no exception is made.
    if (blockage.Blocks(*vertex, std::nullopt, direction)) {
      blocked_vertices_[vertex].AddInhibitor(direction, blockage);
      //VLOG(16) << "Blockage: " << blockage.shape()
      //         << " blocks " << vertex->centre()
      //         << " with padding=" << blockage.padding() << " in "
      //         << direction << " direction";
      continue;
    }
  }
}

RoutingBlockageCache::RoutingBlockageCache(const RoutingGrid &grid)
    : grid_(grid),
      search_window_margin_(grid.FigureSearchWindowMargin()) {}

void RoutingBlockageCache::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding) {
  // Find possibly-affected vertices.
  std::vector<const RoutingVertex*> vertices =
      DeterminePossiblyAffectedVertices(rectangle, padding);

  int64_t min_separation = grid_.GetMinSeparation(rectangle.layer());

  RoutingGridBlockage<geometry::Rectangle> *blockage =
      new RoutingGridBlockage<geometry::Rectangle>(
          grid_, rectangle, padding + min_separation);
  for (const RoutingVertex *vertex : vertices) {
    ApplyBlockageToOneVertex(*blockage, vertex, std::nullopt);
  }

  // Edge blockages are much simpler; we only need the shape and a net, and to
  // ask tracks (mostly) what edges are affected:
  std::vector<const RoutingEdge*> edges =
      DetermineAffectedEdges(rectangle, padding);
  for (const RoutingEdge *edge : edges) {
    blocked_edges_[edge].sources[rectangle.net()].insert(blockage);
  }

  rectangle_blockages_.emplace_back(blockage);
}

void RoutingBlockageCache::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding) {
  std::vector<const RoutingVertex*> vertices =
      DeterminePossiblyAffectedVertices(polygon, padding);

  int64_t min_separation = grid_.GetMinSeparation(polygon.layer());

  RoutingGridBlockage<geometry::Polygon> *blockage =
      new RoutingGridBlockage<geometry::Polygon>(
          grid_, polygon, padding + min_separation);

  for (const RoutingVertex *vertex : vertices) {
    ApplyBlockageToOneVertex(*blockage, vertex, std::nullopt);
  }

  std::vector<const RoutingEdge*> edges =
      DetermineAffectedEdges(polygon, padding);
  for (const RoutingEdge *edge : edges) {
    blocked_edges_[edge].sources[polygon.net()].insert(blockage);
  }

  polygon_blockages_.emplace_back(blockage);
}

// An edge is blocked if it has ANY blockages, UNLESS all of the blockages
// belong to nets which are contained in the "for_nets" set. An empty "for_nets"
// equivalence class indicates that no nets are acceptable excpetions, therefore
// any net is a blockage.
bool RoutingBlockageCache::IsEdgeBlocked(
    const RoutingEdge &edge,
    const EquivalentNets &for_nets) const {
  return IsEdgeBlocked(edge, for_nets, cancelled_blockages_);
}

bool RoutingBlockageCache::IsEdgeBlocked(
    const RoutingEdge &edge,
    const EquivalentNets &for_nets,
    const std::optional<std::vector<SourceBlockage>>
        &more_cancelled_blockages_) const {
  if (parent_ &&
      parent_->get().IsEdgeBlocked(edge, for_nets, cancelled_blockages_)) {
    return true;
  }
  auto entry = blocked_edges_.find(&edge);
  if (entry == blocked_edges_.end()) {
    return false;
  }
  const EdgeBlockages &blockages = entry->second;
  for (const auto &entry : blockages.sources) {
    const std::string &net = entry.first;

    // TODO(aryap): Exception checking (cancelled blockages) goes here. This
    // entry should only be considered if the blockages set is non-empty (after
    // removing cancellations).

    if (net == "" || !for_nets.Contains(net)) {
      // There exists a blockage which isn't excluded, or there are blockages
      // with no nets, which cannot be excluded.
      return true;
    }
  }
  return false;
}

bool RoutingBlockageCache::VertexBlockages::IsBlockedByUsers(
    const EquivalentNets &exceptional_nets,
    const std::optional<geometry::Layer> &on_layer) const {
  for (const auto &by_net_entry : users_) {
    const std::string &net = by_net_entry.first;

    // If there is any non-empty set of blockages on a layer matching *on_layer,
    // we test for net exceptions:
    for (const auto &by_layer_entry : by_net_entry.second) {
      const geometry::Layer &layer = by_layer_entry.first;
      if (on_layer && layer != *on_layer) {
        continue;
      }
      if (by_layer_entry.second.empty()) {
        // TODO(aryap): Or if they are all excluded...
        continue;
      }

      // By this point we have a non-excluded layer and a non-empty set of
      // blockages. Therefore we check if the nets are exceptionally allowed.
      // Otherwise, it's an applicable blockage.
      if (net == "" || !exceptional_nets.Contains(net)) {
        // There exists a blockage which isn't excluded, or there are blockages
        // with no nets, which cannot be excluded.
        return true;
      }

      // If we got here and no layer was specified then we tested the net it
      // didn't result in a block, therefore there is no way the next iteration
      // will yield anything different. So we can just break:
      if (!on_layer) {
        break;
      }
    }
  }
  return false;
}

bool RoutingBlockageCache::VertexBlockages::IsInhibitedInDirection(
    const std::optional<RoutingTrackDirection> &direction_or_any,
    const std::optional<geometry::Layer> &layer_or_any) const {
  for (const auto &[direction, by_layer] : inhibitors_) {
    if (direction_or_any && direction != *direction_or_any) {
      continue;
    }

    for (const auto &[layer, blockages] : by_layer) {
      if (layer_or_any && layer != *layer_or_any) {
        continue;
      }

      if (blockages.empty()) {
        // TODO(aryap): Or if they are all excluded...
        continue;
      }

      return true;
    }
  }
  return false;
}

// We need to accommodate the RoutingVertex availability checks
//   Available
//   AvailableForAll
//   AvailableForNetsOnAnyLayer
// The first and third more than the second.
//
// OK we're just going to copy the behaviour in - sorry, we're going to match
// the behaviour in - RoutingGrid/Blockage. If no direction is specified, the
// blockage is more restrictive, since the footprint is assumed to be a large
// square and not a minimally sized via encap. So if no direction is given, any
// blocked direction will count as a blockage.
bool RoutingBlockageCache::IsVertexBlocked(
    const RoutingVertex &vertex,
    const EquivalentNets &for_nets,
    const std::optional<RoutingTrackDirection> &direction_or_any,
    const std::optional<geometry::Layer> &layer_or_any) const {
  auto entry = blocked_vertices_.find(&vertex);
  if (entry == blocked_vertices_.end()) {
    return false;
  }
  
  const VertexBlockages &blockages = entry->second;
  if (blockages.IsBlockedByUsers(for_nets, layer_or_any)) {
    return true;
  }
  
  if (blockages.IsInhibitedInDirection(direction_or_any, layer_or_any)) {
    return true;
  }

  return false;
}

std::vector<const RoutingVertex*>
RoutingBlockageCache::DeterminePossiblyAffectedVertices(
    const geometry::Rectangle &rectangle,
    int64_t padding) const {
  const geometry::Layer &layer = rectangle.layer();

  std::set<RoutingVertex*> targets;

  // Check on-grid vertices for each RoutingGridGeometry in which the shape is
  // involved:
  auto grid_geometries = grid_.FindRoutingGridGeometriesUsingLayer(layer);
  for (auto ref : grid_geometries) {
    const RoutingGridGeometry &grid_geometry = ref.get();
    std::set<RoutingVertex*> enveloping_vertices;
    grid_geometry.EnvelopingVertices(
        rectangle,
        &enveloping_vertices,
        padding + search_window_margin_,
        1);   // Number of concentric vertex layers.
    targets.insert(enveloping_vertices.begin(),
                   enveloping_vertices.end());
  }

  // Check off-grid vertices:
  int64_t radius = std::max(
      rectangle.Width(), rectangle.Height()) + padding + search_window_margin_;
  std::vector<RoutingVertex*> nearby_off_grid =
      grid_.off_grid_vertices().FindNearby(rectangle.centre(), radius);

  // Yikes cv-fuckery.
  std::vector<const RoutingVertex*> const_targets;
  const_targets.reserve(targets.size() + nearby_off_grid.size());
  for (RoutingVertex *vertex : targets) {
    const_targets.push_back(vertex);
  }
  for (RoutingVertex *vertex : nearby_off_grid) {
    const_targets.push_back(vertex);
  }

  return const_targets;
}

template<typename T>
std::vector<const RoutingEdge*>
RoutingBlockageCache::DetermineAffectedOnGridEdges(
    const T &shape, int64_t padding) const {
  const geometry::Layer &layer = shape.layer();

  std::set<RoutingEdge*> targets;
  // Check on-grid vertices for each RoutingGridGeometry in which the shape is
  // involved:
  auto grid_geometries = grid_.FindRoutingGridGeometriesUsingLayer(layer);
  for (auto ref : grid_geometries) {
    const RoutingGridGeometry &grid_geometry = ref.get();
    std::set<RoutingTrack*> tracks;
    grid_geometry.NearestTracks(shape,
                                &tracks,
                                &tracks,
                                1);   // Nearest edges to the boundary, no more.
    for (RoutingTrack *track : tracks) {
      auto edges = track->EdgesBlockedByShape(shape, padding);
      if (!edges) {
        continue;
      }
      targets.insert(edges->begin(), edges->end());
    }
  }

  return std::vector<const RoutingEdge*>(targets.begin(), targets.end());
}

template<typename T>
std::vector<const RoutingEdge*>
RoutingBlockageCache::DetermineAffectedEdges(
    const T &shape,
    int64_t padding) const {
  const geometry::Layer &layer = shape.layer();

  std::vector<const RoutingEdge*> const_targets =
      DetermineAffectedOnGridEdges(shape, padding);

  // Check off-grid edges:
  const std::set<RoutingEdge*> off_grid_edges = grid_.off_grid_edges();
  // TODO(aryap): We just check all of these? Hello?
  std::vector<RoutingEdge*> off_grid_collisions;
  for (RoutingEdge *edge : off_grid_edges) {
    bool hazard = grid_.WireWouldIntersect(*edge, shape, padding);
    if (!hazard) {
      continue;
    }
    off_grid_collisions.push_back(edge);
  }

  // Yikes cv-fuckery.
  const_targets.reserve(const_targets.size() + off_grid_collisions.size());
  for (RoutingEdge *edge : off_grid_collisions) {
    const_targets.push_back(edge);
  }
  return const_targets;
}

template
std::vector<const RoutingEdge*>
RoutingBlockageCache::DetermineAffectedEdges(
    const geometry::Rectangle &rectangle,
    int64_t padding) const;

template
std::vector<const RoutingEdge*>
RoutingBlockageCache::DetermineAffectedEdges(
    const geometry::Polygon &polygon,
    int64_t padding) const;

}   // namespace bfg
