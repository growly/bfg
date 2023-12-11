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
#include <utility>
#include <vector>

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

// TODO(aryap):
//  1) What does RoutingGrid::available_vertices_by_layer_ actually do?

// A RoutingGrid manages a multi-layer rectilinear wire grid for for
// connections between points.
//
// The grid comprises RoutingTracks, which own a set of RoutingVertexs and
// RoutingEdges. It also owns a separate collection of RoutingVertexs and
// RoutingEdges that do not fall onto specific tracks.
//

using bfg::geometry::Compass;

namespace bfg {

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
  const RoutingLayerInfo &lhs_info = GetRoutingLayerInfo(lhs);
  const RoutingLayerInfo &rhs_info = GetRoutingLayerInfo(rhs);
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

std::optional<std::pair<RoutingVertex*, const geometry::Layer>>
RoutingGrid::GenerateGridVertexForPort(
    const geometry::Port &port) {
  const std::set<geometry::Layer> layers =
      physical_db_.FindReachableLayersByPinLayer(port.layer());
  for (const geometry::Layer &layer : layers) {
    LOG(INFO) << "checking for grid vertex on layer " << layer;
    RoutingVertex *vertex = GenerateGridVertexForPoint(port.centre(), layer);
    if (vertex) {
      // This is
      //  std::optional<std::pair<RoutingVertex*, const geometry::Layer>>({vertex, layer});
      return {{vertex, layer}};
    }
  }
  return std::nullopt;
}

RoutingVertex *RoutingGrid::GenerateGridVertexForPoint(
    const geometry::Point &point, const geometry::Layer &layer) {
  // A key function of this class is to determine an appropriate starting point
  // on the routing grid for routing to/from an arbitrary point.
  //
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

  geometry::Point target_point = point;
  target_point.set_layer(layer);

  std::vector<CostedVertex> costed_vertices;
  for (const auto &entry : available_vertices_by_layer_) {
    // Is this layer reachable from the target?
    std::optional<std::reference_wrapper<const RoutingViaInfo>> needs_via;
    if (entry.first != layer) {
      needs_via = GetRoutingViaInfo(entry.first, layer);
      if (!needs_via)
        continue;
      LOG(INFO) << "layer " << physical_db_.DescribeLayer(layer)
                << " is accessible for routing via layer "
                << physical_db_.DescribeLayer(needs_via->get().layer);
    }

    for (RoutingVertex *vertex : entry.second) {
      uint64_t vertex_cost = vertex->L1DistanceTo(target_point);
      if (needs_via) {
        vertex_cost += (10.0 * needs_via->get().cost);
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
  //    X---+       X
  // (B)|   |(B')
  //    +---O
  //     (A')
  //
  //
  //    X           X
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

    LOG(INFO) << "vertex " << candidate->centre() << " layer " << vertex_layer
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

    // Try putting it on the vertical track and then horizontal track.
    std::vector<RoutingTrack*> tracks = {
      candidate->vertical_track(), candidate->horizontal_track()};
    RoutingVertex *bridging_vertex = nullptr;
    for (size_t i = 0; i < tracks.size() && bridging_vertex == nullptr; ++i) {
      bridging_vertex = tracks[i]->CreateNearestVertexAndConnect(
          target_point, candidate);
    }
    if (bridging_vertex == nullptr)
      continue;

    // TODO(aryap): Need a way to roll back this temporary objects in case the
    // caller's entire process fails - i.e. a vertex can be created for the
    // starting point but not for the ending point.

    // Success, so add a new vertex at this position and the bridging one too.
    if (bridging_vertex != candidate) {
      // If the closest vertex was the candidate itself, no bridging vertex is
      // necessary. Otherwise:

      bridging_vertex->AddConnectedLayer(vertex_layer);
      AddVertex(bridging_vertex);
    }

    RoutingVertex *off_grid = new RoutingVertex(target_point);
    off_grid->AddConnectedLayer(vertex_layer);
    AddVertex(off_grid);

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid);
    edge->set_layer(vertex_layer);
    LOG(INFO) << "connected new vertex " << bridging_vertex->centre()
              << " on layer " << edge->ExplicitOrTrackLayer();
    bridging_vertex->AddEdge(edge);
    off_grid->AddEdge(edge);
    
    // TODO(aryap): It's unclear what layer this edge is on. The opposite of
    // what the bridging edge is on, I guess.
    // TODO(aryap): It's not clear if the off-grid edge will be legal. We have
    // to check with the whole grid.

    off_grid_edges_.insert(edge);
    return off_grid;
  }
  return nullptr;
}

std::optional<geometry::Rectangle> RoutingGrid::ViaFootprint(
    const RoutingVertex &vertex,
    int64_t padding) const {
  const std::vector<geometry::Layer> &layers = vertex.connected_layers();
  if (layers.size() != 2) {
    return std::nullopt;
  }
  const geometry::Layer &first_layer = layers.front();
  const geometry::Layer &second_layer = layers.back();

  // Get the applicable via info for via sizing and encapsulation values:
  const RoutingViaInfo &routing_via_info = GetRoutingViaInfoOrDie(
      first_layer, second_layer);
  int64_t via_width = std::max(
      routing_via_info.width, routing_via_info.height) + 2 * std::max(
      routing_via_info.overhang_length, routing_via_info.overhang_width) +
      2 * padding;
  geometry::Point lower_left = vertex.centre() - geometry::Point(
      via_width / 2, via_width / 2);
  geometry::Rectangle footprint = geometry::Rectangle(
      lower_left, via_width, via_width);
  return footprint;
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

void RoutingGrid::ConnectLayers(
    const geometry::Layer &first, const geometry::Layer &second) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  const RoutingLayerInfo &horizontal_info = split_directions.first;
  const RoutingLayerInfo &vertical_info = split_directions.second;

  LOG(INFO) << "Drawing grid between layers " << horizontal_info.layer
            << ", " << vertical_info.layer;

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal_info, vertical_info);

  std::vector<RoutingVertex*> &first_layer_vertices =
      GetAvailableVertices(first);
  std::vector<RoutingVertex*> &second_layer_vertices =
      GetAvailableVertices(second);

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
        vertical_info.layer, RoutingTrackDirection::kTrackVertical, x);
    track->set_width(vertical_info.wire_width);
    vertical_tracks.insert({x, track});
    AddTrackToLayer(track, vertical_info.layer);
    num_x++;
  }

  for (int64_t y = grid_geometry.y_start();
       y <= grid_geometry.y_max();
       y += grid_geometry.y_pitch()) {
    RoutingTrack *track = new RoutingTrack(
        horizontal_info.layer,
        RoutingTrackDirection::kTrackHorizontal, y);
    track->set_width(horizontal_info.wire_width);
    horizontal_tracks.insert({y, track});
    AddTrackToLayer(track, horizontal_info.layer);
    num_y++;
  }

  std::vector<std::vector<RoutingVertex*>> &vertices =
      grid_geometry.vertices_by_grid_position();

  // Generate a vertex at the intersection of every horizontal and vertical
  // track.
  size_t i = 0;
  for (int64_t x = grid_geometry.x_start();
       x < grid_geometry.x_max();
       x += grid_geometry.x_pitch()) {
    // This (and the horizontal one) must exist by now, so we can make this
    // fatal.
    RoutingTrack *vertical_track = vertical_tracks.find(x)->second;
    LOG_IF(FATAL, !vertical_track) << "Vertical routing track is nullptr";

    size_t j = 0;
    for (int64_t y = grid_geometry.y_start();
         y < grid_geometry.y_max();
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

      VLOG(10) << "Vertex created: " << vertex->centre() << " on layers: "
               << absl::StrJoin(vertex->connected_layers(), ", ");

      vertex->set_grid_position_x(i);
      vertex->set_grid_position_y(j);

      vertices[i][j] = vertex;

      // Assign neighbours. Since we do the reciprocal relationship too, we
      // assigning up to all 8 neighbours per iteration.
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
}

void RoutingGrid::AddVertex(RoutingVertex *vertex) {
  for (const geometry::Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingVertex*> &available = GetAvailableVertices(layer);
    available.push_back(vertex);
  }
  vertices_.push_back(vertex);  // The class owns all of these.
}

bool RoutingGrid::AddRouteBetween(
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
  std::set<RoutingVertex*> blocked_vertices;
  std::set<RoutingEdge*> blocked_edges;
  for (geometry::Port *port : avoid) {
    auto pin_access = physical_db_.FindReachableLayersByPinLayer(port->layer());
    if (pin_access.empty()) {
      LOG(WARNING)
          << "Pin " << *port << " has no known pin accesss layers; "
          << "avoiding just the pin layer itself: " << port->layer();
      geometry::Rectangle pin_projection = *port;
      pin_projection.set_layer(port->layer());
      AddBlockage(
          pin_projection, 100 /* FIXME */, &blocked_vertices, &blocked_edges);
      continue;
    }
    for (const geometry::Layer &layer : pin_access) {
      geometry::Rectangle pin_projection = *port;
      pin_projection.set_layer(layer);
      AddBlockage(
          pin_projection, 100 /* FIXME */, &blocked_vertices, &blocked_edges);
    }
    LOG(INFO) << "avoiding " << blocked_vertices.size() << " vertices and "
              << blocked_edges.size() << " edges";
  }

  auto begin_vertex_and_access_layer = GenerateGridVertexForPort(begin);
  if (!begin_vertex_and_access_layer) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    return false;
  }
  RoutingVertex *begin_vertex = begin_vertex_and_access_layer->first;
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  auto end_vertex_and_access_layer = GenerateGridVertexForPort(end);
  if (!end_vertex_and_access_layer) {
    LOG(ERROR) << "Could not find available vertex for end port.";
    return false;
  }
  RoutingVertex *end_vertex = end_vertex_and_access_layer->first;
  LOG(INFO) << "Nearest vertex to end (" << end << ") is "
            << end_vertex->centre();

  std::unique_ptr<RoutingPath> shortest_path(
      ShortestPath(begin_vertex, end_vertex));

  if (!shortest_path) {
    LOG(WARNING) << "No path found.";
    return false;
  }

  // Remember the ports to which the path should connect.
  //
  // Ports are typically on port layers (i.e. PINs), but for convenience we also
  // record the layer we need to use to access said ports:
  shortest_path->set_start_port(&begin);
  shortest_path->set_start_access_layer(begin_vertex_and_access_layer->second);
  shortest_path->set_end_port(&end);
  shortest_path->set_end_access_layer(end_vertex_and_access_layer->second);

  LOG(INFO) << "Found path: " << *shortest_path;

  // Once the path is found, but _before_ it is installed, the temporarily
  // blocked nodes should be re-enabled. This might be permanently blocked by
  // installing the path finally!
  for (RoutingVertex *vertex : blocked_vertices) {
    vertex->set_available(true);
  }
  for (RoutingEdge *edge : blocked_edges) {
    edge->set_available(true);
  }

  // Assign net and install:
  if (!net.empty())
    shortest_path->set_net(net);

  InstallPath(shortest_path.release());

  return true;
}

