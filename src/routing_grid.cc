#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <utility>
#include <vector>

#include <absl/cleanup/cleanup.h>
#include <absl/status/status.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "geometry/compass.h"
#include "geometry/poly_line.h"
#include "geometry/rectangle.h"
#include "physical_properties_database.h"
#include "poly_line_cell.h"
#include "poly_line_inflator.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_path.h"
#include "routing_track.h"
#include "routing_track_blockage.h"
#include "routing_vertex.h"
#include "routing_via_info.h"

// TODO(aryap):
//  1) What does RoutingGrid::available_vertices_by_layer_ actually do?

// A RoutingGrid manages a multi-layer rectilinear wire grid for for
// connections between points.
//
// The grid comprises RoutingTracks, which own a set of RoutingVertexs and
// RoutingEdges. It also owns a separate collection of RoutingVertexs and
// RoutingEdges that do not fall onto specific tracks.
//
// NOTE(aryap): An optimisation for the router that prefers single-layer
// routing: maintaining the strict vertical/horizontal constraint on layers
// tends to create sub-optimal routes in congested areas because changing layers
// incurs forces other paths to go way around the long way. It might be better
// to assume single-layer routing, and then as a post-processing step find all
// intersections of used edges and elevate one of the conflicting edges to a
// different layer. But even if we assume that vertices must be able to
// accomodate a via at all times the post-processing step might backtrack us
// into an unroutable state.

using bfg::geometry::Compass;

namespace bfg {

// These partial specialisations need to occur before any use because dem's the
// rulz.
//
// We have a specialisation for {Rectangle, Polygon} X {Vertex, Edge}.
template<>
bool RoutingGridBlockage<geometry::Rectangle>::Blocks(
    const RoutingVertex &vertex,
    int64_t padding,
    std::optional<RoutingTrackDirection> access_direction) const {
  return routing_grid_.ViaWouldIntersect(
      vertex, shape_, padding, access_direction);
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::Blocks(
    const RoutingVertex &vertex,
    int64_t padding,
    std::optional<RoutingTrackDirection> access_direction) const {
  return routing_grid_.ViaWouldIntersect(
      vertex, shape_, padding, access_direction);
}

template<>
bool RoutingGridBlockage<geometry::Rectangle>::Blocks(
    const RoutingEdge &edge, int64_t padding) const {
  return routing_grid_.WireWouldIntersect(edge, shape_, padding);
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::Blocks(
    const RoutingEdge &edge, int64_t padding) const {
  return routing_grid_.WireWouldIntersect(edge, shape_, padding);
}

template<typename T>
void RoutingGridBlockage<T>::AddChildTrackBlockage(
    RoutingTrack *track, RoutingTrackBlockage *blockage) {
  child_track_blockages_.emplace_back(
      track, 
      std::unique_ptr<RoutingTrackBlockage>(blockage));
}

template<typename T>
void RoutingGridBlockage<T>::ClearChildTrackBlockages() {
  for (auto &entry : child_track_blockages_) {
    RoutingTrack *track = entry.first;
    RoutingTrackBlockage *blockage = entry.second.get();
    // NOTE(aryap): It is conceivable that RoutingGridBlockage would want to
    // store 'child' blockages which aren't temporary, but this is not the
    // case today.
    track->RemoveTemporaryBlockage(blockage);
  }
}

template<typename T>
RoutingGridBlockage<T>::~RoutingGridBlockage() {
  ClearChildTrackBlockages();
  // Objects destroyed.
}

template<typename T>
void RoutingGrid::ApplyBlockage(
    const RoutingGridBlockage<T> &blockage,
    std::set<RoutingVertex*> *blocked_vertices) {
  const geometry::Layer &layer = blockage.shape().layer();
  // Find any possibly-blocked vertices and make them unavailable:
  std::vector<RoutingGridGeometry*> grid_geometries =
      FindRoutingGridGeometriesUsingLayer(layer);
  for (RoutingGridGeometry *grid_geometry : grid_geometries) {
    std::set<RoutingVertex*> vertices;
    grid_geometry->EnvelopingVertices(blockage.shape(), &vertices);
    for (RoutingVertex *vertex : vertices) {
      if (!vertex->available())
        continue;

      std::set<RoutingTrackDirection> access_directions = {
          RoutingTrackDirection::kTrackHorizontal,
          RoutingTrackDirection::kTrackVertical};

      const std::string &net = blockage.shape().net();
      bool any_access = false;
      for (const auto &direction : access_directions) {
        bool blocked_at_all = false;
        if (blockage.BlocksWithoutPadding(*vertex, direction)) {
          blocked_at_all = true;
          vertex->set_net(blockage.shape().net());
          VLOG(16) << "Blockage: " << blockage.shape()
                   << " blocks " << vertex->centre()
                   << " directly (without padding) in "
                   << direction << " direction";
        } else if (blockage.Blocks(*vertex, direction)) {
          blocked_at_all = true;
          if (net != "") {
            vertex->set_connectable_net(net);
          }
          VLOG(16) << "Blockage: " << blockage.shape()
                   << " blocks " << vertex->centre()
                   << " with padding=" << blockage.padding() << " in "
                   << direction << " direction";
        }
        any_access = !blocked_at_all || any_access;
      }

      if (!any_access) {
        vertex->set_available(false);
        if (blocked_vertices) {
          blocked_vertices->insert(vertex);
        }
      }
    }
  }
}

template<>
void RoutingGrid::ForgetBlockage(
    RoutingGridBlockage<geometry::Rectangle> *blockage) {
  auto predicate = [&](
      const std::unique_ptr<RoutingGridBlockage<geometry::Rectangle>> &entry) {
      return entry.get() == blockage;
  };
  auto it = std::find_if(
      rectangle_blockages_.begin(), rectangle_blockages_.end(), predicate);
  if (it == rectangle_blockages_.end())
    return;
  rectangle_blockages_.erase(it);
}

template<>
void RoutingGrid::ForgetBlockage(
    RoutingGridBlockage<geometry::Polygon> *blockage) {
  auto predicate = [&](
      const std::unique_ptr<RoutingGridBlockage<geometry::Polygon>> &entry) {
      return entry.get() == blockage;
  };
  auto it = std::find_if(
      polygon_blockages_.begin(), polygon_blockages_.end(), predicate);
  if (it == polygon_blockages_.end())
    return;
  polygon_blockages_.erase(it);
}

bool RoutingGrid::ValidAgainstKnownBlockages(const RoutingEdge &edge) const {
  // *snicker* Cute opportunity for std::any_of here:
  for (const auto &blockage : rectangle_blockages_) {
    if (blockage->Blocks(edge))
      return false;
  }
  for (const auto &blockage : polygon_blockages_) {
    if (blockage->Blocks(edge))
      return false;
  }
  return true;
}

bool RoutingGrid::ValidAgainstKnownBlockages(
    const RoutingVertex &vertex,
    std::optional<RoutingTrackDirection> access_direction) const {
  // *snicker* Cute opportunity for std::any_of here:
  for (const auto &blockage : rectangle_blockages_) {
    if (blockage->Blocks(vertex, access_direction))
      return false;
  }
  for (const auto &blockage : polygon_blockages_) {
    if (blockage->Blocks(vertex, access_direction))
      return false;
  }
  return true;
}

bool RoutingGrid::ValidAgainstInstalledPaths(const RoutingEdge &edge) const {
  auto edge_footprint = EdgeFootprint(edge);
  if (!edge_footprint) {
    // No way to check.
    return false;
  }
  const geometry::Layer &layer = edge.EffectiveLayer();
  int64_t min_separation = physical_db_.Rules(layer).min_separation;

  // Check proximity to all installed edges:
  std::set<const RoutingEdge*> used_edges;
  for (const RoutingPath *path : paths_) {
    used_edges.insert(path->edges().begin(), path->edges().end());
  }
  for (const RoutingEdge *edge : off_grid_edges_) {
    used_edges.insert(off_grid_edges_.begin(), off_grid_edges_.end());
  }
  for (const RoutingEdge *edge : used_edges) {
    if (edge->EffectiveLayer() != layer)
      continue;
    auto existing_footprint = EdgeFootprint(*edge);
    if (!existing_footprint)
      continue;
    int64_t distance = static_cast<int64_t>(
        std::ceil(existing_footprint->ClosestDistanceTo(*edge_footprint)));
    if (distance < min_separation) {
      return false;
    }
  }
  return true;
}

bool RoutingGrid::ValidAgainstInstalledPaths(
    const RoutingVertex &vertex,
    std::optional<RoutingTrackDirection> access_direction) const {
  // FIXME(aryap): Use access_direction
  //
  // In this case we have to do labourious check for proximity to all used paths
  // and vertices.
  std::set<RoutingVertex*> used_vertices;
  for (RoutingPath *path : paths_) {
    used_vertices.insert(path->vertices().begin(), path->vertices().end());
  }

  for (const geometry::Layer &candidate_layer :
       vertex.connected_layers()) {
    auto routing_layer_info = GetRoutingLayerInfo(candidate_layer);
    if (!routing_layer_info) {
      // No routing layer info, probably not a routing layer.
      continue;
    }
    // TODO(aryap): We have fragmented sources for this information. Some
    // places I've used the PhysicalPropertiesDatabase, others the copies of
    // the data in the RoutingLayerInfo etc structures. Gross!
    int64_t min_separation = routing_layer_info->get().min_separation;

    std::optional<geometry::Rectangle> via_encap = ViaFootprint(
        vertex, candidate_layer, 0);   // No additional padding.

    if (!via_encap) {
      continue;
    }

    for (RoutingVertex *other : used_vertices) {
      if (other == &vertex) {
        continue;
      }

      for (const geometry::Layer &other_layer : other->connected_layers()) {
        if (candidate_layer != other_layer) {
          continue;
        }

        std::optional<geometry::Rectangle> other_via_encap = ViaFootprint(
           *other, other_layer, 0);

        int64_t distance = static_cast<int64_t>(
            std::ceil(via_encap->ClosestDistanceTo(*other_via_encap)));
        if (distance < min_separation) {
          VLOG(12) << "Candidate vertex " << vertex.centre()
                   << " is too close to " << other->centre() << " on layer "
                   << candidate_layer << " (distance " << distance <<
                   " < min separation " << min_separation << ")";
          return false;
        } else if (VLOG_IS_ON(16)) {
          VLOG(12) << "Candidate vertex " << vertex.centre()
                   << " is ok with " << other->centre() << " on layer "
                   << candidate_layer << " (distance " << distance <<
                   " >= min separation " << min_separation << ")";
        }
      }
    }
  }

  return true;
}

RoutingGrid::~RoutingGrid()  {
  for (auto entry : tracks_by_layer_) {
    for (RoutingTrack *track : entry.second) {
      delete track;
    }
  }
  for (RoutingPath *path : paths_) { delete path; }
  for (RoutingEdge *edge : off_grid_edges_) { delete edge; }
  for (RoutingVertex *vertex : vertices_) { delete vertex; }
}

// Return the (horizontal, vertical) routing infos.
// TODO(aryap): UGH using make_pair makes a copy of the references?
// I have to use std::reference_wrapper<const RoutingLayerInfo>?
// Why don't I just use pointers, exactly?
//    UGH compare this to the last signature:
//
//  std::pair<RoutingLayerInfo*, RoutingLayerInfo*>
//      RoutingGrid::PickHorizontalAndVertical(
//          const Layer &lhs, const Layer &rhs) const;
//
// And I could even use std::make_pair because it wasn't so smart:
//  https://stackoverflow.com/questions/53483124/reference-wrapper-make-pair-vs-class-template-argument-deduction-ctad
std::pair<std::reference_wrapper<const RoutingLayerInfo>,
          std::reference_wrapper<const RoutingLayerInfo>>
    RoutingGrid::PickHorizontalAndVertical(
        const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  const RoutingLayerInfo &lhs_info = GetRoutingLayerInfoOrDie(lhs);
  const RoutingLayerInfo &rhs_info = GetRoutingLayerInfoOrDie(rhs);
  if (lhs_info.direction == RoutingTrackDirection::kTrackHorizontal &&
      rhs_info.direction == RoutingTrackDirection::kTrackVertical) {
    return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
        lhs_info, rhs_info);
  } else if (lhs_info.direction == RoutingTrackDirection::kTrackVertical &&
             rhs_info.direction == RoutingTrackDirection::kTrackHorizontal) {
    return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
        rhs_info, lhs_info);
  }
  LOG(FATAL) << "Exactly one of each layer must be horizontal and one must be"
             << "vertical: " << lhs << ", " << rhs;
  return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
      lhs_info, rhs_info);
}

