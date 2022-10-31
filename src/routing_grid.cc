#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <utility>
#include <vector>

#include <absl/strings/str_join.h>
#include <glog/logging.h>

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
  // The first cut of this algorithm is to just find the closest of all the
  // available vertices on the given layer.

  auto it = available_vertices_by_layer_.find(layer);
  if (it == available_vertices_by_layer_.end()) {
    LOG(FATAL) << "Could not find a list of available vertices on layer: "
               << layer;
  }

  if (it->second.empty())
    return nullptr;

  std::vector<std::pair<uint64_t, RoutingVertex*>> costed_vertices;
  for (RoutingVertex *vertex : it->second) {
    uint64_t vertex_cost = vertex->L1DistanceTo(point);
    costed_vertices.emplace_back(vertex_cost, vertex);
  }

  // Should sort automatically based on operator< for first and second entries
  // in pairs.
  std::sort(costed_vertices.begin(),
            costed_vertices.end(),
            std::greater<std::pair<uint64_t, RoutingVertex*>>());

  // To ensure we can go the "last mile", we check if the required paths, as
  // projected on the tracks on which the nearest vertex lies, are legal.
  // Consider 4 vertices X on the RoutingGrid surrounding the port O. The
  // sections (A) and (B) must be legal on the horizontal or vertical tracks
  // to which we must connect O to get to X:
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
  // If neither is legal, we cannot use that vertex. In the diagram, (A) and
  // (B) are the bridging edges, and (A') and (B') are the off-grid edges.
  //
  // We generate a new RoutingVertex for the landing spot on each track and
  // provide that to the grid-router to use in finding a shortest path.
  //
  // TODO(aryap): A better idea is to create up to, say, 4 candidate bridging
  // vertices on legal tracks around the point so that they can be included in
  // the global shortest-path search. This would avoid having to turn corners
  // and go backwards, for example.
  while (!costed_vertices.empty()) {
    RoutingVertex *candidate = costed_vertices.back().second;
    costed_vertices.pop_back();

    if (candidate->vertical_track() == nullptr) {
      // FIXME(aryap): Is this a problem?
      VLOG(10) << "Cannot use vertex " << candidate
               << " as candidate because vertical track is nullptr";
      continue;
    } else if (candidate->horizontal_track() == nullptr) {
      VLOG(1) << "Cannot use vertex " << candidate
              << " as candidate because horizontal track is nullptr";
      continue;
    }

    // Try putting it on the vertical track and then horizontal track.
    std::vector<RoutingTrack*> tracks = {
      candidate->vertical_track(), candidate->horizontal_track()};
    RoutingVertex *bridging_vertex = nullptr;
    for (size_t i = 0; i < tracks.size() && bridging_vertex == nullptr; ++i) {
      bridging_vertex = tracks[i]->CreateNearestVertexAndConnect(
          point, candidate);
    }
    if (bridging_vertex == nullptr)
      continue;

    // TODO(aryap): Need a way to roll back this temporary objects in case the
    // caller's entire process fails - i.e. a vertex can be created for the
    // starting point but not for the ending point.

    // Success, so add a new vertex at this position and the bridging one too.
    if (bridging_vertex == candidate) {
      // The closest vertex was the candidate itself, no bridging vertex necessary.
      return bridging_vertex;
    }

    bridging_vertex->AddConnectedLayer(layer);
    AddVertex(bridging_vertex);

    RoutingVertex *off_grid = new RoutingVertex(point);
    off_grid->AddConnectedLayer(layer);
    AddVertex(off_grid);

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid);
    edge->set_layer(layer);
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

namespace {

// C++ modulo is more 'remainder' than 'modulo' because of how negative numbers
// are handled:
//    mod(-3, 5) = 2
//    rem(-3, 5) = -3 (since -3 / 5 = 0)
// So we have to do this:
int64_t modulo(int64_t a, int64_t b) {
  int64_t remainder = a % b;
  return remainder < 0 ? remainder + b : remainder;
}

}   // namespace