bool RoutingGrid::AddRouteToNet(
    const geometry::Port &begin, const std::string &net) {
  RoutingVertex *begin_vertex = GenerateGridVertexForPoint(
      begin.centre(), begin.layer());
  if (!begin_vertex) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    return false;
  }
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  std::unique_ptr<RoutingPath> shortest_path(
      ShortestPath(begin_vertex, net));

  if (!shortest_path) {
    LOG(WARNING) << "No path found to net " << net << ".";
    return false;
  }

  // Remember the ports to which the path should connect.
  shortest_path->set_start_port(&begin);

  LOG(INFO) << "Found path: " << *shortest_path;
  //LOG(INFO) << "final vertex is on layer " << shortest_path->vertices().end()->layer();

  // Assign net and install:
  shortest_path->set_net(net);

  InstallPath(shortest_path.release());

  return true;
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
  LOG_IF(FATAL, pos == vertices_.end())
      << "Did not find vertex we're removing in RoutingGrid list of "
      << "vertices: " << vertex;
  vertices_.erase(pos);
  if (and_delete)
    delete vertex;
  return true; // TODO(aryap): Always returning true, huh...
}

void RoutingGrid::InstallPath(RoutingPath *path) {
  LOG_IF(FATAL, path->Empty()) << "Cannot install an empty path.";

  // Remove edges from the track which owns them.
  for (RoutingEdge *edge : path->edges()) {
    if (edge->track() != nullptr) {
      edge->track()->MarkEdgeAsUsed(
          edge, path->net() == "" ? nullptr : &path->net());
    }
    edge->set_available(false);
  }

  LOG_IF(FATAL, path->vertices().size() != path->edges().size() + 1)
      << "Path vertices and edges mismatched. There are "
      << path->edges().size() << " edges and "
      << path->vertices().size() << " vertices";

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

    // Disable neighbours.
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
      std::set<RoutingVertex*> neighbours =
          next_vertex->GetNeighbours(position);
      for (RoutingVertex *neighbour : neighbours) {
        neighbour->set_available(false);
      }
    };

    ++i;
  }
  
  paths_.push_back(path);
}