absl::StatusOr<RoutingGrid::VertexWithLayer> RoutingGrid::ConnectToGrid(
    const geometry::Port &port) {
  auto try_add_access_vertices = AddAccessVerticesForPoint(
      port.centre(), port.layer());
  if (try_add_access_vertices.ok()) {
    return *try_add_access_vertices;
  }
  // Fall back to slower, possibly broken method.
  auto try_nearest_available = ConnectToNearestAvailableVertex(port);
  if (try_nearest_available.ok()) {
    return *try_nearest_available;
  }

  return absl::NotFoundError("Could not connect to grid");
}

// Using the given RoutingGridGeometry, find the tracks which surround
// *off_grid and attempt to create vertices on each of those tracks for edges
// from *off_grid to land on.
//
// NOTE(aryap): Does not rollback changes to *off_grid on error, so assume the
// caller will just give up on the object and delete it from the grid.
absl::Status RoutingGrid::ConnectToSurroundingTracks(
    const RoutingGridGeometry &grid_geometry,
    const geometry::Layer &access_layer,
    std::optional<
        std::reference_wrapper<
            const std::set<RoutingTrackDirection>>> directions,
    RoutingVertex *off_grid) {
  std::set<RoutingTrack*> nearest_tracks;
  grid_geometry.NearestTracks(
      off_grid->centre(), &nearest_tracks, &nearest_tracks);

  auto ok_access_direction = [&](const RoutingTrackDirection &candidate) {
    if (directions) {
      const auto &access_directions = directions->get();
      return access_directions.find(candidate) != access_directions.end();
    }
    return true;
  };

  bool any_success = false;
  for (RoutingTrack *track : nearest_tracks) {
    RoutingVertex *bridging_vertex = nullptr;
    bool bridging_vertex_is_new = false;
    bool off_grid_already_exists = false;
    bool success = track->CreateNearestVertexAndConnect(
        *this,
        off_grid,
        access_layer,
        &bridging_vertex,
        &bridging_vertex_is_new,
        &off_grid_already_exists);

    if (!success) {
      continue;
    } else if (bridging_vertex == off_grid) {

      if (bridging_vertex_is_new) {
        return absl::InternalError(
          "Doesn't make sense for bridging_vertex == target "
          "and bridging_vertex_is_new to both be true");
      }

      // Since our off_grid vertex has landed on the track, the access direction
      // to the off_grid point is just the track direction. If this is not an
      // allowable direction, give up now:
      if (!ok_access_direction(track->direction())) {
        continue;
      }

      any_success = true || any_success;
      continue;
    }

    if (off_grid_already_exists) {
      std::stringstream ss;
      ss << *track << " already has a vertex at the position of off_grid "
         << off_grid->centre();
      return absl::InternalError(ss.str());
    }

    if (bridging_vertex_is_new) {
      AddVertex(bridging_vertex);
    }

    // At this point, the bridging vertex needs to be connected to off_grid.
    // Any condition that precludes that should have been handled already.

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid);
    edge->set_layer(access_layer);

    // Since we're creating a jog edge to connect to the off_grid point, we have
    // to check that its direction is permissible as an access direction.
    if (!ok_access_direction(edge->Direction())) {
      delete edge;
      continue;
    }

    // We do not check ValidAgainstInstalledPaths because that is slow. We hope
    // that by now the other rules have prevented such a possibility. Fingers
    // crossed....
    if (!ValidAgainstKnownBlockages(*edge)) {
      VLOG(15) << "Invalid off grid edge between "
               << bridging_vertex->centre()
               << " and " << off_grid->centre();
      // Rollback extra hard!
      RemoveVertex(bridging_vertex, true);  // and delete!
      delete edge;
      continue;
    }

    bridging_vertex->AddEdge(edge);
    off_grid->AddEdge(edge);
    off_grid_edges_.insert(edge);
    any_success = true || any_success;
  }
  return any_success ? absl::OkStatus() : absl::NotFoundError("");
}

absl::StatusOr<RoutingGrid::VertexWithLayer>
RoutingGrid::AddAccessVerticesForPoint(const geometry::Point &point,
                                       const geometry::Layer &layer) {
  // Add each of the possible on-grid access vertices for a given off-grid
  // point to the RoutingGrid. For example, given an arbitrary point O, we must
  // find the four nearest on-grid points A, B, C, D:
  //
  //        (A)
  //     X   +       X           X
  //      (2)| <-(1)
  //  (B)+---O-------+
  //         |  ^   (D)
  //         |  (4)
  //      (3)|
  //     X   +       X           X
  //        (C)
  //
  // If O lands on a grid column and/or row, we do not need to find a bridging
  // vertex on that column/row.
  //
  // For each vertex (A, B, C, D) we create, we also have to add a bridging edge
  // to the off-grid vertex (1, 2, 3, 4, respectively).
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      layer_access = physical_db_.FindReachableLayersByPinLayer(layer);

  LOG_IF(WARNING, layer_access.empty())
      << "Pin layer access was empty; is this a pin layer? " << layer;
  if (layer_access.empty()) {
    // If the given layer does not provide access to other layers, use the layer itself.
    layer_access.push_back({layer, {layer}});

    // TODO(aryap): More generally use any layer that we can reach with a via?
    //layer_access = physical_db_.FindLayersReachableThroughOneViaFrom(layer);
  }

  struct AccessOption {
    RoutingGridGeometry *grid_geometry;
    geometry::Layer target_layer;
    geometry::Layer access_layer;
    double total_via_cost;
  };

  std::vector<AccessOption> access_options;

  // Find usable RoutingGridGeometries (grids):
  for (const auto &entry : layer_access) {
    const geometry::Layer &target_layer = entry.first;
    for (const geometry::Layer &access_layer : entry.second) {
      std::vector<RoutingGridGeometry*> layer_grid_geometries =
          FindRoutingGridGeometriesUsingLayer(access_layer);

      auto cost = FindViaStackCost(target_layer, access_layer);
      if (!cost)
        // No via stack.
        continue;

      for (RoutingGridGeometry *grid_geometry : layer_grid_geometries) {
        access_options.push_back(AccessOption {
            .grid_geometry = grid_geometry,
            .target_layer = target_layer,
            .access_layer = access_layer,
            .total_via_cost = *cost});
      }
    }
  }

  auto comparator = [](const AccessOption &lhs, const AccessOption &rhs) {
    return lhs.total_via_cost < rhs.total_via_cost;
  };
  std::sort(access_options.begin(), access_options.end(), comparator);

  // Now that our options are sorted by the via cost they would incur, iterate
  // in increasing cost order until one of the options can accommodate the
  // target point.
  for (AccessOption &option : access_options) {
    const geometry::Layer &target_layer = option.target_layer;
    const geometry::Layer &access_layer = option.access_layer;
    RoutingGridGeometry *grid_geometry = option.grid_geometry;

    LOG(INFO) << "Access to " << point << " (layer " << target_layer
              << ") from layer " << access_layer
              << " possible through grid geometry " << grid_geometry
              << " with via cost " << option.total_via_cost;

    // FIXME: Should check if off_grid position is an existing on-grid vertex!
    RoutingVertex *existing = grid_geometry->VertexAt(point);
    if (existing) {
      return {{existing, target_layer}};
    }

    std::unique_ptr<RoutingVertex> off_grid(new RoutingVertex(point));
    off_grid->AddConnectedLayer(target_layer);
    off_grid->AddConnectedLayer(access_layer);

    std::set<RoutingTrackDirection> access_directions = {
        RoutingTrackDirection::kTrackHorizontal,
        RoutingTrackDirection::kTrackVertical};
    for (const auto &direction : access_directions) {
      if (!ValidAgainstKnownBlockages(*off_grid, direction) ||
          !ValidAgainstInstalledPaths(*off_grid, direction)) {
        access_directions.erase(direction);
      }
    }
    if (access_directions.empty()) {
      VLOG(15) << "Invalid off grid candidate at " << off_grid->centre();
      continue;
    }

    if (!ConnectToSurroundingTracks(
          *grid_geometry, access_layer, access_directions, off_grid.get()).ok()) {
      // TODO(aryap): Accumulate errors?
      // The off-grid vertex could not be connected to any surrounding tracks.
      continue;
    }

    RoutingVertex *vertex = off_grid.release();
    AddVertex(vertex);
    return {{vertex, target_layer}};
  }

  return absl::NotFoundError("");
}

absl::StatusOr<RoutingGrid::VertexWithLayer>
RoutingGrid::ConnectToNearestAvailableVertex(const geometry::Port &port) {
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      layer_access = physical_db_.FindReachableLayersByPinLayer(port.layer());
  for (const auto &entry : layer_access) {
    for (const geometry::Layer &layer : entry.second) {
      LOG(INFO) << "checking for grid vertex on layer " << layer;
      auto vertex = ConnectToNearestAvailableVertex(port.centre(), layer);
      if (vertex.ok()) {
        return {VertexWithLayer{.vertex = *vertex, .layer = layer}};
      }
      return vertex.status();
    }
  }
  return absl::NotFoundError(
      absl::StrCat("Couldn't find nearest available vertex for port ",
                   port.Describe()));
}

