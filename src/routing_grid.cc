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
#include <utility>
#include <vector>

#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "physical_properties_database.h"
#include "poly_line.h"
#include "routing_grid.h"
#include "rectangle.h"

namespace bfg {

void RoutingEdge::set_track(RoutingTrack *track) {
  track_ = track;
  if (track_ != nullptr) set_layer(track_->layer());
}

const Layer &RoutingEdge::ExplicitOrTrackLayer() const {
  if (track_ != nullptr)
    return track_->layer();
  return layer_;
}


bool RoutingVertex::RemoveEdge(RoutingEdge *edge) {
  return edges_.erase(edge) > 0;
}

RoutingPath::RoutingPath(
    RoutingVertex *start, const std::deque<RoutingEdge*> edges)
    : edges_(edges.begin(), edges.end()) {
  vertices_.push_back(start);
  RoutingVertex *last = start;
  for (RoutingEdge *edge : edges) {
    RoutingVertex *next_vertex =
        edge->first() == last ? edge->second() : edge->first();
    vertices_.push_back(next_vertex);
    last = next_vertex;
  }
}

void RoutingPath::ToPolyLinesAndVias(
    const PhysicalPropertiesDatabase &physical_db,
    std::vector<std::unique_ptr<PolyLine>> *polylines,
    std::vector<std::unique_ptr<Via>> *vias) const {
  if (Empty())
    return;

  LOG_IF(FATAL, vertices_.size() != edges_.size() + 1)
      << "There should be one more vertex than there are edges.";
  std::unique_ptr<PolyLine> last;
  for (size_t i = 0; i < vertices_.size() - 1; ++i) {
    RoutingVertex *current = vertices_.at(i);
    RoutingEdge *edge = edges_.at(i);
    const Layer &layer = edge->ExplicitOrTrackLayer();

    const RoutingLayerInfo &info = physical_db.GetLayerInfo(layer);

    if (!last || last->layer() != layer) {
      Via *via = nullptr;
      if (last) {
        // This is a change in layer, so we finish the last line and store it.
        last->AddSegment(current->centre(), info.wire_width);
        via = new Via(current->centre(), last->layer(), layer);
        vias->emplace_back(via);
        last->set_end_via(via);
        polylines->push_back(std::move(last));
      }
      // Start a new line.
      last.reset(new PolyLine());
      last->set_layer(layer);
      last->set_start(current->centre());
      last->set_start_via(via);
      continue;
    }
    last->AddSegment(current->centre());
  }
  last->AddSegment(vertices_.back()->centre());
  polylines->push_back(std::move(last));

  // Copy pointers to the start and end ports, if any.
  if (!polylines->empty()) {
    polylines->front()->set_start_port(start_port_);
    polylines->front()->set_end_port(end_port_);
  }
}

std::ostream &operator<<(std::ostream &os, const RoutingPath &path) {
  if (path.Empty()) {
    os << "empty path";
    return os;
  }
  for (RoutingVertex *vertex : path.vertices()) {
    os << vertex->centre() << " ";
  }
  return os;
}

bool RoutingTrackBlockage::Contains(int64_t position) {
  return position >= start_ && position <= end_;
}

bool RoutingTrackBlockage::IsAfter(int64_t position) {
  return position <= start_;
}

bool RoutingTrackBlockage::IsBefore(int64_t position) {
  return position >= end_;
}

// Whether the given span [low, high] overlaps with this blockage.
bool RoutingTrackBlockage::Blocks(int64_t low, int64_t high) {
  return Contains(low) || Contains(high) || (low <= start_ && high >= end_);
}

bool RoutingTrack::RemoveEdge(RoutingEdge *edge, bool and_delete) {
  if (edges_.erase(edge) == 0)
    return false;

  // Remove the edge from the vertices on which it lands.
  edge->first()->RemoveEdge(edge);
  edge->second()->RemoveEdge(edge);
  edge->set_track(nullptr);
  if (and_delete)
    delete edge;
  return true;
}

bool RoutingTrack::MaybeAddEdgeBetween(
    RoutingVertex *one, RoutingVertex *the_other) {
  if (IsBlockedBetween(one->centre(), the_other->centre()))
    return false;
  RoutingEdge *edge = new RoutingEdge(one, the_other);
  edge->set_track(this);
  edge->first()->AddEdge(edge);
  edge->second()->AddEdge(edge);
  edges_.insert(edge);
  return true;
}

bool RoutingTrack::AddVertex(RoutingVertex *vertex) {
  LOG_IF(FATAL, IsBlocked(vertex->centre()))
      << "RoutingTrack cannot add vertex at " << vertex->centre()
      << ", it is blocked";
  LOG_IF(FATAL, vertices_.find(vertex) != vertices_.end())
      << "Duplicate vertex added to track";

  // Generate an edge between the new vertex and every other vertex, unless it
  // would be blocked.
  bool any_success = false;
  for (RoutingVertex *other : vertices_) {
    // We _don't want_ short-circuiting here.
    any_success |= MaybeAddEdgeBetween(vertex, other);
  }
  vertices_.insert(vertex);
  return any_success;
}

bool RoutingTrack::RemoveVertex(RoutingVertex *vertex) {
  if (vertices_.erase(vertex) == 0) {
    // We didn't know about this vertex.
    return false;
  }

  for (RoutingEdge *edge : edges_) {
    if (edge->first() == vertex || edge->second() == vertex)
      RemoveEdge(edge, true);
  }
  return true;
}

void RoutingTrack::MarkEdgeAsUsed(RoutingEdge *edge,
                                  std::set<RoutingVertex*> *removed_vertices) {
  if (edges_.find(edge) == edges_.end())
    // Possible off-grid edge?
    return;

  CreateBlockage(edge->first()->centre(), edge->second()->centre());

  // Remove the edge from our collection.
  // Ownership of this edge is transferred to the RoutingPath that owns it.
  RemoveEdge(edge, false);

  // Remove other edges that are blocked by this.
  for (RoutingEdge *edge : edges_) {
    if (IsBlockedBetween(edge->first()->centre(), edge->second()->centre())) {
      // Ownership of other blocked edges is not transferred; they are just
      // removed.
      RemoveEdge(edge, true);
    }
  }

  // Remove other vertices that are blocked by this.
  for (RoutingVertex *vertex : vertices_) {
    if (IsBlocked(vertex->centre()))
      removed_vertices->insert(vertex);
  }
}

RoutingVertex *RoutingTrack::CreateNearestVertexAndConnect(
    const Point &point,
    RoutingVertex *target) {
  // Candidate position:
  Point candidate_centre;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      candidate_centre = Point(point.x(), offset_);
      break;
    case RoutingTrackDirection::kTrackVertical:
      candidate_centre = Point(offset_, point.y());
      break;
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }

  if (candidate_centre == point) {
    return target;
  }

  if (IsBlocked(candidate_centre))
    return nullptr;

  if (IsBlockedBetween(candidate_centre, target->centre()))
    return nullptr;

  RoutingVertex *bridging_vertex = new RoutingVertex(candidate_centre);
  if (!AddVertex(bridging_vertex)) {
    LOG(FATAL) << "I thought we made sure this couldn't happen already.";
    delete bridging_vertex;
    return nullptr;
  }

  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      bridging_vertex->set_horizontal_track(this);
      break;
    case RoutingTrackDirection::kTrackVertical:
      bridging_vertex->set_vertical_track(this);
      break;
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }

  return bridging_vertex;
}

void RoutingTrack::ReportAvailableEdges(
    std::vector<RoutingEdge*> *edges_out) {
  std::copy_if(
      edges_.begin(),
      edges_.end(),
      edges_out->begin(),
      [](RoutingEdge* edge) { return edge->available(); });
}

void RoutingTrack::ReportAvailableVertices(
    std::vector<RoutingVertex*> *vertices_out) {
  std::copy_if(
      vertices_.begin(),
      vertices_.end(),
      vertices_out->begin(),
      [](RoutingVertex* vertex) { return vertex->available(); });
}

std::string RoutingTrack::Debug() const {
  std::stringstream ss;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      ss << "horizontal";
      break;
    case RoutingTrackDirection::kTrackVertical:
      ss << "vertical";
      break;
    default:
      ss << "unknown direction";
      break;
  }
  ss << " routing track offset=" << offset_
     //<< " start=" << start_
     //<< " end=" << end_
     << " #edges=" << edges_.size() << " #vertices="
     << vertices_.size();
  return ss.str();
}

bool RoutingTrack::IsBlockedBetween(
    const Point &one_end, const Point &other_end) const {
  int64_t low = ProjectOntoTrack(one_end);
  int64_t high = ProjectOntoTrack(other_end);
  if (low > high)
    std::swap(low, high);

  for (RoutingTrackBlockage *blockage : blockages_) {
    if (blockage->Blocks(low, high)) return true;
  }
  // Does not overlap, start or stop in any blockages.
  return false;
}

int64_t RoutingTrack::ProjectOntoTrack(const Point &point) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      return point.x();
    case RoutingTrackDirection::kTrackVertical:
      return point.y();
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }
  return 0;
}