RoutingPath *RoutingGrid::ShortestPath(
    RoutingVertex *begin, 
    RoutingVertex *end,
    std::function<bool(RoutingVertex*)> usable_vertex,
    std::function<bool(RoutingEdge*)> usable_edge) {
  // FIXME(aryap): This is very bad.
  if (!usable_vertex(begin)) {
    LOG(WARNING) << "Start vertex for path is not available";
    return nullptr;
  }
  if (!usable_vertex(end)) {
    LOG(WARNING) << "End vertex for path is not available";
    return nullptr;
  }

  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  std::vector<double> cost(vertices_.size());
  // TODO(aryap): This is marginally faster?
  //double cost[vertices_.size()];

  // Recording the edge to take back to the start that makes the shortest path,
  // as well as the vertex it leads to. If RoutingEdge* is nullptr then this is
  // invalid.
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  auto vertex_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    // We want the lowest value at the back of the array.
    // But in a priority_queue, we want the highest value at the start of the
    // collection so that the least element is popped first (because that's how
    // priorit_queue works).
    return cost[a->contextual_index()] > cost[b->contextual_index()];
  };
  // All vertices sorted according to their cost.
  std::priority_queue<RoutingVertex*,
                      std::vector<RoutingVertex*>,
                      decltype(vertex_sort_fn)> queue(vertex_sort_fn);
  bool seen[vertices_.size()];

  size_t begin_index = begin->contextual_index();
  size_t end_index = end->contextual_index();

  cost[begin_index] = 0;

  for (size_t i = 0; i < vertices_.size(); ++i) {
    RoutingVertex *vertex = vertices_[i];
    LOG_IF(FATAL, i != vertex->contextual_index())
      << "Vertex " << i << " no longer matches its index "
      << vertex->contextual_index();
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
    size_t current_index = current->contextual_index();

    // NOTE: We assume current is available, otherwise we will never have it
    // added to the queue to detect goal completion here.
    if (current == end) {
      continue;
    }

    for (RoutingEdge *edge : current->edges()) {
      if (!usable_edge(edge))
        continue;

      // We don't know what direction we're using the edge in, and edges are
      // not directional per se, so pick the side that isn't the one we came in
      // on:
      // TODO(aryap): Maybe bake this into the RoutingEdge.
      RoutingVertex *next =
          edge->first() == current ? edge->second() : edge->first();

      if (!usable_vertex(next))
        continue;

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

  RoutingPath *path = new RoutingPath(begin, shortest_edges);
  return path;
}

RoutingPath *RoutingGrid::ShortestPath(
    RoutingVertex *begin, const std::string &to_net) {
  LOG_IF(WARNING, !begin->available()) << "Start vertex for path is not available";

  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  auto is_target = [&](RoutingVertex *candidate) {
    return candidate->net() == to_net;
  };

  double cost[vertices_.size()];
  bool seen[vertices_.size()];

  // Recording the edge to take back to the start that makes the shortest path,
  // as well as the vertex it leads to. If RoutingEdge* is nullptr then this is
  // invalid.
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  auto vertex_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    // We want the lowest value at the back of the array.
    // But in a priority_queue, we want the highest value at the start of the
    // collection so that the least element is popped first (because that's how
    // priorit_queue works).
    return cost[a->contextual_index()] > cost[b->contextual_index()];
  };
  // All vertices sorted according to their cost.
  std::priority_queue<RoutingVertex*,
                      std::vector<RoutingVertex*>,
                      decltype(vertex_sort_fn)> queue(vertex_sort_fn);

  auto target_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    return cost[a->contextual_index()] < cost[b->contextual_index()];
  };
  std::set<RoutingVertex*,
           decltype(target_sort_fn)> found_targets(target_sort_fn);

  size_t begin_index = begin->contextual_index();
  cost[begin_index] = 0;

  for (size_t i = 0; i < vertices_.size(); ++i) {
    RoutingVertex *vertex = vertices_[i];
    LOG_IF(FATAL, i != vertex->contextual_index())
      << "Vertex " << i << " no longer matches its index "
      << vertex->contextual_index();
    prev[i].second = nullptr;
    if (i == begin_index)
      continue;
    cost[i] = std::numeric_limits<double>::max();
    seen[i] = false;
  }

  queue.push(begin);
  seen[begin_index] = true;

  while (!queue.empty()) {
    RoutingVertex *current = queue.top();
    queue.pop();

    if (is_target(current)) {
      found_targets.insert(current);
      continue;
    }

    if (!current->available()) {
      continue;
    }

    size_t current_index = current->contextual_index();

    for (RoutingEdge *edge : current->edges()) {
      if (!edge->available())
        continue;

      // We don't know what direction we're using the edge in, and edges are
      // not directional per se, so pick the side that isn't the one we came in
      // on:
      // TODO(aryap): Maybe bake this into the RoutingEdge.
      RoutingVertex *next =
          edge->first() == current ? edge->second() : edge->first();

      size_t next_index = next->contextual_index();

      double next_cost = cost[current_index] + edge->cost() + next->cost();

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

  if (found_targets.empty())
    return nullptr;

  // Pick the lowest-cost target:
  RoutingVertex *end_target = *found_targets.begin();
  size_t end_index = end_target->contextual_index();

  std::deque<RoutingEdge*> shortest_edges;

  RoutingEdge *last_edge = prev[end_index].second;
  size_t last_index = prev[end_index].first;
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

  RoutingPath *path = new RoutingPath(begin, shortest_edges);
  return path;
}

void RoutingGrid::AddBlockages(
    const geometry::ShapeCollection &shapes,
    int64_t padding,
    std::set<RoutingVertex*> *changed_out) {
  for (const auto &rectangle : shapes.rectangles()) {
    AddBlockage(*rectangle, padding, changed_out);
  }
  for (const auto &polygon : shapes.polygons()) {
    AddBlockage(*polygon, padding, changed_out);
  }
  // Do not add ports as permanent blockages. They must be considered
  // route-by-route, since some ports might be needed for connection.
  // FIXME(aryap): Need to add temporary blockages during search.
  //for (const auto &port : shapes.ports()) {
  //  AddBlockage(*port, padding);
  //}
}

void RoutingGrid::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  const geometry::Layer &layer = rectangle.layer();
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end())
    return;
  for (RoutingTrack *track : it->second) {
    if (blocked_vertices || blocked_edges) {
      track->AddTemporaryBlockage(
          rectangle, padding, blocked_vertices, blocked_edges);
    } else {
      // Add permanent blockage.
      track->AddBlockage(rectangle, padding);
    }
  }

  // Find any possibly-blocked vertices and make them unavailable:
  std::vector<RoutingGridGeometry*> grid_geometries;
  FindRoutingGridGeometriesForLayer(layer, &grid_geometries);
  for (RoutingGridGeometry *grid_geometry : grid_geometries) {
    std::set<RoutingVertex*> vertices;
    grid_geometry->EnvelopingVertices(rectangle, &vertices);
    for (RoutingVertex *vertex : vertices) {
      if (!vertex->available())
        continue;
      if (ViaWouldIntersect(*vertex,
                            rectangle,
                            padding)) {
        vertex->set_available(false);
        if (blocked_vertices) {
          blocked_vertices->insert(vertex);
        }
        VLOG(15) << "blockage: " << rectangle << " would block " << vertex;
      }
    }
  }
}