absl::StatusOr<RoutingVertex*> RoutingGrid::ConnectToNearestAvailableVertex(
    const geometry::Point &point, const geometry::Layer &layer) {
  // If constrained to one or two layers on a fixed grid, we can determine the
  // nearest vertices quickly by shortlisting those vertices whose positions
  // would correspond to the given point by construction (since we also
  // construct the grid).
  //
  // The more general solution, of finding the nearest vertex across any number
  // of layers, requires us to sort all available vertices by their proximity
  // to the position. This can be quite expensive. Also, there remains the
  // question of whether the vertex we find can be routed to.
  //
  // We have to check for available vertices on all the layers we might be able
  // to route to (i.e. insert a via to), and pick the cheapest.
  //
  // The first cut of this algorithm is to just find the closest of all the
  // available vertices on the given layer.

  // We need a copy to manipulate the layer:
  geometry::Point target_point = point;
  target_point.set_layer(layer);

  std::vector<CostedVertex> costed_vertices;
  for (const auto &entry : available_vertices_by_layer_) {
    // Is this layer reachable from the target?
    std::optional<std::pair<geometry::Layer, double>> needs_via;
    if (entry.first != layer) {
      needs_via = ViaLayerAndCost(entry.first, layer);
      if (!needs_via)
        continue;
      LOG(INFO) << "layer " << physical_db_.DescribeLayer(layer)
                << " is accessible for routing via layer "
                << physical_db_.DescribeLayer(needs_via->first);
    }

    for (RoutingVertex *vertex : entry.second) {
      // Do not consider unavailable vertices!
      if (!vertex->available())
        continue;
      uint64_t vertex_cost = static_cast<uint64_t>(
          vertex->L1DistanceTo(target_point));
      if (needs_via) {
        // FIXME(aryap): Use via cost!
        vertex_cost += (10.0 * needs_via->second);
      }
      costed_vertices.emplace_back(CostedVertex {
          .cost = vertex_cost,
          .layer = entry.first,
          .vertex = vertex
      });
    }
  }

  // Should sort automatically based on operator< for first and second entries
  // in pairs.
  //
  // This inequality goes the wrong way because we pop from the back of the
  // vector, not the front, and that's where we want the lowest-cost elements to
  // end up.
  static auto comp = [](const CostedVertex &lhs,
                        const CostedVertex &rhs) {
    return lhs.cost > rhs.cost;
  };
  std::sort(costed_vertices.begin(), costed_vertices.end(), comp);

  // To ensure we can go the "last mile", we check if the required paths, as
  // projected on the tracks on which the nearest vertex lies, are legal.
  // Consider 4 vertices X on the RoutingGrid surrounding the port O.
  //
  //     (A)
  //    X---+       X           X
  // (B)|   |(B')
  //    +---O
  //     (A')
  //
  //
  //    X           X           X
  //
  // To access O we must go off-grid and beat a path on the layer closest to
  // it. We should not need to hop between horizontal/vertical track layers
  // unless a direct path on a single layer is blocked.
  //
  // In the diagram, (A) and (B) are the bridging edges, and (A') and (B') are
  // the off-grid edges.
  //
  // We have to check each possible path {(A), (B')} and {(B), (A')} for each
  // of the vertices. If we can't establish the path on the cloest layer alone
  // we might have to hop between them - in practice however this is unlikely
  // to work since grid spacing won't allow for two vias so close.
  //
  // We generate a new RoutingVertex for the landing spot on each track and
  // provide that to the grid-router to use in finding a shortest path.
  //
  // TODO(aryap): A better idea is to create up to, say, 4 candidate bridging
  // vertices on legal tracks around the point so that they can be included in
  // the global shortest-path search. This would avoid having to turn corners
  // and go backwards, for example.
  while (!costed_vertices.empty()) {
    RoutingVertex *candidate = costed_vertices.back().vertex;
    const geometry::Layer vertex_layer = costed_vertices.back().layer;
    costed_vertices.pop_back();

    VLOG(10) << "Searching "  << costed_vertices.size() << " vertex "
             << candidate << " centre " << candidate->centre()
             << " layer " << vertex_layer
             << " cost " << costed_vertices.back().cost;

    if (candidate->vertical_track() == nullptr) {
      // FIXME(aryap): Is this a problem?
      VLOG(10) << "Cannot use vertex " << candidate
               << " as candidate because vertical track is nullptr";
      continue;
    } else if (candidate->horizontal_track() == nullptr) {
      VLOG(10) << "Cannot use vertex " << candidate
               << " as candidate because horizontal track is nullptr";
      continue;
    }

    std::unique_ptr<RoutingVertex> off_grid(new RoutingVertex(target_point));
    off_grid->AddConnectedLayer(vertex_layer);
    // FIXME: This function needs to allow collisions for same-net shapes!
    // FIXME: Need to check if RoutingVertex and RoutingEdges we create off grid
    // go too close to in-use edges and vertices!
    if (!ValidAgainstKnownBlockages(*off_grid) ||
        !ValidAgainstInstalledPaths(*off_grid)) {
      VLOG(15) << "Invalid off grid candidate at " << off_grid->centre();
      continue;
    }

    // Try putting it on the vertical track and then horizontal track.
    std::vector<RoutingTrack*> tracks = candidate->Tracks();
    RoutingVertex *bridging_vertex = nullptr;
    bool bridging_vertex_is_new = false;

    bool success = false;
    for (size_t i = 0; i < tracks.size(); ++i) {
      bridging_vertex = nullptr;
      bool off_grid_already_exists = false;
      success = tracks[i]->CreateNearestVertexAndConnect(
          *this,
          off_grid.get(),
          vertex_layer,
          &bridging_vertex,
          &bridging_vertex_is_new,
          &off_grid_already_exists);

      if (!success) {
        continue;
      } else if (off_grid_already_exists) {
        if (bridging_vertex_is_new) {
          std::stringstream ss;
          ss << "Doesn't make sense for off_grid_already_exists and "
             << "bridging_vertex_is_new to both be true";
          return absl::InternalError(ss.str());
        }
        // We're done! We can just use an existing vertex since 'off_grid'
        // happens to already exist. 'off_grid' should NOT be added to the
        // routing grid, it should be discarded.
        return bridging_vertex;
      }
      break;
    }

    if (!success) {
      continue;
    }

    // Add off_grid now that we have a viable bridging_vertex.
    RoutingVertex *off_grid_copy = off_grid.get();
    AddVertex(off_grid.release());

    if (bridging_vertex == off_grid_copy) {
      // off_grid landed on the track and was subsumed and connected, we have
      // nothing left to do.
      return off_grid_copy;
    }

    // TODO(aryap): Need a way to roll back these temporary objects in case the
    // caller's entire process fails - i.e. a vertex can be created for the
    // starting point but not for the ending point.

    if (bridging_vertex_is_new) {
      // If the bridging_vertex was an existing vertex on the track, we don't
      // need to add it.
      AddVertex(bridging_vertex);
    }

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid_copy);
    edge->set_layer(vertex_layer);
    if (!ValidAgainstKnownBlockages(*edge) ||
        !ValidAgainstInstalledPaths(*edge)) {
      VLOG(15) << "Invalid off grid edge between " << bridging_vertex->centre()
               << " and " << off_grid_copy->centre();
      // Rollback extra hard!
      if (bridging_vertex_is_new) {
        RemoveVertex(bridging_vertex, true);  // and delete!
      }
      RemoveVertex(off_grid_copy, true);  // and delete!
      delete edge;    
      continue;
    }
    LOG(INFO) << "Connected new vertex " << bridging_vertex->centre()
              << " on layer " << edge->EffectiveLayer();
    bridging_vertex->AddEdge(edge);
    off_grid_copy->AddEdge(edge);
    
    // TODO(aryap): It's unclear what layer this edge is on. The opposite of
    // what the bridging edge is on, I guess.
    // TODO(aryap): It's not clear if the off-grid edge will be legal. We have
    // to check with the whole grid.

    off_grid_edges_.insert(edge);
    return off_grid_copy;
  }
  return nullptr;
}

std::optional<geometry::Rectangle> RoutingGrid::ViaFootprint(
    const geometry::Point &centre,
    const geometry::Layer &other_layer,
    const geometry::Layer &footprint_layer,
    int64_t padding,
    std::optional<RoutingTrackDirection> direction) const {
  if (footprint_layer == other_layer) {
    return std::nullopt;
  }
  // Get the applicable via info for via sizing and encapsulation values:
  const RoutingViaInfo &routing_via_info = GetRoutingViaInfoOrDie(
      footprint_layer, other_layer);
  int64_t via_width =
      routing_via_info.EncapWidth(footprint_layer) + 2 * padding;
  int64_t via_length =
      routing_via_info.EncapLength(footprint_layer) + 2 * padding;

  geometry::Point lower_left;

  if (!direction) {
    int64_t square_width = std::max(via_width, via_length);
    lower_left = centre - geometry::Point(square_width / 2, square_width / 2);
    return geometry::Rectangle(lower_left, square_width, square_width);
  }

  switch (*direction) {
    case RoutingTrackDirection::kTrackVertical:
      lower_left = centre - geometry::Point(via_width / 2, via_length / 2);
      return geometry::Rectangle(lower_left, via_width, via_length);
    case RoutingTrackDirection::kTrackHorizontal:
      lower_left = centre - geometry::Point(via_length / 2, via_width / 2);
      return geometry::Rectangle(lower_left, via_length, via_width);
    default:
      LOG(FATAL) << "Unknown RoutingTrackDirection: " << *direction;
  }
  return std::nullopt;
}

std::optional<geometry::Rectangle> RoutingGrid::ViaFootprint(
    const RoutingVertex &vertex,
    const geometry::Layer &footprint_layer,
    int64_t padding,
    std::optional<RoutingTrackDirection> direction) const {
  std::set<geometry::Layer> vertex_layers = vertex.connected_layers();

  // We expect footprint_layer to appear in the vertex's list of connected
  // layers.
  vertex_layers.erase(footprint_layer);

  // If there is more than 1 layer left in the connected layer list, we have a
  // problem because we assume that the vertex connects to at most 2 layers.
  // That shouldn't happen. There should be 0 or 1.
  if (vertex_layers.size() != 1)
    return std::nullopt;

  const geometry::Layer &other_layer = *vertex_layers.begin();

  return ViaFootprint(
      vertex.centre(), other_layer, footprint_layer, padding, direction);

  //const std::vector<geometry::Layer> &layers = vertex.connected_layers();
  //if (layers.size() != 2) {
  //  return std::nullopt;
  //}
  //const geometry::Layer &first_layer = layers.front();
  //const geometry::Layer &second_layer = layers.back();

  //return ViaFootprint(
  //    vertex.centre(), first_layer, second_layer, padding, direction);
}

std::optional<geometry::Rectangle> RoutingGrid::TrackFootprint(
    const RoutingEdge &edge, int64_t padding) const {
  const geometry::Layer &layer = edge.EffectiveLayer();
  const RoutingLayerInfo &layer_info = GetRoutingLayerInfoOrDie(layer);
  auto edge_as_rectangle = edge.AsRectangle(layer_info.wire_width);
  if (!edge_as_rectangle)
    return std::nullopt;
  if (padding == 0)
    return edge_as_rectangle;
  geometry::Rectangle &rectangle = edge_as_rectangle.value();
  return rectangle.WithPadding(padding);
}