RoutingTrackBlockage *RoutingTrack::CreateBlockage(
      const Point &one_end, const Point &other_end) {
  int64_t low = ProjectOntoTrack(one_end);
  int64_t high = ProjectOntoTrack(other_end);
  if (low > high)
    std::swap(low, high);

  if (blockages_.empty()) {
    RoutingTrackBlockage *blockage = new RoutingTrackBlockage(low, high);
    blockages_.push_back(blockage);
    // Already sorted!
    return blockage;
  }
  // RoutingTrackBlockages should already be sorted in ascending order of
  // position.
  //
  // TODO(aryap): I'm trying to find a range of consecutive values for which
  // some predicate is true. I can't find a helpful standard library
  // implementation that isn't just storing a bunch of iterators returned by
  // subsequent calls to std::find_if. But since we need an iterator to remove
  // elements from a vector, we have to store an iterator anyway.
  auto first = blockages_.end();
  auto last = blockages_.end();
  for (auto it = blockages_.begin(); it != blockages_.end(); ++it) {
    RoutingTrackBlockage *blockage = *it;
    if (blockage->Blocks(low, high)) {
      if (first == blockages_.end())
        first = it;
      else if (it == std::next(last)) {
        // Extend the last iterator only if it is consecutive.
        last = it;
      }
    }
  }

  if (first == blockages_.end()) {
    // If no blockages were spanned the new blockage stands alone.
    RoutingTrackBlockage *blockage = new RoutingTrackBlockage(low, high);
    blockages_.push_back(blockage);
    SortBlockages();
    return blockage;
  }

  if (last == blockages_.end()) {
    last = first;
  }

  // Remove elements [first, last] from blockages after combining them into one
  // blockage spanned by the new one. We rely on the sorted order of the blockages.
  RoutingTrackBlockage *blockage = new RoutingTrackBlockage(
      std::min(low, (*first)->start()),
      std::max(high, (*last)->end()));

  // Delete the old elements
  ++last;
  for (auto it = first; it != last; ++it)
    delete *it;
  blockages_.erase(first, last);

  blockages_.push_back(blockage);
  SortBlockages();
  return blockage;
}

void RoutingTrack::SortBlockages() {
  // Instead of declaring some
  //   static bool CompareAsLess(
  //       const RoutingTrackBlockage &a, const RoutingTrackBlockage &b) { ... };
  // and passing &CompareAsLess with type
  //       bool (*)(const RoutingTrackBlockage&, const RoutingTrackBlockage&)
  // we get to use mOdErN c++. TODO(aryap): But if we made this a class member
  // we wouldn't have to indirect through the getters/setters.
  static auto comp = [](RoutingTrackBlockage *lhs,
                        RoutingTrackBlockage *rhs) {
    return lhs->start() != rhs->start() ?
        lhs->start() < rhs->start() : lhs->end() < rhs->end();
  };
  std::sort(blockages_.begin(), blockages_.end(), comp);
}

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track) {
  os << track.Debug();
  return os;
}

uint64_t RoutingVertex::L1DistanceTo(const Point &point) {
  // The L-1 norm, or Manhattan distance.
  int64_t dx = point.x() - centre_.x();
  int64_t dy = point.y() - centre_.y();
  return std::abs(dx) + std::abs(dy);
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
        const Layer &lhs, const Layer &rhs) const {
  const RoutingLayerInfo &lhs_info = physical_db_.GetLayerInfo(lhs);
  const RoutingLayerInfo &rhs_info = physical_db_.GetLayerInfo(rhs);
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
    const Point &point, const Layer &layer) {
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
    const Layer &layer) {
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
  return remainder < 0? remainder + b : remainder;
}

}   // namespace

void RoutingGrid::ConnectLayers(
    const Layer &first, const Layer &second) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  const RoutingLayerInfo &horizontal_info = split_directions.first;
  const RoutingLayerInfo &vertical_info = split_directions.second;

  // Determine the area over which the grid is valid.
  Rectangle overlap = horizontal_info.area.OverlapWith(vertical_info.area);
  LOG(INFO) << "Drawing grid between layers " << horizontal_info.layer << ", "
            << vertical_info.layer << " over " << overlap;
  
  //                      x_min v   v x_start
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //  origin   |      |      |  +   |      |
  //  O -----> | ---> | ---> | -+-> | ---> |
  //    offset   pitch          ^ start of grid boundary
  //
  int64_t x_offset = horizontal_info.offset;
  int64_t x_pitch = horizontal_info.pitch;
  int64_t x_min = overlap.lower_left().x();
  int64_t x_start = x_min + (x_pitch - modulo(x_min - x_offset, x_pitch));
  int64_t x_max = overlap.upper_right().x();
  
  int64_t y_offset = vertical_info.offset;
  int64_t y_pitch = vertical_info.pitch;
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
    vertical_tracks.insert({x, track});
    AddTrackToLayer(track, vertical_info.layer);
  }

  for (int64_t y = y_start; y < y_max; y += y_pitch) {
    RoutingTrack *track = new RoutingTrack(
        horizontal_info.layer, RoutingTrackDirection::kTrackHorizontal, y);
    horizontal_tracks.insert({y, track});
    AddTrackToLayer(track, horizontal_info.layer);
  }

  // Generate a vertex at the intersection of every horizontal and vertical
  // track.
  for (int64_t x = x_start; x < x_max; x += x_pitch) {
    // This (and the horizontal one) must exist by now, so we can make this
    // fatal.
    RoutingTrack *vertical_track = vertical_tracks.find(x)->second;

    for (int64_t y = y_start; y < y_max; y += y_pitch) {
      RoutingTrack *horizontal_track = horizontal_tracks.find(y)->second;

      RoutingVertex *vertex = new RoutingVertex(Point(x, y));
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
    const Layer &layer = entry.first;
    for (RoutingTrack *track : entry.second) {
      VLOG(10) << layer << " track: " << *track;
    }
  }
}