void RoutingGrid::AddBlockage(const geometry::Polygon &polygon,
                              int64_t padding,
                              std::set<RoutingVertex*> *changed_out) {
  const geometry::Layer &layer = polygon.layer();
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end())
    return;
  for (RoutingTrack *track : it->second) {
    track->AddBlockage(polygon, padding);
  }

  std::vector<RoutingGridGeometry*> grid_geometries;
  FindRoutingGridGeometriesForLayer(layer, &grid_geometries);

  for (RoutingGridGeometry *grid_geometry : grid_geometries) {
    std::set<RoutingVertex*> vertices;
    grid_geometry->EnvelopingVertices(polygon, &vertices);
    for (RoutingVertex *vertex : vertices) {
      if (!vertex->available())
        continue;
      if (ViaWouldIntersect(*vertex,
                            polygon,
                            padding)) {
        vertex->set_available(false);
        if (changed_out) {
          changed_out->insert(vertex);
        }
        VLOG(15) << "blockage: " << polygon << " would block " << vertex;
      }
    }
  }
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

void RoutingGrid::ExportAvailableVerticesAsSquares(
    const std::string &layer, Layout *layout) const {
  layout->SetActiveLayerByName(layer);
  for (RoutingVertex *vertex : vertices_) {
    if (vertex->available()) {
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

const RoutingLayerInfo &RoutingGrid::GetRoutingLayerInfo(
    const geometry::Layer &layer) const {
  auto it = routing_layer_info_.find(layer);
  LOG_IF(FATAL, it == routing_layer_info_.end())
      << "Could not find routing information for layer " << layer;
  return it->second;
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

PolyLineCell *RoutingGrid::CreatePolyLineCell() const {
  std::unique_ptr<PolyLineCell> cell(new PolyLineCell());
  for (RoutingPath *path : paths_) {
    path->ToPolyLinesAndVias(*this, &cell->poly_lines(), &cell->vias());
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

void RoutingGrid::FindRoutingGridGeometriesForLayer(
    const geometry::Layer layer,
    std::vector<RoutingGridGeometry*> *grid_geometries) {
  for (auto &entry : grid_geometry_by_layers_) {
    const Layer &first = entry.first;
    for (auto &inner : entry.second) {
      const Layer &second = inner.first;
      if (first != layer && second != layer)
        continue;
      grid_geometries->push_back(&inner.second);
    }
  }
}

} // namespace bfg