// TODO(aryap): It's convenient to have an edge generate the footprint it would
// imply as a wire, complete with vias at either end for the vertices (with
// appropriate metal encap sizes given the layers the vertices connect). That
// would require RoutingEdge to have knowledge of physical constraints or at
// least the RoutingGrid, since we hide behind that abstraction in Routing
// stuff. And that would require RoutingGrid to know a lot about RoutingEdge and
// also the opposite, which is annoying and not very clean as an abstraction.
//
// TODO(aryap): Memoise this: we only need to generate the footprint once for
// many checks.
std::optional<geometry::Rectangle> RoutingGrid::EdgeFootprint(
    const RoutingEdge &edge, int64_t padding) const {
  if (!edge.layer()) {
    LOG(WARNING) << "Edge cannot be turned into rectangle without layer_ set";
    return std::nullopt;
  }

  const geometry::Layer &layer = *edge.layer();
  int64_t width = GetRoutingLayerInfoOrDie(layer).wire_width + padding;

  // A rectangle of wire-width without via encaps at either end represents the
  // middle section of the edge.
  auto wire_only_bounds = edge.AsRectangle(width);
  if (!wire_only_bounds) {
    LOG(FATAL) << "Edge does not have simple rectangle form!";
    return std::nullopt;
  }
  
  // Sort [first, second] vertices into bottom-left-most to upper-right-most:
  std::vector<RoutingVertex*> vertices = {edge.first(), edge.second()};
  std::sort(
      vertices.begin(),
      vertices.end(),
      [](RoutingVertex *lhs, RoutingVertex *rhs) {
        const geometry::Point &left_point = lhs->centre();
        const geometry::Point &right_point = rhs->centre();
        if (left_point.x() == right_point.x()) {
          return left_point.y() < right_point.y();
        }
        return left_point.x() < right_point.x();
      }
  );

  const RoutingVertex *lower_left = vertices.front();
  const RoutingVertex *upper_right = vertices.back();

  auto lower_left_footprint = ViaFootprint(
      *lower_left, layer, padding, edge.Direction());
  auto upper_right_footprint = ViaFootprint(
      *upper_right, layer, padding, edge.Direction());

  std::vector<geometry::Point> lower_left_options = {
    wire_only_bounds->lower_left()
  };
  std::vector<geometry::Point> upper_right_options = {
    wire_only_bounds->upper_right()
  };
  if (lower_left_footprint) {
    lower_left_options.push_back(lower_left_footprint->lower_left());
    upper_right_options.push_back(lower_left_footprint->upper_right());
  }
  if (upper_right_footprint) {
    lower_left_options.push_back(upper_right_footprint->lower_left());
    upper_right_options.push_back(upper_right_footprint->upper_right());
  }

  // TODO(aryap): A good exercise! Define a function on geometry::Point that
  // will accept either any iterator of Points or any container of Points to
  // automatically return the (min, max) x (x, y) over a range.
  geometry::Point lower_left_point = {
      std::min_element(lower_left_options.begin(),
                       lower_left_options.end(),
                       geometry::Point::CompareX)->x(),
      std::min_element(lower_left_options.begin(),
                       lower_left_options.end(),
                       geometry::Point::CompareY)->y()
  };
  geometry::Point upper_right_point = {
      std::max_element(upper_right_options.begin(),
                       upper_right_options.end(),
                       geometry::Point::CompareX)->x(),
      std::max_element(upper_right_options.begin(),
                       upper_right_options.end(),
                       geometry::Point::CompareY)->y()
  };

  return {{lower_left_point, upper_right_point}};
}

std::vector<RoutingVertex*> &RoutingGrid::GetAvailableVertices(
    const geometry::Layer &layer) {
  auto it = available_vertices_by_layer_.find(layer);
  if (it == available_vertices_by_layer_.end()) {
    auto insert_it = available_vertices_by_layer_.insert({layer, {}});
    LOG_IF(FATAL, !insert_it.second)
        << "Couldn't create entry for layer " << layer
        << " in available vertices map.";
    it = insert_it.first;
  }
  // Gotta dereference the iterator to get to the goods!
  return it->second;
}

std::optional<std::pair<geometry::Layer, double>> RoutingGrid::ViaLayerAndCost(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  if (lhs == rhs)
    return std::nullopt;

  std::optional<std::reference_wrapper<const RoutingViaInfo>> needs_via =
      GetRoutingViaInfo(lhs, rhs);
  if (!needs_via) {
    return std::nullopt;
  }
  return std::make_pair(needs_via->get().layer(), needs_via->get().cost());
}

bool RoutingGrid::ConnectLayers(
    const geometry::Layer &first, const geometry::Layer &second) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  const RoutingLayerInfo &horizontal_info = split_directions.first;
  const RoutingLayerInfo &vertical_info = split_directions.second;

  auto maybe_routing_via_info = GetRoutingViaInfo(first, second);
  if (!maybe_routing_via_info) {
    LOG(ERROR) << "Could not get RoutingViaInfo for " << first
               << ", " << second;
    return false;
  }
  const RoutingViaInfo &routing_via_info = maybe_routing_via_info->get();

  LOG(INFO) << "Drawing grid between layers " << horizontal_info.layer
            << ", " << vertical_info.layer;

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal_info, vertical_info);

  size_t num_vertices = 0;
  size_t num_x = 0;
  size_t num_y = 0;

  std::map<int64_t, RoutingTrack*> vertical_tracks;
  std::map<int64_t, RoutingTrack*> horizontal_tracks;

  // Generate tracks to hold edges and vertices in each direction.
  for (int64_t x = grid_geometry.x_start();
       x <= grid_geometry.x_max();
       x += grid_geometry.x_pitch()) {
    RoutingTrack *track = new RoutingTrack(
        vertical_info.layer,
        RoutingTrackDirection::kTrackVertical,
        grid_geometry.x_pitch(),
        vertical_info.wire_width,
        routing_via_info.EncapWidth(vertical_info.layer),
        routing_via_info.EncapLength(vertical_info.layer),
        vertical_info.min_separation,
        x);
    vertical_tracks.insert({x, track});
    grid_geometry.vertical_tracks_by_index().push_back(track);
    AddTrackToLayer(track, vertical_info.layer);
    num_x++;
  }

  for (int64_t y = grid_geometry.y_start();
       y <= grid_geometry.y_max();
       y += grid_geometry.y_pitch()) {
    RoutingTrack *track = new RoutingTrack(
        horizontal_info.layer,
        RoutingTrackDirection::kTrackHorizontal,
        grid_geometry.y_pitch(),
        horizontal_info.wire_width,
        routing_via_info.EncapWidth(horizontal_info.layer),
        routing_via_info.EncapLength(horizontal_info.layer),
        horizontal_info.min_separation,
        y);
    horizontal_tracks.insert({y, track});
    grid_geometry.horizontal_tracks_by_index().push_back(track);
    AddTrackToLayer(track, horizontal_info.layer);
    num_y++;
  }

  std::vector<std::vector<RoutingVertex*>> &vertices =
      grid_geometry.vertices_by_grid_position();

  // Generate a vertex at the intersection of every horizontal and vertical
  // track.
  size_t i = 0;
  for (int64_t x = grid_geometry.x_start();
       x <= grid_geometry.x_max();
       x += grid_geometry.x_pitch()) {
    // This (and the horizontal one) must exist by now, so we can make this
    // fatal.
    RoutingTrack *vertical_track = vertical_tracks.find(x)->second;
    LOG_IF(FATAL, !vertical_track) << "Vertical routing track is nullptr";

    size_t j = 0;
    for (int64_t y = grid_geometry.y_start();
         y <= grid_geometry.y_max();
         y += grid_geometry.y_pitch()) {
      RoutingTrack *horizontal_track = horizontal_tracks.find(y)->second;
      LOG_IF(FATAL, !horizontal_track) << "Horizontal routing track is nullptr";

      RoutingVertex *vertex = new RoutingVertex(geometry::Point(x, y));
      vertex->set_horizontal_track(horizontal_track);
      vertex->set_vertical_track(vertical_track);

      horizontal_track->AddVertex(vertex);
      vertical_track->AddVertex(vertex);

      ++num_vertices;
      vertex->AddConnectedLayer(first);
      vertex->AddConnectedLayer(second);

      AddVertex(vertex);

      VLOG(20) << "Vertex created: " << vertex->centre() << " on layers: "
               << absl::StrJoin(vertex->connected_layers(), ", ");

      vertex->set_grid_position_x(i);
      vertex->set_grid_position_y(j);

      vertices[i][j] = vertex;

      // Assign neighbours. Since we do the reciprocal relationship too, we
      // assign up to all 8 neighbours per iteration.
      if (i > 0) {
        // Left neighbour.
        RoutingVertex *neighbour = vertices[i - 1][j];
        vertex->AddNeighbour(Compass::LEFT, neighbour);
        neighbour->AddNeighbour(Compass::RIGHT, vertex);

        if (j > 0) {
          // Lower left neighbour.
          RoutingVertex *neighbour = vertices[i - 1][j - 1];
          vertex->AddNeighbour(Compass::LOWER_LEFT, neighbour);
          neighbour->AddNeighbour(Compass::UPPER_RIGHT, vertex);
        }

        if (j < vertices[i].size() - 1) {
          // Upper left neighbour.
          RoutingVertex *neighbour = vertices[i - 1][j + 1];
          vertex->AddNeighbour(Compass::UPPER_LEFT, neighbour);
          neighbour->AddNeighbour(Compass::LOWER_RIGHT, vertex);
        }
      }
      if (j > 0) {
        // Lower neighbour.
        RoutingVertex *neighbour = vertices[i][j - 1];
        vertex->AddNeighbour(Compass::LOWER, neighbour);
        neighbour->AddNeighbour(Compass::UPPER, vertex);
      }
      j++;
    }
    i++;
  }

  // This adds a copy of the object to our class's bookkeeping. It's kinda
  // annoying. I'd rather create it on the fly to avoid the copy.
  // TODO(aryap): Avoid this copy.
  AddRoutingGridGeometry(first, second, grid_geometry);

  size_t num_edges = 0;
  for (auto entry : tracks_by_layer_)
    for (RoutingTrack *track : entry.second)
      num_edges += track->edges().size();

  LOG(INFO) << "Connected layer " << first << " and " << second << "; "
            << "generated " << horizontal_tracks.size() << " horizontal and "
            << vertical_tracks.size() << " vertical tracks, "
            << num_vertices << " vertices and "
            << num_edges << " edges.";

  for (auto entry : tracks_by_layer_) {
    const geometry::Layer &layer = entry.first;
    for (RoutingTrack *track : entry.second) {
      VLOG(10) << layer << " track: " << *track;
    }
  }

  return true;
}

bool RoutingGrid::ContainsVertex(RoutingVertex *vertex) const {
  // Did you know? std::begin(...) and std::end(...) were introduced in C++11
  // and have more general compatibility, such as with old C-style arrays.
  return std::find(
      vertices_.begin(), vertices_.end(), vertex) != vertices_.end();
}

void RoutingGrid::AddVertex(RoutingVertex *vertex) {
  for (const geometry::Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingVertex*> &available = GetAvailableVertices(layer);
    available.push_back(vertex);
    // LOG(INFO) << "available (" << layer << "): " << available.size();
  }
  DCHECK(!ContainsVertex(vertex));
  vertices_.push_back(vertex);  // The class owns all of these.
}