void RoutingGrid::AddVertex(RoutingVertex *vertex) {
  for (const Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingVertex*> &available = GetAvailableVertices(layer);
    available.push_back(vertex);
  }
  vertices_.push_back(vertex);  // The class owns all of these.
}

bool RoutingGrid::AddRouteBetween(const Port &begin, const Port &end) {
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

  InstallPath(shortest_path.release());

  return true;
}

bool RoutingGrid::RemoveVertex(RoutingVertex *vertex, bool and_delete) {
  if (vertex->horizontal_track())
    vertex->horizontal_track()->RemoveVertex(vertex);
  if (vertex->vertical_track())
    vertex->vertical_track()->RemoveVertex(vertex);

  for (const Layer &layer : vertex->connected_layers()) {
    auto it = available_vertices_by_layer_.find(layer);
    if (it == available_vertices_by_layer_.end())
      continue;
    auto &available_vertices = it->second;
    auto pos = std::find(
        available_vertices.begin(), available_vertices.end(), vertex);
    LOG_IF(FATAL, pos == available_vertices.end())
        << "Did not find vertex we're removing in available ones for layer "
        << layer << "; vertex: " << vertex;
    available_vertices.erase(pos);
  }

  auto pos = std::find(vertices_.begin(), vertices_.end(), vertex);
  LOG_IF(FATAL, pos == vertices_.end())
      << "Did not find vertex we're removing in RoutingGrid list of "
      << "vertices_: " << vertex;
  vertices_.erase(pos);
  if (and_delete)
    delete vertex;
  return true; // TODO(aryap): Always returning true, huh...
}

void RoutingGrid::InstallPath(RoutingPath *path) {
  LOG_IF(FATAL, path->Empty()) << "Cannot install an empty path.";
  // Remove edges from the track which owns them.
  std::set<RoutingVertex*> unusable_vertices;
  for (RoutingEdge *edge : path->edges()) {
    if (edge->track() != nullptr)
      edge->track()->MarkEdgeAsUsed(edge, &unusable_vertices);
  }

  // Remove vertices from all of the tracks which reference them.
  for (RoutingVertex *vertex : path->vertices()) {
    unusable_vertices.erase(vertex);
    RemoveVertex(vertex, false);
  }

  for (RoutingVertex *vertex : unusable_vertices) {
    RemoveVertex(vertex, true);
  }
  
  paths_.push_back(path);
}

RoutingPath *RoutingGrid::ShortestPath(
    RoutingVertex *begin, RoutingVertex *end) {
  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  std::vector<double> cost(vertices_.size());

  // Recording the edge to take back to the start that makes the shortest path,
  // as well as the vertex it leads to. If RoutingEdge* is nullptr then this is
  // invalid.
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  // All vertices sorted according to their cost.
  std::vector<RoutingVertex*> queue;

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
  }

  queue.push_back(begin);

  while (!queue.empty()) {
    // Have to resort the queue so that new cost changes take effect. (The
    // queue is already mostly sorted so an insertion sort will be fast.)
    std::sort(queue.begin(), queue.end(),
              [&](RoutingVertex *a, RoutingVertex *b) {
      // We want the lowest value at the back of the array.
      return cost[a->contextual_index()] > cost[b->contextual_index()];
    });

    RoutingVertex *current = queue.back();
    queue.pop_back();
    size_t current_index = current->contextual_index();

    if (current == end) {
      break;
    }

    for (RoutingEdge *edge : current->edges()) {
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

        // Since we now have a faster way to get to this edge, we should visit it.
        queue.push_back(next);
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

void RoutingGrid::AddTrackToLayer(RoutingTrack *track, const Layer &layer) {
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
    path->ToPolyLinesAndVias(physical_db_, &cell->poly_lines(), &cell->vias());
  }
  return cell.release();
}

} // namespace bfg