void RoutingGrid::ConnectLayers(
    const geometry::Layer &first, const geometry::Layer &second) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  const RoutingLayerInfo &horizontal_info = split_directions.first;
  const RoutingLayerInfo &vertical_info = split_directions.second;

  // Determine the area over which the grid is valid.
  geometry:: Rectangle overlap =
      horizontal_info.area.OverlapWith(vertical_info.area);
  LOG(INFO) << "Drawing grid between layers " << horizontal_info.layer
            << ", " << vertical_info.layer << " over " << overlap;
  
  //                      x_min v   v x_start
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //  origin   |      |      |  +   |      |
  //  O -----> | ---> | ---> | -+-> | ---> |
  //    offset   pitch          ^ start of grid boundary
  //
  int64_t x_offset = vertical_info.offset;
  int64_t x_pitch = vertical_info.pitch;
  LOG_IF(FATAL, x_pitch == 0)
      << "Routing pitch for layer " << vertical_info.layer << " is 0";
  int64_t x_min = overlap.lower_left().x();
  int64_t x_start = x_min + (x_pitch - modulo(x_min - x_offset, x_pitch));
  int64_t x_max = overlap.upper_right().x();
  
  int64_t y_offset = vertical_info.offset;
  int64_t y_pitch = vertical_info.pitch;
  LOG_IF(FATAL, y_pitch == 0)
      << "Routing pitch for layer " << horizontal_info.layer << " is 0";
  int64_t y_min = overlap.lower_left().y();
  int64_t y_start = y_min + (y_pitch - modulo(y_min - y_offset, y_pitch));
  int64_t y_max = overlap.upper_right().y();

  std::vector<RoutingVertex*> &first_layer_vertices =
      GetAvailableVertices(first);
  std::vector<RoutingVertex*> &second_layer_vertices =
      GetAvailableVertices(second);

  size_t num_vertices = 0;

  std::map<int64_t, RoutingTrack*> vertical_tracks;
  std::map<int64_t, RoutingTrack*> horizontal_tracks;

  // Generate tracks to hold edges and vertices in each direction.
  for (int64_t x = x_start; x < x_max; x += x_pitch) {
    RoutingTrack *track = new RoutingTrack(
        vertical_info.layer, RoutingTrackDirection::kTrackVertical, x);
    track->set_width(vertical_info.wire_width);
    vertical_tracks.insert({x, track});
    AddTrackToLayer(track, vertical_info.layer);
  }

  for (int64_t y = y_start; y < y_max; y += y_pitch) {
    RoutingTrack *track = new RoutingTrack(
        horizontal_info.layer,
        RoutingTrackDirection::kTrackHorizontal, y);
    track->set_width(horizontal_info.wire_width);
    horizontal_tracks.insert({y, track});
    AddTrackToLayer(track, horizontal_info.layer);
  }

  // Generate a vertex at the intersection of every horizontal and vertical
  // track.
  for (int64_t x = x_start; x < x_max; x += x_pitch) {
    // This (and the horizontal one) must exist by now, so we can make this
    // fatal.
    RoutingTrack *vertical_track = vertical_tracks.find(x)->second;
    LOG_IF(FATAL, !vertical_track) << "Vertical routing track is nullptr";

    for (int64_t y = y_start; y < y_max; y += y_pitch) {
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
      // TODO(aryap): Remove these.
    }
  }

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
    const std::string &net) {
  RoutingVertex *begin_vertex = GenerateGridVertexForPoint(
      begin.centre(), begin.layer());
  if (!begin_vertex) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    return false;
  }
  LOG(INFO) << "Nearest vertex to begin is " << begin_vertex->centre();

  RoutingVertex *end_vertex = GenerateGridVertexForPoint(
      end.centre(), end.layer());
  if (!end_vertex) {
    LOG(ERROR) << "Could not find available vertex for end port.";
    return false;
  }
  LOG(INFO) << "Nearest vertex to end is " << end_vertex->centre();

  std::unique_ptr<RoutingPath> shortest_path(
      ShortestPath(begin_vertex, end_vertex));

  if (!shortest_path) {
    LOG(WARNING) << "No path found.";
    return false;
  }

  // Remember the ports to which the path should connect.
  shortest_path->set_start_port(&begin);
  shortest_path->set_end_port(&begin);

  LOG(INFO) << "Found path: " << *shortest_path;

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
  LOG(INFO) << "Nearest vertex to begin is " << begin_vertex->centre();

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
      << path->edges().size() << " edges and " << path->vertices().size()
      << " vertices";

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
    ++i;
  }
  
  paths_.push_back(path);
}

RoutingPath *RoutingGrid::ShortestPath(
    RoutingVertex *begin, RoutingVertex *end) {
  // FIXME(aryap): This is very bad.
  if (!begin->available()) {
    LOG(WARNING) << "Start vertex for path is not available";
    return nullptr;
  }
  if (!end->available()) {
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
      if (!edge->available())
        continue;

      // We don't know what direction we're using the edge in, and edges are
      // not directional per se, so pick the side that isn't the one we came in
      // on:
      // TODO(aryap): Maybe bake this into the RoutingEdge.
      RoutingVertex *next =
          edge->first() == current ? edge->second() : edge->first();

      if (!next->available())
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
    int64_t padding) {
  for (const auto &rectangle : shapes.rectangles()) {
    AddBlockage(*rectangle);
  }
  for (const auto &polygon : shapes.polygons()) {
    AddBlockage(*polygon);
  }
  for (const auto &port : shapes.ports()) {
    AddBlockage(*port);
  }
}

void RoutingGrid::AddBlockage(const geometry::Rectangle &rectangle,
                              int64_t padding) {
  auto it = tracks_by_layer_.find(rectangle.layer());
  if (it == tracks_by_layer_.end())
    return;
  for (RoutingTrack *track : it->second) {
    track->AddBlockage(rectangle, padding);
  }
}

void RoutingGrid::AddBlockage(const geometry::Polygon &polygon,
                              int64_t padding) {
  auto it = tracks_by_layer_.find(polygon.layer());
  if (it == tracks_by_layer_.end())
    return;
  for (RoutingTrack *track : it->second) {
    track->AddBlockage(polygon, padding);
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

const RoutingViaInfo &RoutingGrid::GetRoutingViaInfo(
    const Layer &lhs, const Layer &rhs) const {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;

  const auto first_it = via_infos_.find(first);
  LOG_IF(FATAL, first_it == via_infos_.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
  const std::map<Layer, RoutingViaInfo> &inner_map = first_it->second;
  const auto second_it = inner_map.find(second);
  LOG_IF(FATAL, second_it == inner_map.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
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

void RoutingGrid::AddTrackToLayer(RoutingTrack *track, const geometry::Layer &layer) {
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

} // namespace bfg