absl::Status RoutingGrid::AddRouteBetween(
    const geometry::Port &begin,
    const geometry::Port &end,
    const std::set<geometry::Port*> &avoid,
    const std::string &net) {
  // Override the vertex availability check for this search to avoid
  // obstructions in the given avoid set. Useful since it doesn't mutate the
  // global starting state for the purpose of the search, but we have to
  // serialise searches to ensure determinism anyway, making it kind of
  // a pointless feature. It's also noticeably slower, since now a boolean check
  // per graph entity is replaced by a container lookup.
  //
  // Instead, we could temporarily mark the effected vertices unavailable and
  // reset them after.
  // TODO(aryap): That doesn't work today because RoutingTrack has a very
  // complicated way of managing, sorting and merging blockages, and temporary
  // blockages would need to be freshly supported. Probably not worth the
  // complexity just to avoid temporarily blockages like pins.
  //auto usable_vertex = [&](RoutingVertex *vertex) {
  //  if (!vertex->available()) return false;

  //  std::optional<geometry::Rectangle> via_footprint = ViaFootprint(
  //      *vertex,
  //      100);   // FIXME(aryap): This padding value is the max of the
  //              // min_separations on the top and bottom layers and should just
  //              // be automatically found.
  //  if (!via_footprint)
  //    return true;

  //  for (geometry::Port *port : avoid) {
  //    if (port->Overlaps(via_footprint.value()))
  //      return false;
  //  }
  //  return true;
  //};
  TemporaryBlockageInfo temporary_blockages;
  SetUpTemporaryBlockages(avoid, &temporary_blockages);

  absl::Cleanup tear_down_temporary_blockages = [&]() {
    TearDownTemporaryBlockages(temporary_blockages);
  };

  auto begin_connection = ConnectToGrid(begin);
  if (!begin_connection.ok()) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    return absl::NotFoundError(
        "Could not find available vertex for begin port.");
  }
  RoutingVertex *begin_vertex = begin_connection->vertex;
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  auto end_connection = ConnectToGrid(end);
  if (!end_connection.ok()) {
    LOG(ERROR) << "Could not find available vertex for end port.";
    return absl::NotFoundError("Could not find available vertex for end port.");
  }
  RoutingVertex *end_vertex = end_connection->vertex;
  LOG(INFO) << "Nearest vertex to end (" << end << ") is "
            << end_vertex->centre();

  auto shortest_path_result = ShortestPath(begin_vertex, end_vertex);
  if (!shortest_path_result.ok()) {
    std::string message = absl::StrCat(
        "No path found: ", shortest_path_result.status().message());
    LOG(WARNING) << message;
    return absl::NotFoundError(message);
  }
  std::unique_ptr<RoutingPath> shortest_path(*shortest_path_result);

  // Remember the ports to which the path should connect.
  //
  // Ports are typically on port layers (i.e. PINs), but for convenience we also
  // record the layer we need to use to access said ports:
  shortest_path->set_start_port(&begin);
  shortest_path->start_access_layers().insert(begin_connection->layer);
  shortest_path->set_end_port(&end);
  shortest_path->end_access_layers().insert(end_connection->layer);

  LOG(INFO) << "Found path: " << *shortest_path;

  // Once the path is found, but _before_ it is installed, the temporarily
  // blocked nodes should be re-enabled. This might be permanently blocked by
  // installing the path finally!
  std::move(tear_down_temporary_blockages).Invoke();

  // Assign net and install:
  if (!net.empty())
    shortest_path->set_net(net);

  absl::Status install = InstallPath(shortest_path.release());
  return install;
}

namespace {

std::set<geometry::Layer> EffectiveLayersForInstalledVertex(
    RoutingVertex *vertex) {
  std::set<geometry::Layer> layers;
  for (auto &entry : vertex->installed_in_paths()) {
    for (RoutingEdge *edge : entry.second) {
      layers.insert(edge->EffectiveLayer());
    }
  }
  return layers;
}

}   // namespace

absl::Status RoutingGrid::AddRouteToNet(
    const geometry::Port &begin,
    const std::string &net,
    const std::set<geometry::Port*> &avoid) {
  TemporaryBlockageInfo temporary_blockages;
  SetUpTemporaryBlockages(avoid, &temporary_blockages);

  auto begin_connection = ConnectToGrid(begin);
  if (!begin_connection.ok()) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    TearDownTemporaryBlockages(temporary_blockages);
    return absl::NotFoundError(
        "Could not find available vertex for begin port.");
  }
  RoutingVertex *begin_vertex = begin_connection->vertex;
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  RoutingVertex *end_vertex;

  auto shortest_path_result = ShortestPath(begin_vertex, net, &end_vertex);
  if (!shortest_path_result.ok()) {
    std::string message = absl::StrCat("No path found to net ", net, ".");
    LOG(WARNING) << message;
    TearDownTemporaryBlockages(temporary_blockages);
    return absl::NotFoundError(message);
  }

  // Claim the pointer.
  std::unique_ptr<RoutingPath> shortest_path(*shortest_path_result);

  // Remember the ports to which the path should connect.
  shortest_path->set_start_port(&begin);
  shortest_path->start_access_layers().insert(begin_connection->layer);

  // We expect that we now have a path terminating in a vertex that is attached
  // to the given net.
  //
  // We can assume that the vertex attaches to the net on one of its
  // connectable_layers_, but which one is the best to use depends on which path
  // the vertex is installed in.
  //
  // Because we haven't called InstallPath yet, vertices have not been assigned
  // permanent in/out_edge() values.
  std::set<geometry::Layer> end_layers = EffectiveLayersForInstalledVertex(
      end_vertex);
  shortest_path->end_access_layers().insert(
      end_layers.begin(), end_layers.end());

  LOG(INFO) << "Found path: " << *shortest_path;

  // Assign net and install:
  shortest_path->set_net(net);

  TearDownTemporaryBlockages(temporary_blockages);

  absl::Status install = InstallPath(shortest_path.release());
  return install;
}

bool RoutingGrid::RemoveVertex(RoutingVertex *vertex, bool and_delete) {
  if (vertex->horizontal_track())
    vertex->horizontal_track()->RemoveVertex(vertex);
  if (vertex->vertical_track())
    vertex->vertical_track()->RemoveVertex(vertex);

  // Check for instances of this vertex in off-grid edges:
  for (auto it = off_grid_edges_.begin(); it != off_grid_edges_.end();) {
    RoutingEdge *edge = *it;
    if (edge->first() == vertex || edge->second() == vertex) {
      VLOG(10) << "Removing off-grid edge " << edge
               << " because it includes vertex " << vertex;
      edge->PrepareForRemoval();
      delete edge;
      it = off_grid_edges_.erase(it);
    } else {
      ++it;
    }
  }

  for (const geometry::Layer &layer : vertex->connected_layers()) {
    auto it = available_vertices_by_layer_.find(layer);
    if (it == available_vertices_by_layer_.end())
      continue;
    auto &available_vertices = it->second;
    auto pos = std::find(
        available_vertices.begin(), available_vertices.end(), vertex);
    if (pos == available_vertices.end()) {
      // Already removed from availability list.
      continue;
    }
    available_vertices.erase(pos);
  }

  auto pos = std::find(vertices_.begin(), vertices_.end(), vertex);
  LOG_IF(WARNING, pos == vertices_.end())
      << "Did not find vertex we're removing in RoutingGrid list of "
      << "vertices: " << vertex;
  vertices_.erase(pos);
  if (and_delete)
    delete vertex;
  return true; // TODO(aryap): Always returning true, huh...
}

// Disable neighbouring vertices now that this one is in use. The implication of
// a vertex here is that a via will be placed at vertex->centre(), which means
// that none of the surrounding vertices can be used for vias.
//
// We do this even though the vias might get "optimised out".
void RoutingGrid::InstallVertexInPath(RoutingVertex *vertex) {
  if (vertex->horizontal_track() && vertex->vertical_track()) {
    // If the vertex is on the grid, we only disable the recorded neighbours.
    // We maybe could get await without adding blockages to their tracks as well
    // because these neighbours are, by virtue of being on the grid, spaced
    // appropriately to accommodate a via and a wire track next to each other -
    // however, we rely on these blockages to determine appropriate connection
    // points for new off-grid vertices, so we should add the blockages anyway.
    //
    // NOTE(aryap): Nearby bridging vertices will not be disabled, be will be
    // unusable anyway (they are created to access a single off-grid point)
    // so this optimisation is ok.
    //
    // TODO(aryap): Determine if the junction vertices should have blockages
    // added where they are on-grid in InstallVertexInPath.
    static const std::vector<Compass> kDisabledNeighbours = {
      Compass::UPPER_LEFT,
      Compass::UPPER,
      Compass::UPPER_RIGHT,
      Compass::LEFT,
      Compass::RIGHT,
      Compass::LOWER_LEFT,
      Compass::LOWER,
      Compass::LOWER_RIGHT,
    };
    for (const auto &position : kDisabledNeighbours) {
      std::set<RoutingVertex*> neighbours = vertex->GetNeighbours(position);
      for (RoutingVertex *neighbour : neighbours) {
        if (neighbour->available() ) {
          neighbour->set_available(false);
          neighbour->set_connectable_net(vertex->net());
        } else if (neighbour->connectable_net() &&
                   *neighbour->connectable_net() != vertex->net()) {
          // If the neighbour is flagged as usable for a different net, disable
          // that.
          neighbour->set_connectable_net(std::nullopt);
        }
      }
    };
    return;
  }
  // If the vertex is off-grid, we have to search for affected neighbours
  // more painstakingly.
  //
  // There are two different sets of surrounding vertices on the grid:
  //
  //      |E     |I     |K     |M
  //  ----x------x------x------x-----
  //      |      |      |      |
  //      |F     |A     |C     |N
  //  ----x------x------x------x-----
  //      |      |  x <------------------ Z, off-grid point
  //      |G     |B     |D     |O
  //  ----x------x------x------x-----
  //      |      |      |      |
  //      |H     |J     |L     |P
  //  ----x------x------x------x-----
  //      |      |      |      |
  //
  // The inner vertices surrounding Z (A, B, C, D) will definitely conflict, so
  // we don't bother to check the distance between a via at O and a via at any
  // of their positions. The outer vertices (E - P) will only conflict if the
  // via at Z is positioned in a certain way within the ABDC rectangle, so we
  // have to check those explicitly.
  std::set<RoutingVertex*> vertices;
  std::set<RoutingVertex*> inner_vertices;
  for (const geometry::Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingGridGeometry*> grid_geometries =
        FindRoutingGridGeometriesUsingLayer(layer);
    for (RoutingGridGeometry *grid_geometry : grid_geometries) {
      grid_geometry->EnvelopingVertices(
          vertex->centre(),
          &vertices,
          0,
          2);   // num_concentric_layers = 2 yields vertices A - P in the above
                // example.
      grid_geometry->EnvelopingVertices(
          vertex->centre(),
          &inner_vertices,
          0,
          1);   // num_concentric_layers = 1 yields vertices A - D.
    }
  }
  std::vector<RoutingVertex*> outer_vertices;
  std::set_difference(vertices.begin(), vertices.end(),
                      inner_vertices.begin(), inner_vertices.end(),
                      std::back_inserter(outer_vertices));

  std::set<RoutingTrack*> blocked_tracks;
  for (RoutingVertex *enveloping_vertex : inner_vertices) {
    enveloping_vertex->set_available(false);
    enveloping_vertex->set_connectable_net(vertex->net());
    // We also have to add blockages to the tracks on which these vertices
    // appear, since by being off-grid we're _presumably_ too close to
    // accomodate both a via and an edge next to each other.
    //
    // (We can explicitly check this if it is ever necessary.)
    //
    //      |A           |C
    //  ----x-----(1)----x-----
    //   (4)|            |
    //            O      |
    //      x-----x     (2)
    //      |B           |
    //      |            |
    //  --- x ----(3)----x-----
    //      |E           |D
    //
    //  B is too close to A. We project the via footprint at B onto tracks (1)
    //  and (3). Since B is on track (4), the blockage to track (4) will be
    //  taken care of elsewhere.
    //
    //  O is too close to A, C, D, B, E.  We project the via footprint onto
    //  tracks (1), (2), (3), (4), since O participates in none of those tracks.
    blocked_tracks.insert(enveloping_vertex->horizontal_track());
    blocked_tracks.insert(enveloping_vertex->vertical_track());
  }
  // This would happen anyway (because off-grid vertices have at least one
  // nullptr track), but we make it explicit:
  blocked_tracks.erase(nullptr);
  blocked_tracks.erase(vertex->horizontal_track());
  blocked_tracks.erase(vertex->vertical_track());

  for (const geometry::Layer &layer : vertex->connected_layers()) {
    // If there is an edge on this layer, we use its direction. Otherwise we use
    // the routing grid default direction for the layer.
    RoutingEdge *edge = vertex->GetEdgeOnLayer(layer);
    RoutingTrackDirection direction;
    if (edge) {
      direction = edge->Direction();
    } else {
      auto routing_layer_info = GetRoutingLayerInfo(layer);
      if (!routing_layer_info) {
        // No routing on this layer and no known direction, ignore.
        continue;
      }
      direction = routing_layer_info->get().direction;
    }

    std::optional<geometry::Rectangle> via_encap = ViaFootprint(
       *vertex, layer, 0, direction);
    if (!via_encap)
      continue;
    //LOG(INFO) << "via encap: " << *via_encap << " about " << vertex->centre()
    //          << " layer " << layer << " for edge " << *edge;
    for (RoutingTrack *track : blocked_tracks) {
      if (track->layer() != layer)
        continue;
      track->AddBlockage(*via_encap);
    }

    int64_t min_separation = physical_db_.Rules(layer).min_separation;
    for (RoutingVertex *enveloping_vertex : outer_vertices) {
      std::optional<geometry::Rectangle> outer_via_encap = ViaFootprint(
          *enveloping_vertex, layer, 0);
      int64_t min_distance = static_cast<int64_t>(std::ceil(
          via_encap->ClosestDistanceTo(*outer_via_encap)));
      if (min_distance < min_separation) {
        //LOG(INFO) << "vertex " << enveloping_vertex->centre()
        //          << " is too close (" << min_distance << " < "
        //          << min_separation << ") to "
        //          << *via_encap << " at " << vertex->centre();
        enveloping_vertex->set_available(false);
        enveloping_vertex->set_connectable_net(vertex->net());
      }
    }
  }
}

absl::Status RoutingGrid::InstallPath(RoutingPath *path) {
  if (path->Empty()) {
    return absl::InvalidArgumentError("Cannot install an empty path.");
  }

  LOG(INFO) << "Installing path " << *path << " with net " << path->net();

  // Legalise the path. TODO(aryap): This might modify the edges the path
  // contains, which smells funny.
  path->Legalise();

  // Mark edges as unavailable with track which owns them.
  for (RoutingEdge *edge : path->edges()) {
    if (edge->track() != nullptr) {
      edge->track()->MarkEdgeAsUsed(edge, path->net());
    } else {
      edge->set_in_use_by_net(path->net());
    }

    std::vector<RoutingVertex*> spanned_vertices = edge->SpannedVertices();
    for (RoutingVertex *vertex : spanned_vertices) {
      vertex->installed_in_paths()[path].insert(edge);
    }
  }

  if (path->vertices().size() != path->edges().size() + 1) {
    std::stringstream ss;
    ss << "Path vertices and edges mismatched. There are "
       << path->edges().size() << " edges and "
       << path->vertices().size() << " vertices";
    return absl::InvalidArgumentError(ss.str());
  }

  size_t i = 0;
  RoutingEdge *edge = nullptr;
  path->vertices()[0]->set_available(false);
  while (i < path->edges().size()) {
    RoutingVertex *last_vertex = path->vertices()[i];
    RoutingVertex *next_vertex = path->vertices()[i + 1];
    RoutingEdge *edge = path->edges()[i];
    last_vertex->set_out_edge(edge);
    next_vertex->set_in_edge(edge);
    next_vertex->set_available(false);
    next_vertex->set_net(path->net());
    ++i;
  }

  for (RoutingVertex *vertex : path->vertices()) {
    InstallVertexInPath(vertex);
  }
  
  paths_.push_back(path);
  return absl::OkStatus();
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin, RoutingVertex *end) {
  return ShortestPath(
      begin,
      [=](RoutingVertex *v) { return v == end; },   // The target.
      nullptr,
      [](RoutingVertex *v) { return v->available(); },
      [](RoutingEdge *e) { return e->Available(); },
      true);
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin,
    const std::string &to_net,
    RoutingVertex **discovered_target) {
  auto path = ShortestPath(
      begin,
      [&](RoutingVertex *v) {
        // Check that putting a via at this position doesn't conflict with vias
        // for other nets (since the encapsulating metal layers would
        // conflict):
        std::set<RoutingVertex*> neighbours = v->GetNeighbours();
        // ChangesEdge is a proxy for a vertex that might become a via.
        // NOTE: It's not the *same* as a vertex that might become a via, but
        // that isn't decided til RoutingPath has to export geometry :/
        for (RoutingVertex *neighbour : neighbours) {
          if (!neighbour->available() &&
              neighbour->ChangesEdge() &&
              neighbour->net() != to_net)
            return false;
        }
        return v->net() == to_net;
      },
      discovered_target,
      // Usable vertices are:
      [&](RoutingVertex *v) {
        return v->available() || (
            v->connectable_net() && *v->connectable_net() == to_net);
      },
      // Usable edges are:
      [&](RoutingEdge *e) {
        if (e->Available()) return true;
        if (e->blocked()) {
          VLOG(16) << "edge " << *e << " is blocked";
          return false;
        }
        if (e->in_use_by_net() && *e->in_use_by_net() == to_net) {
          return true;
        } else {
          VLOG(16) << "cannot use edge " << *e << " for net " << to_net;
        }
        return false;
      },
      false);   // Targets don't have to be 'usable', since we expect them to
                // already be used by the target net.
  // TODO(aryap): InstallPath obviates this.
  //if (path.ok()) {
  //  (*path)->set_encap_end_port(true);
  //}
  return path;
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin, 
    std::function<bool(RoutingVertex*)> is_target,
    RoutingVertex **discovered_target,
    std::function<bool(RoutingVertex*)> usable_vertex,
    std::function<bool(RoutingEdge*)> usable_edge,
    bool target_must_be_usable) {
  // FIXME(aryap): This is very bad.
  if (!usable_vertex(begin)) {
    return absl::NotFoundError("Start vertex for path is not available");
  }

  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  // Prefer consistent C++/STLisms over e.g. bool seen[vertices_.size()];
  std::vector<double> cost(vertices_.size());
  std::vector<bool> seen(vertices_.size());

  // Records the edges to follow backward to the start, forming the shortest
  // path. If RoutingEdge* is nullptr then this is invalid. The index into this
  // array is the index of the sink node, the entry gives the path back to the
  // source.
  // TODO(aryap): Apparently using pairs everywhere is bad style and I should
  // use structs:
  // https://google.github.io/styleguide/cppguide.html#Structs_vs._Classes
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  // We want the lowest value at the back of the array. But in a priority_queue,
  // we want the highest value at the start of the collection so that the
  // 'least' element is popped first (because that's how priority_queue works).
  auto vertex_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    return cost[a->contextual_index()] > cost[b->contextual_index()];
  };
  // All vertices sorted according to their cost.
  std::priority_queue<RoutingVertex*,
                      std::vector<RoutingVertex*>,
                      decltype(vertex_sort_fn)> queue(vertex_sort_fn);
  std::set<RoutingVertex*> found_targets;

  size_t begin_index = begin->contextual_index();

  cost[begin_index] = 0;

  for (size_t i = 0; i < vertices_.size(); ++i) {
    RoutingVertex *vertex = vertices_[i];
    if (i != vertex->contextual_index()) {
      std::stringstream ss;
      ss << "Vertex " << i << " no longer matches its index "
         << vertex->contextual_index();
      return absl::InternalError(ss.str());
    }
    prev[i].second = nullptr;
    if (i == begin_index)
      continue;
    cost[i] = std::numeric_limits<double>::max();
    seen[i] = false;
  }

  queue.push(begin);
  seen[begin_index] = true;

  while (!queue.empty()) {
    // NOTE(aryap): CPU profiler says this is slow:
    //  CPUPROFILE=bfg.prof LD_PRELOAD="/usr/local/lib/libprofiler.so" ./bfg
    //  pprof --gv bfg bfg.prof
    // 
    // Have to re-sort the queue so that new cost changes take effect. (The
    // queue is already mostly sorted so an insertion would be fast.)
    // std::sort(queue.begin(), queue.end(), vertex_sort_fn);

    RoutingVertex *current = queue.top();
    queue.pop();

    if (target_must_be_usable) {
      // If the target must be usable for a valid route (e.g. point to point
      // routing from vertex to vertex), we ignore unusable nodes as possible
      // targets.
      if (!usable_vertex(current))
        continue;
      if (is_target(current)) {
        found_targets.insert(current);
        continue;
      }
    } else {
      // If the target doesn't necessarily have to be usable, we check for a
      // valid target _before_ culling unusable nodes.
      if (is_target(current)) {
        found_targets.insert(current);
        continue;
      }
      if (!usable_vertex(current))
        continue;
    }

    size_t current_index = current->contextual_index();

    for (RoutingEdge *edge : current->edges()) {
      if (!usable_edge(edge)) {
        continue;
      }

      // We don't know what direction we're using the edge in, and edges are
      // not directional per se, so pick the side that isn't the one we came in
      // on:
      // TODO(aryap): Maybe bake this into the RoutingEdge.
      RoutingVertex *next =
          edge->first() == current ? edge->second() : edge->first();

      size_t next_index = next->contextual_index();

      double next_cost = cost[current_index] + edge->cost() + next->cost();

      LOG_IF(FATAL, !std::isfinite(next_cost)) << "!";

      if (next_cost < cost[next_index]) {
        cost[next_index] = next_cost;
        prev[next_index] = std::make_pair(current_index, edge);

        // If we haven't seen this node before we should definitely visit it.
        if (!seen[next_index]) {
          queue.push(next);
          seen[next_index] = true;
        }
      }
    }
  }

  if (found_targets.empty()) {
    LOG(INFO) << "No usable targets found.";
    return nullptr;
  }

  // Sort all of the found targets according to their final cost:
  std::vector<RoutingVertex*> sorted_targets(
      found_targets.begin(), found_targets.end());
  auto target_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    //double cost_to_complete = 
    return cost[a->contextual_index()] < cost[b->contextual_index()];
  };
  std::sort(sorted_targets.begin(), sorted_targets.end(), target_sort_fn);
  RoutingVertex *end_target = sorted_targets.front();
  size_t end_index = end_target->contextual_index();

  if (discovered_target)
    *discovered_target = end_target;

  std::deque<RoutingEdge*> shortest_edges;

  size_t last_index = prev[end_index].first;
  RoutingEdge *last_edge = prev[end_index].second;

  while (last_edge != nullptr) {
    LOG_IF(FATAL, (last_edge->first() != vertices_[last_index] &&
                   last_edge->second() != vertices_[last_index]))
        << "last_edge does not land back at source vertex";

    shortest_edges.push_front(last_edge);

    if (last_index == begin_index) {
      // We found our way back.
      break;
    }

    auto &last_entry = prev[last_index];
    last_index = last_entry.first;

    last_edge = last_entry.second;
  }

  if (shortest_edges.empty()) {
    return nullptr;
  } else if (shortest_edges.front()->first() != begin &&
             shortest_edges.front()->second() != begin) {
    LOG(FATAL) << "Did not find beginning vertex.";
    return nullptr;
  }

  RoutingPath *path = new RoutingPath(*this, begin, shortest_edges);
  return path;
}

void RoutingGrid::AddBlockages(
    const geometry::ShapeCollection &shapes,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *changed_out) {
  // When adding permanent blockages (is_temporary == false) we skip pin shapes,
  // since those might be needed for connection by the routing grid.
  // TODO(aryap): Not sure about this interaction... maybe this is a flag?
  for (const auto &rectangle : shapes.rectangles()) {
    if (!is_temporary && rectangle->is_connectable())
      continue;
    AddBlockage(*rectangle, padding, is_temporary, changed_out);
  }
  for (const auto &polygon : shapes.polygons()) {
    if (!is_temporary && polygon->is_connectable())
      continue;
    AddBlockage(*polygon, padding, is_temporary, changed_out);
  }
  for (const auto &port : shapes.ports()) {
    if (!is_temporary && port->is_connectable())
      continue;
    AddBlockage(*port, padding);
  }
}

RoutingGridBlockage<geometry::Rectangle> *RoutingGrid::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  const geometry::Layer &layer = rectangle.layer();
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end())
    return nullptr;

  // TODO(aryap): RoutingTracks are equipped with min_separation, but
  // RoutingGridBlockages are not. padding is sometimes treated as a temporary
  // additional value and sometimes as the min_separation value.
  // RoutingGridBlockage has two explicit checks:
  //    BlocksWithoutPadding, meaning that the shapes overlap, and
  //    Blocks, meaning that the shapes come within the min_separation.
  // In RoutingGridBlockage "padding" is min_separation. Does it need to have
  // both?
  //
  // Or should RoutingGridBlockage be able to look up the min_separation on its
  // own? RoutingGridBlockage could easily do this, but then it would need a
  // handle to physical information:
  int64_t min_separation = physical_db_.Rules(layer).min_separation;

  // Create and save the blockage:
  RoutingGridBlockage<geometry::Rectangle> *blockage =
      new RoutingGridBlockage<geometry::Rectangle>(
          *this, rectangle, padding + min_separation);
  rectangle_blockages_.emplace_back(blockage);

  for (RoutingTrack *track : it->second) {
    if (is_temporary) {
      RoutingTrackBlockage* track_blockage = track->AddTemporaryBlockage(
          rectangle, padding, blocked_vertices, blocked_edges);
      blockage->AddChildTrackBlockage(track, track_blockage);
    } else {
      // Add permanent blockage.
      track->AddBlockage(rectangle, padding);
    }
  }

  ApplyBlockage(*blockage, blocked_vertices);
  return blockage;
}

RoutingGridBlockage<geometry::Polygon> *RoutingGrid::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices) {
  const geometry::Layer &layer = polygon.layer();

  int64_t min_separation = physical_db_.Rules(layer).min_separation;

  // Create and save the blockage:
  RoutingGridBlockage<geometry::Polygon> *blockage =
      new RoutingGridBlockage<geometry::Polygon>(
          *this, polygon, padding + min_separation);
  polygon_blockages_.emplace_back(blockage);

  // Tracks don't support temporary Polygon blockages, so for now we just skip:
  if (is_temporary) {
    LOG(WARNING) << "Temporary blockage is a Polygon which tracks don't "
                 << "support, ignoring: " << polygon;
  } else {
    auto it = tracks_by_layer_.find(layer);
    if (it == tracks_by_layer_.end())
      return nullptr;
    for (RoutingTrack *track : it->second) {
      track->AddBlockage(polygon, padding);
    }
  }

  ApplyBlockage(*blockage, blocked_vertices);
  return blockage;
}

void RoutingGrid::RemoveUnavailableVertices() {
  for (auto &entry : available_vertices_by_layer_) {
    std::vector<RoutingVertex*> &available = entry.second;
    for (auto it = available.begin(); it != available.end();) {
      RoutingVertex *vertex = *it;
      if (!vertex->available()) {
        it = available.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void RoutingGrid::ExportEdgesAsRectangles(
    const std::string &layer, bool available_only, Layout *layout) const {
  layout->SetActiveLayerByName(layer);

  const int64_t kPadding = 2;

  for (const auto &entry : tracks_by_layer_) {
    const geometry::Layer &track_layer = entry.first;
    for (const RoutingTrack *track : entry.second) {
      track->ExportEdgesAsRectangles(layer, available_only, layout);
    }
  }

  for (RoutingEdge *edge : off_grid_edges_) {
    if (available_only && edge->blocked())
      continue;
    auto rectangle = edge->AsRectangle(kPadding);
    if (!rectangle)
      continue;
    layout->AddRectangle(*rectangle);
  }
}

void RoutingGrid::ExportVerticesAsSquares(
    const std::string &layer, bool available_only, Layout *layout) const {
  layout->SetActiveLayerByName(layer);
  for (RoutingVertex *vertex : vertices_) {
    if (!available_only || vertex->available()) {
      layout->AddSquare(vertex->centre(), 10);
    }
  }
}

void RoutingGrid::AddRoutingViaInfo(
    const Layer &lhs,
    const Layer &rhs,
    const RoutingViaInfo &info) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  // Order first and second.
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  LOG_IF(FATAL,
      via_infos_.find(first) != via_infos_.end() &&
      via_infos_[first].find(second) != via_infos_[first].end())
      << "Attempt to specify RoutingViaInfo for layers " << first << " and "
      << second << " again.";
  via_infos_[first][second] = info;
}

const RoutingViaInfo &RoutingGrid::GetRoutingViaInfoOrDie(
    const Layer &lhs, const Layer &rhs) const {
  auto via_info = GetRoutingViaInfo(lhs, rhs);
  LOG_IF(FATAL, !via_info)
      << "No known connection between layer " << lhs << " and layer " << rhs;
  return *via_info;
}

std::optional<std::reference_wrapper<const RoutingViaInfo>>
RoutingGrid::GetRoutingViaInfo(const Layer &lhs, const Layer &rhs) const {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;

  const auto first_it = via_infos_.find(first);
  if (first_it == via_infos_.end())
    return std::nullopt;
  const std::map<Layer, RoutingViaInfo> &inner_map = first_it->second;
  const auto second_it = inner_map.find(second);
  if (second_it == inner_map.end())
    return std::nullopt;
  return second_it->second;
}

void RoutingGrid::AddRoutingLayerInfo(const RoutingLayerInfo &info) {
  const Layer &layer = info.layer;
  auto layer_info_it = routing_layer_info_.find(layer);
  LOG_IF(FATAL, layer_info_it != routing_layer_info_.end())
      << "Duplicate routing layer info: " << layer;
  routing_layer_info_.insert({layer, info});
}

std::optional<std::reference_wrapper<const RoutingLayerInfo>>
    RoutingGrid::GetRoutingLayerInfo(const geometry::Layer &layer) const {
  auto it = routing_layer_info_.find(layer);
  if (it == routing_layer_info_.end())
    return std::nullopt;
  return it->second;
}

const RoutingLayerInfo &RoutingGrid::GetRoutingLayerInfoOrDie(
    const geometry::Layer &layer) const {
  auto info = GetRoutingLayerInfo(layer);
  LOG_IF(FATAL, !info)
      << "Could not find routing information for layer " << layer;
  return *info;
}

void RoutingGrid::AddTrackToLayer(
    RoutingTrack *track, const geometry::Layer &layer) {
  // Create the first vector of tracks.
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end()) {
    tracks_by_layer_.insert({layer, {track}});
    return;
  }
  it->second.push_back(track);
}

bool RoutingGrid::PointsAreTooCloseForVias(
    const geometry::Layer &shared_layer,
    const geometry::Point &lhs,
    const geometry::Layer &lhs_connectee,
    const geometry::Point &rhs,
    const geometry::Layer &rhs_connectee) const {
  int64_t separation = static_cast<int64_t>(lhs.L2DistanceTo(rhs));

  auto maybe_lhs_via = GetRoutingViaInfo(shared_layer, lhs_connectee);
  if (!maybe_lhs_via)
    return false;

  auto maybe_rhs_via = GetRoutingViaInfo(shared_layer, rhs_connectee);
  if (!maybe_rhs_via)
    return false;

  auto maybe_shared_layer_info = GetRoutingLayerInfo(shared_layer);
  if (!maybe_shared_layer_info)
    return false;

  const RoutingViaInfo &lhs_via = *maybe_lhs_via;
  const RoutingViaInfo &rhs_via = *maybe_rhs_via;
  const RoutingLayerInfo &shared_layer_info = *maybe_shared_layer_info;
  
  int64_t lhs_max_via_half_width = lhs_via.MaxViaSide() / 2;
  int64_t lhs_max_via_overhang = lhs_via.MaxOverhang();

  int64_t rhs_max_via_half_width = rhs_via.MaxViaSide() / 2;
  int64_t rhs_max_via_overhang = rhs_via.MaxOverhang();

  int64_t min_separation = shared_layer_info.min_separation;

  int64_t required =
      lhs_max_via_half_width + lhs_max_via_overhang +
      min_separation +
      rhs_max_via_half_width + rhs_max_via_overhang;

  if (separation < required) {
    LOG(INFO)
        << "Via between " << shared_layer << " and " << lhs_connectee
        << " requires at least " << required << " units to via between "
        << shared_layer << " and " << rhs_connectee
        << ", but there are only " << separation << " units; therefore "
        << lhs << " and " << rhs << " are too close together.";
    return true;
  }
  return false;
}

bool RoutingGrid::VerticesAreTooCloseForVias(
    const RoutingVertex &lhs, const RoutingVertex &rhs) const {
  const std::set<geometry::Layer> &lhs_layers = lhs.connected_layers();
  const std::set<geometry::Layer> &rhs_layers = rhs.connected_layers();
  std::set<geometry::Layer> shared_layers;
  std::set_intersection(lhs_layers.begin(), lhs_layers.end(),
                        rhs_layers.begin(), rhs_layers.end(),
                        std::inserter(shared_layers, shared_layers.begin()));
  if (shared_layers.empty())
    return false;

  // Shortcuts for on-grid vertices:
  if (lhs.horizontal_track() == rhs.horizontal_track() &&
      lhs.grid_position_x() &&
      rhs.grid_position_x()) {
    // They might be horizontal neighbours:
    size_t diff = std::max(*lhs.grid_position_x(), *rhs.grid_position_x()) -
        std::min(*lhs.grid_position_x(), *rhs.grid_position_x());
    return diff == 1U;
    // (This is the same as:
    //  int64_t diff = std::abs(static_cast<int64_t>(*lhs.grid_position_x()) -
    //      static_cast<int64_t>(*rhs.grid_position_x()));
    //  return diff == 1;
    // )
  } else if (lhs.vertical_track() == rhs.vertical_track() &&
             lhs.grid_position_y() &&
             rhs.grid_position_y()) {
    // They might be vertical neighbours:
    size_t diff = std::max(*lhs.grid_position_y(), *rhs.grid_position_y()) -
        std::min(*lhs.grid_position_y(), *rhs.grid_position_y());
    return diff == 1U;
  }

  // Check that the distance between the two vertices can accommodate vias on
  // each of the shared layers. To do this we have to get the rules for the vias
  // placeable on each `shared_layer`, which means getting the rules for via
  // encapsulation and such. That means means we need to get a handle to all
  // vias between `shared_layer` and every connectable layer, then figure out
  // if the rules for that via would cause a violation.
  for (const geometry::Layer &source_layer : shared_layers) {
    for (const geometry::Layer &lhs_connectee : lhs.connected_layers()) {
      if (lhs_connectee == source_layer)
        continue;
      for (const geometry::Layer &rhs_connectee : rhs.connected_layers()) {
        if (rhs_connectee == source_layer)
          continue;
        if (PointsAreTooCloseForVias(source_layer,
                                     lhs.centre(),
                                     lhs_connectee,
                                     rhs.centre(),
                                     rhs_connectee)) {
          return true;
        }
      }
    }
  }

  return false;
}

void RoutingGrid::SetUpTemporaryBlockages(
    const std::set<geometry::Port*> &avoid,
    TemporaryBlockageInfo *blockage_info) {
  for (geometry::Port *port : avoid) {
    std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
        layer_access = physical_db_.FindReachableLayersByPinLayer(
            port->layer());
    for (const auto &entry : layer_access) {
      const geometry::Layer &access_layer = entry.first;
      const std::set<geometry::Layer> &reachable_by_one_via =
          entry.second;
      for (const auto &footprint_layer : reachable_by_one_via) {
        // Not all reachable layers are actually usable by the routing grid.
        // Instead of making ViaFootprint handle this, we just check:
        if (!GetRoutingViaInfo(access_layer, footprint_layer)) {
          continue;
        }
        auto pin_projection = ViaFootprint(
            port->centre(), access_layer, footprint_layer);
        if (!pin_projection) {
          continue;
        }
        int64_t min_separation =
            physical_db_.Rules(footprint_layer).min_separation;

        pin_projection->set_layer(footprint_layer);
        RoutingGridBlockage<geometry::Rectangle> *pin_blockage =
            AddBlockage(*pin_projection,
                        min_separation,
                        true, // Temporary blockage.
                        &blockage_info->blocked_vertices,
                        &blockage_info->blocked_edges);
        if (pin_blockage)
          blockage_info->pin_blockages.push_back(pin_blockage);
      }
      VLOG(13) << "avoiding "
               << blockage_info->blocked_vertices.size() << " vertices and "
               << blockage_info->blocked_edges.size() << " edges";
    }
  }
}

void RoutingGrid::TearDownTemporaryBlockages(
    const TemporaryBlockageInfo &blockage_info) {
  for (RoutingVertex *const vertex : blockage_info.blocked_vertices) {
    vertex->set_available(true);
  }
  for (RoutingEdge *const edge : blockage_info.blocked_edges) {
    edge->set_blocked(false);
  }
  for (RoutingGridBlockage<geometry::Rectangle> *const blockage :
          blockage_info.pin_blockages) {
    ForgetBlockage(blockage);
  }
}

std::vector<RoutingGrid::CostedLayer> RoutingGrid::LayersReachableByVia(
    const geometry::Layer &from_layer) const {
  std::vector<RoutingGrid::CostedLayer> reachable;

  // Greater (in the std::less sense) layers are found directly:
  auto it = via_infos_.find(from_layer);
  if (it != via_infos_.end()) {
    for (const auto &inner : it->second) {
      const geometry::Layer &to = inner.first;
      double cost = inner.second.cost();
      reachable.push_back({to, cost});
    }
  }

  // Lesser layers are found indirectly:
  for (const auto &outer : via_infos_) {
    const geometry::Layer &maybe_reachable = outer.first;
    if (maybe_reachable == from_layer)
      continue;
    for (const auto &inner : outer.second) {
      if (inner.first == from_layer) {
        double cost = inner.second.cost();
        reachable.push_back({maybe_reachable, cost});
      }
    }
  }
  return reachable;
}

std::optional<double> RoutingGrid::FindViaStackCost(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  auto maybe_stack = FindViaStack(lhs, rhs);
  if (!maybe_stack)
    return std::nullopt;
  const std::vector<RoutingViaInfo> &via_stack = *maybe_stack;
  double total_cost = 0.0;
  for (const RoutingViaInfo &info : via_stack) {
    total_cost += info.cost();
  }
  return total_cost;
}

std::optional<std::vector<RoutingViaInfo>> RoutingGrid::FindViaStack(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  std::vector<RoutingViaInfo> via_stack;
  if (lhs == rhs) {
    return via_stack;
  }

  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const geometry::Layer &from = ordered_layers.first;
  const geometry::Layer &to = ordered_layers.second;

  // Dijkstra's shortest path but over the graph of via connectivity.

  // Best-known cost so far to get to the given layer from `from`.
  std::map<geometry::Layer, double> cost;
  std::map<geometry::Layer, geometry::Layer> previous;
  std::set<geometry::Layer> seen;

  // We can't easily enumerate all known layers from our given structures, so we
  // make the various bookkeeping sparse:
  auto get_cost = [&](const geometry::Layer &layer) {
    auto it = cost.find(layer);
    return it == cost.end() ? std::numeric_limits<double>::max() : it->second;
  };
  auto layer_sort_fn = [&](const geometry::Layer &from,
                           const geometry::Layer &to) {
    return get_cost(from) > get_cost(to);
  };
  std::priority_queue<geometry::Layer,
                      std::vector<geometry::Layer>,
                      decltype(layer_sort_fn)> queue(layer_sort_fn);

  cost[from] = 0.0;
  queue.push(from);

  while (!queue.empty()) {
    const geometry::Layer &current = queue.top();
    queue.pop();

    if (current == to) {
      break;
    }

    std::vector<CostedLayer> reachable = LayersReachableByVia(current);

    for (const auto &next : reachable) {
      const geometry::Layer &next_layer = next.layer;
      double next_cost = get_cost(current) + next.cost;
      if (next_cost < get_cost(next_layer)) {
        cost[next_layer] = next_cost;
        previous[next_layer] = current;

        if (seen.find(next_layer) == seen.end()) {
          queue.push(next_layer);
          seen.insert(next_layer);
        }
      }
    }
  }

  // Walk backwards to find the 'shortest path'.
  if (previous.find(to) == previous.end()) {
    // No path.
    return std::nullopt;
  }

  // [to, intermediary, other_intermediary, from]
  std::vector<geometry::Layer> layer_stack;
  auto it = previous.find(to);
  layer_stack.push_back(to);
  while (it != previous.end()) {
    geometry::Layer next_previous = it->second;
    layer_stack.push_back(next_previous);
    if (next_previous == from) {
      break;
    }
    it = previous.find(next_previous);
  }
  if (layer_stack.back() != from) {
    // No path found.
    return std::nullopt;
  }
  
  for (size_t i = layer_stack.size() - 1; i > 0; --i) {
    const geometry::Layer &rhs = layer_stack.at(i);
    const geometry::Layer &lhs = layer_stack.at(i - 1);
    const RoutingViaInfo &via_info = GetRoutingViaInfoOrDie(lhs, rhs);
    via_stack.push_back(via_info);
  }
  return via_stack;
}

PolyLineCell *RoutingGrid::CreatePolyLineCell() const {
  std::unique_ptr<PolyLineCell> cell(new PolyLineCell());
  for (RoutingPath *path : paths_) {
    path->ToPolyLinesAndVias(&cell->poly_lines(), &cell->vias());
  }
  return cell.release();
}

Layout *RoutingGrid::GenerateLayout() const {
  PolyLineInflator inflator(physical_db_);
  std::unique_ptr<PolyLineCell> grid_lines(CreatePolyLineCell());
  std::unique_ptr<bfg::Layout> grid_layout(
      inflator.Inflate(*this, *grid_lines));
  return grid_layout.release();
}

void RoutingGrid::AddRoutingGridGeometry(
    const geometry::Layer &lhs, const geometry::Layer &rhs,
    const RoutingGridGeometry &grid_geometry) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  LOG_IF(FATAL,
      grid_geometry_by_layers_.find(first) !=
          grid_geometry_by_layers_.end() &&
      grid_geometry_by_layers_[first].find(second) !=
          grid_geometry_by_layers_[first].end())
      << "Attempt to add RoutingGridGeometry for layers " << first << " and "
      << second << " again.";
  grid_geometry_by_layers_[first][second] = grid_geometry;
}

std::optional<std::reference_wrapper<RoutingGridGeometry>>
    RoutingGrid::GetRoutingGridGeometry(
        const geometry::Layer &lhs, const geometry::Layer &rhs) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  auto first_it = grid_geometry_by_layers_.find(first);
  if (first_it == grid_geometry_by_layers_.end())
    return std::nullopt;
  std::map<Layer, RoutingGridGeometry> &inner_map = first_it->second;
  auto second_it = inner_map.find(second);
  if (second_it == inner_map.end())
    return std::nullopt;
  return second_it->second;
}

std::vector<RoutingGridGeometry*>
RoutingGrid::FindRoutingGridGeometriesUsingLayer(
    const geometry::Layer &layer) {
  std::vector<RoutingGridGeometry*> grid_geometries;
  for (auto &entry : grid_geometry_by_layers_) {
    const Layer &first = entry.first;
    for (auto &inner : entry.second) {
      const Layer &second = inner.first;
      if (first != layer && second != layer)
        continue;
      grid_geometries.push_back(&inner.second);
    }
  }
  return grid_geometries;
}

} // namespace bfg
