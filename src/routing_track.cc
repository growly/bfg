#include "routing_track.h"

#include <algorithm>
#include <queue>
#include <set>
#include <vector>

#include <absl/cleanup/cleanup.h>

#include "equivalent_nets.h"
#include "layout.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/radian.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_track_blockage.h"
#include "physical_properties_database.h"

namespace bfg {

int64_t RoutingTrack::ProjectOntoAxis(
    const geometry::Point &point, const RoutingTrackDirection &direction) {
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      return point.x();
    case RoutingTrackDirection::kTrackVertical:
      return point.y();
    default:
      LOG(FATAL) << "Unrecognised RoutingTrackDirection: " << direction;
      break;
  }
  return 0;
}

std::pair<int64_t, int64_t> RoutingTrack::ProjectOntoAxis(
    const geometry::Point &lhs,
    const geometry::Point &rhs,
    const RoutingTrackDirection &direction) {
  int64_t low = ProjectOntoAxis(lhs, direction);
  int64_t high = ProjectOntoAxis(rhs, direction);
  if (low > high)
    std::swap(low, high);
  return {low, high};
}

RoutingTrackDirection RoutingTrack::OrthogonalDirectionTo(
    const RoutingTrackDirection &direction) {
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      return RoutingTrackDirection::kTrackVertical;
    case RoutingTrackDirection::kTrackVertical:
      return RoutingTrackDirection::kTrackHorizontal;
    default:
      LOG(FATAL) << "Unrecognised RoutingTrackDirection: " << direction;
      break;
  }
  return RoutingTrackDirection::kTrackVertical;
}

double RoutingTrack::DirectionToAngle(const RoutingTrackDirection &direction) {
  switch (direction) {
    case RoutingTrackDirection::kTrackHorizontal:
      return 0.0;
    case RoutingTrackDirection::kTrackVertical:
      return geometry::Radian::kPi / 2;
    default:
      LOG(FATAL) << "Unknown RoutingTrackDirection: " << direction;
  }
  return 0.0;
}

RoutingTrackDirection RoutingTrack::AngleToDirection(
    double angle_to_horizon_rads) {
  if (angle_to_horizon_rads == 0.0) {
    return RoutingTrackDirection::kTrackHorizontal;
  } else if (angle_to_horizon_rads == geometry::Radian::kPi / 2) {
    return RoutingTrackDirection::kTrackVertical;
  }
  LOG(FATAL) << "Cannot convert angle to RoutingTrackDirection: "
             << angle_to_horizon_rads;
  return RoutingTrackDirection::kTrackHorizontal;
}

RoutingTrack::~RoutingTrack() {
  for (RoutingEdge *edge : edges_) { delete edge; }
  for (RoutingTrackBlockage *blockage : blockages_.vertex_blockages) {
    delete blockage;
  }
  for (RoutingTrackBlockage *blockage : blockages_.edge_blockages) {
    delete blockage;
  }
}

std::set<RoutingEdge*>::iterator RoutingTrack::RemoveEdge(
    const std::set<RoutingEdge*>::iterator &pos) {
  RoutingEdge *edge = *pos;
  edge->PrepareForRemoval();
  return edges_.erase(pos);
}

bool RoutingTrack::RemoveEdge(RoutingEdge *edge, bool and_delete) {
  if (edges_.erase(edge) == 0)
    return false;

  if (and_delete)
    delete edge;
  return true;
}

RoutingEdge *RoutingTrack::GetEdgeBetween(
    RoutingVertex *lhs, RoutingVertex *rhs) const {
  for (RoutingEdge *edge : edges_) {
    if ((edge->first() == lhs && edge->second() == rhs) ||
        (edge->first() == rhs && edge->second() == lhs)) {
      return edge;
    }
  }
  return nullptr;
}

bool RoutingTrack::MaybeAddEdgeBetween(
    RoutingVertex *one,
    RoutingVertex *the_other,
    const std::optional<EquivalentNets> &for_nets) {
  std::vector<RoutingTrackBlockage*> same_net_collisions;
  std::vector<RoutingTrackBlockage*> temporary_same_net_collisions;
  if (IsEdgeBlockedBetween(one->centre(),
                           the_other->centre(),
                           min_separation_to_new_blockages_,
                           for_nets,
                           &same_net_collisions,
                           &temporary_same_net_collisions))
    return false;
  RoutingEdge *edge = new RoutingEdge(one, the_other);
  edge->set_track(this);
  edge->set_layer(layer_);
  edge->first()->AddEdge(edge);
  edge->second()->AddEdge(edge);
  edges_.insert(edge);

  for (RoutingTrackBlockage *blockage : same_net_collisions) {
    ApplyEdgeBlockageToSingleEdge(*blockage,
                                  blockage->net(),
                                  false,
                                  edge);
  }

  for (RoutingTrackBlockage *blockage : temporary_same_net_collisions) {
    ApplyEdgeBlockageToSingleEdge(*blockage,
                                  blockage->net(),
                                  true,  // These ones are temporary.
                                  edge);
  }

  return true;
}

void RoutingTrack::HealEdges() {
  for (RoutingVertex *vertex : vertices_) {
    if (vertex->Available()) {
      continue;
    }
    std::vector neighbours = GetImmediateNeighbours(*vertex);
    if (neighbours.size() == 2) {
      MaybeAddEdgeBetween(neighbours.front(), neighbours.back(), {});
    }
  }
}

bool RoutingTrack::HealAroundBlockedVertex(const RoutingVertex &vertex) {
  if (vertex.Available()) {
    return false;
  }
  std::vector neighbours = GetImmediateNeighbours(vertex);
  if (neighbours.size() == 2) {
    return MaybeAddEdgeBetween(neighbours.front(), neighbours.back(), {});
  }
  return false;
}

std::vector<RoutingVertex*> RoutingTrack::GetImmediateNeighbours(
    const RoutingVertex &vertex,
    bool available_only) const {
  int64_t vertex_offset = ProjectOntoTrack(vertex.centre());
  if (vertices_by_offset_.empty()) {
    return {};
  }

  RoutingVertex *higher = nullptr;
  RoutingVertex *lower = nullptr;

  auto after = vertices_by_offset_.lower_bound(vertex_offset);
  auto before = after;

  for (;
       after != vertices_by_offset_.end();
       after = std::next(after)) {
    if (after->first == vertex_offset) {
      continue;
    }

    RoutingVertex *current = after->second;
    if (available_only && !current->Available()) {
      continue;
    }

    higher = current;
    break;
  }

  while (before != vertices_by_offset_.begin()) {
    before = std::prev(before);

    if (before->first == vertex_offset) {
      continue;
    }

    RoutingVertex *current = before->second;
    if (available_only && !current->Available()) {
      continue;
    }

    lower = current;
    break;
  } 

  std::vector<RoutingVertex*> neighbours;
  if (lower) {
    neighbours.push_back(lower);
  }
  if (higher) {
    neighbours.push_back(higher);
  }
  return neighbours;
}

bool RoutingTrack::AddVertex(
    RoutingVertex *vertex,
    const std::optional<EquivalentNets> &for_nets) {
  LOG_IF(FATAL, !Intersects(vertex))
      << "RoutingTrack " << Describe() << " cannot accommodate new vertex "
      << vertex->centre();
  LOG_IF(WARNING, IsBlocked(vertex->centre(), 0, for_nets))
      << "RoutingTrack cannot add vertex at " << vertex->centre()
      << ", it is blocked";
  LOG_IF(FATAL, ContainsVertex(vertex))
      << "Duplicate vertex added to track";
  int64_t vertex_offset = ProjectOntoTrack(vertex->centre());
  LOG_IF(FATAL, GetVertexAtOffset(vertex_offset) != nullptr)
      << "There already exists a vertex at offset " << vertex_offset;

  bool any_success = vertices_by_offset_.empty();

  if (edges_only_to_neighbours_) {
    std::vector<RoutingVertex*> neighbours = GetImmediateNeighbours(*vertex);
    for (RoutingVertex *other : neighbours) {
      any_success |= MaybeAddEdgeBetween(vertex, other, for_nets);
    }
  } else {
    // Generate an edge between the new vertex and every other vertex, unless it
    // would be blocked. If there are no other vertices to connect to, we are
    // successful by default.
    for (const auto &entry : vertices_by_offset_) {
      RoutingVertex *other = entry.second;
      // We _don't want_ short-circuiting here. Using the bitwise OR is correct
      // because bools are defined to be true or false, and it forces evaluation
      // of both operands every time.
      any_success |= MaybeAddEdgeBetween(vertex, other, for_nets);
    }
  }

  if (any_success) {
    // The vertex is not owned by this track but, in order to clean up correcly
    // if it is deleted, we add references back here.
    AssignThisTrackToVertex(vertex);
    vertices_by_offset_.insert({vertex_offset, vertex});
  }
  return any_success;
}

bool RoutingTrack::RemoveVertex(RoutingVertex *vertex) {
  int64_t vertex_offset = ProjectOntoTrack(vertex->centre());
  if (vertices_by_offset_.erase(vertex_offset) == 0) {
    // We didn't know about this vertex.
    return false;
  }

  // If we are in the regime where only neighbours are connected, deleting this
  // vertex will disconnect vertices on either side. We must re-establish an
  // edge between the immediate neighbours, if possible:
  if (edges_only_to_neighbours_) {
    std::vector<RoutingVertex*> neighbours = GetImmediateNeighbours(*vertex);
    if (neighbours.size() == 2) {
      // TODO(aryap): for_nets is what here?
      MaybeAddEdgeBetween(neighbours.front(), neighbours.back(), {});
    }
  }

  for (auto it = edges_.begin(); it != edges_.end(); ) {
    RoutingEdge *edge = *it;
    if (edge->first() == vertex || edge->second() == vertex) {
      VLOG(10) << "Removing edge " << edge
               << " because it includes vertex " << vertex;
      // This will remove the edge from the spanning_ set too.
      edge->PrepareForRemoval();
      delete edge;
      it = edges_.erase(it);
    } else {
      ++it;
    }
  }
  return true;
}

bool RoutingTrack::ContainsVertex(RoutingVertex *vertex) const {
  for (const auto &entry : vertices_by_offset_) {
    if (entry.second == vertex) {
      return true;
    }
  }
  return false;
}

bool RoutingTrack::Intersects(RoutingVertex *vertex) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      return vertex->centre().y() == offset_;
    case RoutingTrackDirection::kTrackVertical:
      return vertex->centre().x() == offset_;
    default:
      break;
  }
  LOG(FATAL) << "This track has an invalid direction: " << direction_;
  return false;
}

RoutingVertex *RoutingTrack::GetVertexAtOffset(int64_t offset) const {
  auto it = vertices_by_offset_.find(offset);
  if (it == vertices_by_offset_.end()) {
    return nullptr;
  }
  return it->second;
}

RoutingVertex *RoutingTrack::GetVertexAt(
    const geometry::Point &point) const {
  if (!IsPointOnTrack(point))
    return nullptr;
  int64_t position = ProjectOntoTrack(point);
  return GetVertexAtOffset(position);
}

bool RoutingTrack::IsPointOnTrack(const geometry::Point &point) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      if (point.y() != offset_) {
        return false;
      }
      break;
    case RoutingTrackDirection::kTrackVertical:
      if (point.x() != offset_) {
        return false;
      }
      break;
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }
  return true;
}

void RoutingTrack::MarkEdgeAsUsed(RoutingEdge *edge, const std::string &net) {
  edge->SetPermanentNet(net);
  //LOG(INFO) << "assigning edge " << *edge;

  if (edges_.find(edge) == edges_.end())
    // Possible off-grid edge?
    return;

  // TODO(aryap): This could be a problem because if the current edge merges
  // with an existing blockage, we will treat that blockage as touching this
  // net!

  // Record the vertex and edge blockage.
  MergeNewVertexBlockage(
      edge->first()->centre(),
      edge->second()->centre(),
      min_separation_between_edges_,
      net);

  RoutingTrackBlockage *current_blockage = MergeNewEdgeBlockage(
      edge->first()->centre(),
      edge->second()->centre(),
      min_separation_between_edges_,
      net);

  // Since we add a new blockage of strictly edge's size without any keep-out
  // padding, we are testing for edges that touch this one. Those edges must be
  // marked as 'in use' by the same net as this one, since they can still be
  // used to connect to the given net.
  for (RoutingEdge *other_edge : edges_) {
    if (other_edge == edge)
      continue;
    // FIXME: THIS IS NOT THE SAME AS "IS BLOCKED BY edge THAT WE JUST GOT"
    if (BlockageBlocks(*current_blockage,
                       other_edge->first()->centre(),
                       other_edge->second()->centre())) {
      if (other_edge->Blocked())
        continue;
      // If the edge touches two different nets, it cannot be used for either
      // and must be blocked.
      if (other_edge->PermanentNet() && *other_edge->PermanentNet() != net) {
        // Set permanent blockage on edge.
        other_edge->SetPermanentlyBlocked(true);
        other_edge->SetPermanentNet(std::nullopt);
      } else {
        other_edge->SetPermanentNet(net);
      }
    }
  }

  // Remove other vertices that are blocked by this.
  for (auto &entry : vertices_by_offset_) {
    RoutingVertex *vertex = entry.second;
    // We do _not_ set the in/out edge of the vertices at either end of the
    // given edge, we only set in/out for edges along the way.
    if (vertex != edge->first() && vertex != edge->second()) {
      if (EdgeSpansVertex(*edge, *vertex)) {
        vertex->AddEdges(edge, edge);
        vertex->AddUsingNet(net, false);   // Permanent.
      }
    }
  }
}

bool RoutingTrack::IsPerpendicularTo(const RoutingTrackDirection &other) const {
  LOG_IF(FATAL, direction_ != RoutingTrackDirection::kTrackHorizontal &&
                direction_ != RoutingTrackDirection::kTrackVertical)
    << "direction_ must be horizontal or vertical for the IsPerpendicularTo "
    << "test";
  LOG_IF(FATAL, other != RoutingTrackDirection::kTrackHorizontal &&
                other != RoutingTrackDirection::kTrackVertical)
    << "other direction must be horizontal or vertical for the "
    << "IsPerpendicularTo test";
  // If we can assume that there are only two directions, then perpendicular
  // tracks just have different directions.
  return direction_ != other;
}

void RoutingTrack::AssignThisTrackToVertex(RoutingVertex *vertex) {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      vertex->set_horizontal_track(this);
      break;
    case RoutingTrackDirection::kTrackVertical:
      vertex->set_vertical_track(this);
      break;
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }
}

// FIXME: there are two uses of this function that differ in what 'target'
// means. FIX. 
//  - in the earlier use case 'target' was the nearest vertex on the track to
//  which the off-grid point was inteded to connect
//  - in the later user case 'target' is just the vertex at 'point'
// The function is confusing even though it seems to work because of these
// differences. But remember we're trying to solve whether a bridging vertex,
// which would be created and added to the track internally to this function,
// collides with an existing vertex and so isn't necessary. But what if the
// given 'point' collides with a given vertex and isn't necessary? I think the
// action might dependon which version of 'target' we use.
//
// Create a vertex at the point on this track nearest to 'point', with the vague
// intention of connecting it to 'target'. 
//
// In all cases where 'point' doesn't end up being on the track already we have
// to create a bridging vertex that is on the track and that can be used to
// connect to 'point' with an off-grid edge (handled by the caller).
//
// Sometimes, 'point' lands directly on or requires a bridging_vertex at an
// existing vertex. In those cases we do not need to add a new vertex, but we
// need to return the existing one to the caller.
bool RoutingTrack::CreateNearestVertexAndConnect(
    const RoutingGrid &grid,
    RoutingVertex *target,
    const geometry::Layer &target_layer,
    const EquivalentNets &for_nets,
    RoutingVertex **connecting_vertex,
    bool *bridging_vertex_is_new,
    bool *target_already_exists) {
  *connecting_vertex = nullptr;
  *bridging_vertex_is_new = false;
  *target_already_exists = false;

  const geometry::Point &target_point = target->centre();

  // Candidate position:
  geometry::Point candidate_centre;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      candidate_centre = geometry::Point(target_point.x(), offset_);
      break;
    case RoutingTrackDirection::kTrackVertical:
      candidate_centre = geometry::Point(offset_, target_point.y());
      break;
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }

  if (IsBlocked(candidate_centre, 0, for_nets)) {
    return false;
  }
  // TODO(aryap): This need a net check if it's reinstated:
  //if (IsProbablyBlockedForVia(candidate_centre)) {
  //  return false;
  //}

  int64_t candidate_position = ProjectOntoTrack(candidate_centre);
  RoutingVertex *existing_vertex = GetVertexAtOffset(candidate_position);

  RoutingVertex *bridging_vertex = nullptr;

  if (candidate_centre == target_point) {
    // The target is on the track so we don't need to create a separate
    // bridging vertex. Connect the target to the track. That is unless the
    // target coincides with an existing vertex, in which case we flag this and
    // use the existing vertex in its place.
    if (existing_vertex) {
      *connecting_vertex = existing_vertex;
      *target_already_exists = true;
      return true;
    }

    bridging_vertex = target;
  }

  LOG_IF(WARNING, !target && candidate_centre == target_point)
      << "Nearest point to " << target_point << " on " << *this << " is "
      << candidate_centre << " itself, but no target vertex was given";

  if (!bridging_vertex) {
    if (existing_vertex) {
      // Don't need to add a new bridging vertex at the given position, and
      // don't need to set target_already_exists true since the bridging
      // vertex doesn't coincide with `target`. So also don't need to add any
      // new vertices and create any new edges.
      *connecting_vertex = existing_vertex;
      return true;
    }

    *bridging_vertex_is_new = true;
    bridging_vertex = MakeAndCheckVertexAt(
        grid, candidate_centre, target_layer, for_nets);
    if (!bridging_vertex) {
      return false;
    }
  }

  if (!AddVertex(bridging_vertex, for_nets)) {
    RemoveVertex(bridging_vertex);
    return false;
  }

  *connecting_vertex = bridging_vertex;
  return true;
}

RoutingVertex *RoutingTrack::CreateNewVertexAndConnect(
    const RoutingGrid &grid,
    const geometry::Point &candidate_centre,
    const geometry::Layer &target_layer,
    const EquivalentNets &for_nets) {
  if (!IsPointOnTrack(candidate_centre)) {
    return nullptr;
  }

  RoutingVertex *existing_vertex = GetVertexAt(candidate_centre);
  if (existing_vertex) {
    return nullptr;
  }

  RoutingVertex *validated_vertex = MakeAndCheckVertexAt(
      grid, candidate_centre, target_layer, for_nets);
  if (!validated_vertex) {
    return nullptr;
  }

  if (!AddVertex(validated_vertex, for_nets)) {
    RemoveVertex(validated_vertex);
    return nullptr;
  }

  return validated_vertex;
}

RoutingVertex *RoutingTrack::MakeAndCheckVertexAt(
    const RoutingGrid &grid,
    const geometry::Point &point,
    const geometry::Layer &target_layer,
    const EquivalentNets &for_nets) {
  // We need to ask if this candidate fits in with other installed vertices.
  // This is specifically to check that vertices on adjacent tracks do not
  // violate spacing rules. The track itself only ensures correct spacing
  // along its dimension. Consider these horizontal tracks:
  //
  // -------------A----------------
  //
  //             +-----+
  // ------------|--B--|-----------
  //             +-----+
  //
  // The candidate x might collide with the existing B on the neighbouring
  // track.
  std::unique_ptr<RoutingVertex> added_vertex(new RoutingVertex(point));
  added_vertex->AddConnectedLayer(layer_);
  if (target_layer != layer_) {
    added_vertex->AddConnectedLayer(target_layer);
  }
  absl::Status valid_against_installed_paths =
      grid.ValidAgainstInstalledPaths(*added_vertex, for_nets);
  if (!valid_against_installed_paths.ok()) {
    LOG(WARNING) << "New vertex " << added_vertex->centre()
                 << " on " << Describe()
                 << " is not valid against other installed paths: "
                 << valid_against_installed_paths.message();
    return nullptr;
  }
  return added_vertex.release();
}

void RoutingTrack::ReportAvailableEdges(
    std::vector<RoutingEdge*> *edges_out) const {
  std::copy_if(
      edges_.begin(),
      edges_.end(),
      edges_out->begin(),
      [](RoutingEdge* edge) { return edge->Available(); });
}

void RoutingTrack::ReportAvailableVertices(
    std::vector<RoutingVertex*> *vertices_out) const {
  for (const auto &entry : vertices_by_offset_) {
    RoutingVertex *vertex = entry.second;
    if (vertex->Available()) {
      vertices_out->push_back(vertex);
    }
  }
}

void RoutingTrack::ExportEdgesAsRectangles(
    const std::string &layer,
    bool available_only,
    Layout *layout) const {
  const int64_t kPadding = 2;
  for (RoutingEdge *edge : edges_) {
    if (available_only && edge->Blocked())
      continue;
    auto rectangle = edge->AsRectangle(kPadding);
    if (!rectangle)
      continue;
    layout->AddRectangle(*rectangle);
  }
}

std::string RoutingTrack::Describe() const {
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
  ss << " routing track offset=" << offset_;
     //<< " start=" << start_
     //<< " end=" << end_
     //<< " #edges=" << edges_.size() << " #vertices="
     //<< vertices_by_offset_.size();
  return ss.str();
}

// Returns a horizontal or vertical line at the given offset depending on
// whether the track is horizontal or vertical, respectively.
geometry::Line RoutingTrack::ParallelLineAtOffset(int64_t offset) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      return geometry::Line({0, offset}, {1, offset});
    case RoutingTrackDirection::kTrackVertical:
      // Falthrough intended.
    default:
      return geometry::Line({offset, 0}, {offset, 1});
  }
}

// TODO(aryap): Do we need this any more? Only used in one place, and that
// place more generically calls ParallelLineAtOffset already...
std::pair<geometry::Line, geometry::Line> RoutingTrack::MajorAxisLines(
    int64_t padding) const {
  geometry::Line low = ParallelLineAtOffset(offset_ - padding);
  geometry::Line high = ParallelLineAtOffset(offset_ + padding);
  return {low, high};
}

geometry::Line RoutingTrack::AsLine() const {
  geometry::Point start;
  geometry::Point end;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      start = geometry::Point(0, offset_);
      end = geometry::Point(1, offset_);
      break;
    case RoutingTrackDirection::kTrackVertical:
      start = geometry::Point(offset_, 0);
      end = geometry::Point(offset_, 1);
      break;
    default:
      LOG(FATAL) << "RoutingTrack has unknown direction " << direction_;
      break;
  }
  return geometry::Line(start, end);
}

// Find the point along the track at the given value on the track's major axis.
// e.g. a horizontal track would be given x = 10 and return the Point with x =
// 10, y = offset_. (This is also "projecting" onto the track in a way but I
// didn't want to overload it.)
geometry::Point RoutingTrack::PointOnTrack(
    int64_t projection_onto_track) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      return geometry::Point(projection_onto_track, offset_);
    case RoutingTrackDirection::kTrackVertical:
      return geometry::Point(offset_, projection_onto_track);
    default:
      LOG(FATAL) << "RoutingTrack has unknown direction " << direction_;
      break;
  }
  return geometry::Point();
}

std::pair<int64_t, int64_t> RoutingTrack::ProjectOntoTrack(
    const geometry::Point &lhs, const geometry::Point &rhs) const {
  return ProjectOntoAxis(lhs, rhs, direction_);
}

std::pair<int64_t, int64_t> RoutingTrack::ProjectOntoOffset(
    const geometry::Point &lhs, const geometry::Point &rhs) const {
  return ProjectOntoAxis(lhs, rhs, OrthogonalDirectionTo(direction_));
}

bool RoutingTrack::EdgeSpansVertex(
    const RoutingEdge &edge, const RoutingVertex &vertex) const {
  int64_t pos = ProjectOntoTrack(vertex.centre());
  auto points = ProjectOntoTrack(
      edge.first()->centre(), edge.second()->centre());
  int64_t low = points.first;
  int64_t high = points.second;

  return low <= pos && pos <= high;
}

std::vector<RoutingVertex*> RoutingTrack::VerticesInSpan(
    const geometry::Point &one_end,
    const geometry::Point &other_end) const {
  auto low_high = ProjectOntoTrack(one_end, other_end);

  // Since we don't have a geometric index for the vertex list, we have to check
  // every single one, O(n). So we might as well do it this way.
  auto vertex_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    int64_t projection_a = ProjectOntoTrack(a->centre());
    int64_t projection_b = ProjectOntoTrack(b->centre());
    return projection_a < projection_b;
  };
  std::priority_queue<RoutingVertex*,
                      std::vector<RoutingVertex*>,
                      decltype(vertex_sort_fn)> spanned(vertex_sort_fn);
  for (const auto &entry : vertices_by_offset_) {
    RoutingVertex *vertex = entry.second;
    int64_t position = entry.first;
    if (position >= low_high.first && position <= low_high.second) {
      spanned.push(vertex);
    }
  }
  // TODO(aryap): With internet: is there a nicer API for this?
  std::vector<RoutingVertex*> spanned_as_vector;
  while (!spanned.empty()) {
    spanned_as_vector.push_back(spanned.top());
    spanned.pop();
  }
  return spanned_as_vector;
}

bool RoutingTrack::BlockageBlocks(
    const RoutingTrackBlockage &blockage,
    const geometry::Point &one_end,
    const geometry::Point &other_end,
    int64_t margin) const {
  auto low_high = ProjectOntoTrack(one_end, other_end);

  low_high.first -= (margin - 1);
  low_high.second += (margin - 1);

  return blockage.Blocks(low_high.first, low_high.second);
}

bool RoutingTrack::IsProbablyBlockedForVia(const geometry::Point &point,
                                           int64_t margin) const {
  int64_t point_on_track = ProjectOntoTrack(point);
  // On the straight line of the track we can only ever fall between two
  // vertices, or on top of one, in which case we check that one and the two
  // neighbours. But as usual it's easier to just do an O(n) loop through the
  // vertices_by_offset_ list than to do any pre-sorting or filtering.
  for (const auto &entry : vertices_by_offset_) {
    int64_t track_position = entry.first;
    RoutingVertex *vertex = entry.second;
    int64_t spacing = std::max(
        std::abs(track_position - point_on_track) - margin, 0L);
    if (!vertex->Available() && spacing < pitch_) {
      VLOG(13) << "point " << point << " not suitable on " << *this
               << " because " << vertex->centre() << " is " << spacing
               << " away";
      return true;
    }
  }
  return false;
}

bool RoutingTrack::IsVertexBlocked(
    const geometry::Point &point,
    int64_t margin,
    const std::optional<EquivalentNets> &for_nets) const {
  int64_t low = ProjectOntoTrack(point);
  int64_t high = ProjectOntoTrack(point);

  if (low > high)
    std::swap(low, high);

  low -= (margin - 1);
  high += (margin - 1);

  for (RoutingTrackBlockage *blockage : blockages_.vertex_blockages) {
    if (blockage->Blocks(low, high) && (
          !for_nets || !for_nets->Contains(blockage->net()))) {
      return true;
    }
  }
  for (RoutingTrackBlockage *blockage : temporary_blockages_.vertex_blockages) {
    if (blockage->Blocks(low, high) && (
          !for_nets || !for_nets->Contains(blockage->net()))) {
      return true;
    }
  }
  return false;
}

bool RoutingTrack::IsEdgeBlockedBetween(
    const geometry::Point &one_end,
    const geometry::Point &other_end,
    int64_t margin,
    const std::optional<EquivalentNets> &for_nets,
    std::vector<RoutingTrackBlockage*> *same_net_collisions,
    std::vector<RoutingTrackBlockage*> *temporary_same_net_collisions) const {
  int64_t low = ProjectOntoTrack(one_end);
  int64_t high = ProjectOntoTrack(other_end);

  if (low > high)
    std::swap(low, high);

  low -= (margin - 1);
  high += (margin - 1);

  for (RoutingTrackBlockage *blockage : blockages_.edge_blockages) {
    if (!blockage->Blocks(low, high)) {
      // No problem.
      continue;
    }
    if (!for_nets) {
      return true;
    } else if (!for_nets->Contains(blockage->net())) {
      return true;
    } else if (same_net_collisions) {
      // The blockage applies to the edge, but since nets are defined and the
      // nets match, we don't treat it as a block. We have to report the
      // collisions though.
      same_net_collisions->push_back(blockage);
    }
  }

  for (RoutingTrackBlockage *blockage : temporary_blockages_.edge_blockages) {
    if (!blockage->Blocks(low, high)) {
      continue;
    }
    if (!for_nets) {
      return true;
    } else if (!for_nets->Contains(blockage->net())) {
      return true;
    } else if (temporary_same_net_collisions) {
      temporary_same_net_collisions->push_back(blockage);
    }
  }

  // Does not overlap, start or stop in any blockages.
  return false;
}

int64_t RoutingTrack::ProjectOntoTrack(const geometry::Point &point) const {
  return ProjectOntoAxis(point, direction_);
}

// Get the x- or y-coordinate of the given point if this is a vertical or
// horizontal, respectively.
int64_t RoutingTrack::ProjectOntoOffset(const geometry::Point &point) const {
  return ProjectOntoAxis(point, OrthogonalDirectionTo(direction_));
}

// Given a rectangle and this track (the line):
//
//        +-------------+             y = offset_axis_high
//        |             |
//        +-------------+             y = offset_axis_low
//                         +---+
//    ---------------------+   +--    y = high
//    ---------------------+   +-     y = low
//                         +---+
//                           ^ vertex position
//
// The y-axis is the offset axis because the track runs horizontally.
//
// TODO(growly): Track lines aren't actually infinite. We need to make sure
// shapes outside of the routing grid are not accidentally counted as
// intersections.
bool RoutingTrack::Intersects(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    int64_t min_transverse_separation) const {
  // First check that the minor direction falls on this offset:
  int64_t offset_axis_low = ProjectOntoOffset(rectangle.lower_left());
  int64_t offset_axis_high = ProjectOntoOffset(rectangle.upper_right());

  if (offset_axis_low > offset_axis_high)
    std::swap(offset_axis_low, offset_axis_high);

  bool intersected_vertices = false;
  bool intersected_edges = false;

  int64_t low = offset_ - (min_transverse_separation - 1) - padding;
  int64_t high = offset_ + (min_transverse_separation - 1) + padding;

  // There is no intersection if both the track edges are on the low or the
  // high side of the blockage. Otherwise if one of the edges is straddled or
  // we're entirely within the shape, there is:
  return !((low < offset_axis_low && high < offset_axis_low) ||
           (low > offset_axis_high && high > offset_axis_high));
}

// Given a polygon and this track (the line):
//
//        +-------------+
//        |             |
//        +--------+    |
//                 |    |
//                 +----+
//                         +---+
//    ---------------------+   +--    y = high
//    ---------------------+   +-     y = low
//                         +---+
//                           ^ vertex position
//
// This is not a generic way to determine if a polygon and a rectangle
// intersect. That would be more sophisticated. This is a rudimentary way to
// tell if, for our purposes, there is an intersection issue between the too.
// Practically that means we only check the major axis of the track for
// intersection with the polygon, and we assume the track is never fully
// contained by or fully contains the polygon.
bool RoutingTrack::Intersects(
    const geometry::Polygon &polygon,
    std::vector<geometry::PointPair> *intersections,
    int64_t padding,
    int64_t min_transverse_separation) const {
  // FIXME(aryap): This should be width_ / 2, or at least consider the actual
  // maximum thickness (still divided by 2) at vertices, wherever they are.
  //
  // Also, we actually need to find the maximum width of the polygon within the
  // band of the track, which these major axis lines will not give us. If the
  // polygon expands and contracts within the lines we don't detect it:
  //
  // ----------|-----|-----------
  //         |          | <- undetected
  // ----------|-----|-----------
  int64_t boundary_from_offset = min_transverse_separation + padding - 1; 
  std::pair<geometry::Line, geometry::Line> major_axis_lines =
      MajorAxisLines(boundary_from_offset);

  // If the polygon is entirely internal to the track, we will not find any
  // intersections. Project the bounding box of the polygon onto the track's
  // offset to check:
  geometry::Rectangle polygon_bounding_box = polygon.GetBoundingBox();
  std::pair<int64_t, int64_t> polygon_onto_offset = ProjectOntoOffset(
      polygon_bounding_box.lower_left(), polygon_bounding_box.upper_right());
  if (polygon_onto_offset.first >= ProjectOntoOffset(
          major_axis_lines.first.start()) &&
      polygon_onto_offset.second <= ProjectOntoOffset(
          major_axis_lines.second.start())) {
    std::pair<int64_t, int64_t> polygon_onto_track = ProjectOntoTrack(
        polygon_bounding_box.lower_left(), polygon_bounding_box.upper_right());
    intersections->push_back({
        PointOnTrack(polygon_onto_track.first),
        PointOnTrack(polygon_onto_track.second)});
    VLOG(15) << "blockage " << polygon.Describe()
             << " is entirely contained within track (" << direction_ << ") "
             << offset_ << " between " << polygon_onto_track.first << " and "
             <<polygon_onto_track.second;
    return true;
  }

  // Why is this so complicated? Consider:
  //
  // --------------------------------------------------------- major axis line -
  //     +---------------------------+ <- some stupid polygon
  // - - | - - - - - - - - - - - - - | - - - - - - - - - - - - - - - track ctr.
  //     +------------------------+  |
  // -----------------------------|--|------------------------ major axis line -
  //                              |  |
  //                              +--+
  //
  // We want to know the union of points, on the track centre line, where the
  // stupid polygon comes too close.
  //
  // This is conceptually solved by a general polygon-rectangle intersection
  // then projected onto the centre line, but at this point using a real
  // geometry library or investing any more time into usable routines is not an
  // option. So we need a compromise that is just good enough.
  //
  // Since the problem is only those polygons that are larger within the track
  // bounds than without, we just have to add tests for intersection with the
  // polygon at each of the y-values of its vertices within the those bounds.
  //
  // (A sketch of the proof for why this is correct is that the widest point on
  // the polygon within the rectangle must be a vertex, because if it is not,
  // then it is some point on a line leading to a vertex outside the rectangle.
  // The min/max axis lines would then intersect it and it would be captured.)

  int64_t low_axis_offset = ProjectOntoOffset(major_axis_lines.first.start());
  int64_t high_axis_offset = ProjectOntoOffset(major_axis_lines.second.start());
  DCHECK(low_axis_offset <= high_axis_offset)
      << "These need to be swapped sometimes";

  std::map<int64_t, geometry::Line> test_lines;
  test_lines.insert({low_axis_offset, major_axis_lines.first});
  test_lines.insert({high_axis_offset, major_axis_lines.second});
  for (const geometry::Point &point : polygon.vertices()) {
    int64_t projection = ProjectOntoOffset(point);
    if (projection < low_axis_offset ||
        projection > high_axis_offset) {
      continue;
    }
    if (test_lines.find(projection) != test_lines.end()) {
      // We're already going to test this line.
      continue;
    }
    geometry::Line line = ParallelLineAtOffset(projection);
    test_lines.insert({projection, line});
  }

  static auto comp = [](const geometry::PointPair &lhs,
                 const geometry::PointPair &rhs) {
    if (lhs.first == rhs.first) {
      return lhs.first < rhs.first;
    }
    return lhs.second < rhs.second;
  };

  // Find and de-dupe intersections, then return. Someone else will take care
  // of merging the intervals.
  auto deduped = std::set<
      std::pair<geometry::Point, geometry::Point>, decltype(comp)>(comp);
  for (const auto &entry : test_lines) {
    const geometry::Line &test_line = entry.second;
    std::vector<geometry::PointPair> intersections =
        polygon.IntersectingPoints(test_line);
    deduped.insert(intersections.begin(), intersections.end());
  }
  intersections->insert(intersections->end(), deduped.begin(), deduped.end());

  return !deduped.empty();
}


void RoutingTrack::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    const std::string &net,
    RoutingTrackBlockage **new_vertex_blockage,
    RoutingTrackBlockage **new_edge_blockage) {
  if (IntersectsVertices(rectangle, padding)) {
    RoutingTrackBlockage *blockage = MergeNewVertexBlockage(
        rectangle.lower_left(),
        rectangle.upper_right(),
        min_separation_between_edges_ + padding,
        net);
    if (blockage) {
      ApplyVertexBlockage(*blockage, rectangle.net());
    }
    if (new_vertex_blockage) {
      *new_vertex_blockage = blockage;
    }
  }
  if (IntersectsEdges(rectangle, padding)) {
    RoutingTrackBlockage *blockage = MergeNewEdgeBlockage(
        rectangle.lower_left(),
        rectangle.upper_right(),
        min_separation_between_edges_ + padding,
        net);
    if (blockage) {
      ApplyEdgeBlockage(*blockage, rectangle.net());
    }
    if (new_edge_blockage) {
      *new_edge_blockage = blockage;
    }
  }
}

void RoutingTrack::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding,
    const std::string &net) {
  //LOG(INFO) << "Adding polygon blockage to routing track " << offset_
  //          << " padding=" << padding << ": " << polygon.Describe();
  geometry::Line track = AsLine();

  std::vector<geometry::PointPair> intersections;
  IntersectsVertices(polygon, &intersections, padding);

  for (const auto &pair : intersections) {
    RoutingTrackBlockage *blockage = MergeNewVertexBlockage(
        pair.first,
        pair.second,
        min_separation_between_edges_ + padding,
        net);
    if (blockage) {
      ApplyVertexBlockage(*blockage, polygon.net());
    }
  }

  intersections.clear();
  IntersectsEdges(polygon, &intersections, padding);

  for (const auto &pair : intersections) {
    RoutingTrackBlockage *blockage = MergeNewEdgeBlockage(
        pair.first,
        pair.second,
        min_separation_between_edges_ + padding,
        net);
    if (blockage) {
      ApplyEdgeBlockage(*blockage, polygon.net());
    }
  }
}

// There is no merge process for temporary blockages because they are owned by
// a RoutingGrid; whatever causes the blockage to be created must be able to
// remove it independently of other temporary blockages.
void RoutingTrack::AddTemporaryBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    const std::string &net,
    RoutingTrackBlockage **new_vertex_blockage,
    RoutingTrackBlockage **new_edge_blockage,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  if (IntersectsVertices(rectangle, padding)) {
    std::pair<int64_t, int64_t> low_high = ProjectOntoTrack(
        rectangle.lower_left(), rectangle.upper_right());

    RoutingTrackBlockage *temporary_blockage = new RoutingTrackBlockage(
        low_high.first, low_high.second, net);
    temporary_blockages_.vertex_blockages.push_back(temporary_blockage);
    ApplyVertexBlockage(*temporary_blockage,
                        rectangle.net(),
                        true,   // Temporary.
                        blocked_vertices);
    *new_vertex_blockage = temporary_blockage;
  } else {
    *new_vertex_blockage = nullptr;
  }
  if (IntersectsEdges(rectangle, padding)) {
    std::pair<int64_t, int64_t> low_high = ProjectOntoTrack(
        rectangle.lower_left(), rectangle.upper_right());

    RoutingTrackBlockage *temporary_blockage = new RoutingTrackBlockage(
        low_high.first, low_high.second, net);
    temporary_blockages_.edge_blockages.push_back(temporary_blockage);
    ApplyEdgeBlockage(*temporary_blockage,
                      rectangle.net(),
                      true,   // Temporary.
                      blocked_edges);
    *new_edge_blockage = temporary_blockage;
  } else {
    *new_edge_blockage = nullptr;
  }
}

RoutingTrackBlockage *RoutingTrack::MergeNewBlockage(
    const geometry::Point &one_end,
    const geometry::Point &other_end,
    int64_t margin,
    const std::string &net,
    std::vector<RoutingTrackBlockage*> *container) {
  std::pair<int64_t, int64_t> low_high = ProjectOntoTrack(one_end, other_end);
  int64_t low = low_high.first;
  int64_t high = low_high.second;

  if (container->empty()) {
    RoutingTrackBlockage *blockage = new RoutingTrackBlockage(low, high, net);
    container->push_back(blockage);
    // Already sorted!
    return blockage;
  }

  // FIXME(aryap): Generalising this to also account for net names effectively
  // means that blockages can exist on top of each other. So, for a given net,
  // we maintain the idea that overlapping blockages are merged, but we do not
  // merge blockages on dissimilar nets. So the list of blockages is no longer a
  // sorted list of disjoint blockages, but a sorted list of
  // possibly-overlapping blockages. What does this mean...?

  // RoutingTrackBlockages should already be sorted in ascending order of
  // position.
  //
  // We will merge the given obstruction into an existing blockage if we falling
  // within `margin` of one.
  //
  // TODO(aryap): I'm trying to find a range of consecutive values for which
  // some predicate is true. I can't find a helpful standard library
  // implementation that isn't just storing a bunch of iterators returned by
  // subsequent calls to std::find_if. But since we need an iterator to remove
  // elements from a vector, we have to store an iterator anyway.

  // The goal here is to merge as many blockages as possible. Blockages can be
  // merged if:
  //  - they overlap within margin (exclusive); and
  //  - they have the same net label.
  //  - TODO(aryap): they have the same temporariness (true or false).
  //
  // As a reminder, we treat margin as the minimum separation that is allowed
  // between objects, and since they appear on a discrte unit grid, we subtract
  // 1 to exclude the end of the span in the collision check:
  //
  //     object end
  //     |
  //     | minimum sep. = 5
  //     V ----------------->|
  // .   .   .   .   .   .   .   .
  // ____.   .   .   .   .   .____
  // .   |   .   .   .   .   |   . 
  // .   |   .   .   .   .   |   .
  //     ^                   ^
  //     | ------------->|   `low` value of next blockage
  //     | span to check for collisions = minimum separation - 1.
  //     |
  //     `high` value of left blockage
  std::optional<std::pair<int64_t, int64_t>> span = std::nullopt;
  for (auto it = container->begin(); it != container->end();) {
    RoutingTrackBlockage *blockage = *it;
    if (blockage->net() == net &&
        blockage->Blocks(low - (margin - 1), high + (margin - 1))) {
      // Since blockages are contiguous and arranged in increasing order (by
      // start then end position), every blockage we collide with is the
      // right-most blockage we have to include in the merge. We also guarantee
      // that blockages of the same net do not overlap.
      if (!span) {
        span = std::pair<int64_t, int64_t>(
            std::min(blockage->start(), low), std::max(blockage->end(), high));
      } else {
        span->second = std::max(blockage->end(), high);
      }
      // Whatever existing blockages we collide with will be replaced, so remove
      // them now.
      it = container->erase(it);
      continue;
    }
    ++it;
  }

  if (!span) {
    // If no blockages were spanned the new blockage stands alone.
    RoutingTrackBlockage *blockage = new RoutingTrackBlockage(low, high, net);
    container->push_back(blockage);
    SortBlockages(container);
    return blockage;
  }

  // Remove elements [first, last] that match the new net from blockages after
  // combining them into one blockage spanned by the new one. We rely on the
  // sorted order of the blockages.
  RoutingTrackBlockage *blockage = new RoutingTrackBlockage(
      span->first, span->second, net);

  container->push_back(blockage);
  SortBlockages(container);
  return blockage;
}

void RoutingTrack::SortBlockages(
    std::vector<RoutingTrackBlockage*> *container) {
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
  std::sort(container->begin(), container->end(), comp);
}

bool RoutingTrack::RemoveTemporaryBlockage(RoutingTrackBlockage *blockage) {
  std::vector<RoutingTrackBlockage*> &vertex_blockages =
      temporary_blockages_.vertex_blockages;
  auto it = std::find(
      vertex_blockages.begin(), vertex_blockages.end(), blockage);
  if (it != vertex_blockages.end()) {
    vertex_blockages.erase(it);
    return true;
  }
  std::vector<RoutingTrackBlockage*> &edge_blockages =
      temporary_blockages_.edge_blockages;
  it = std::find(edge_blockages.begin(), edge_blockages.end(), blockage);
  if (it != edge_blockages.end()) {
    edge_blockages.erase(it);
    return true;
  }
  return true;
}

bool RoutingTrack::ApplyVertexBlockageToSingleVertex(
    const RoutingTrackBlockage &blockage,
    const std::string &net,
    bool is_temporary,
    RoutingVertex *vertex) {
  if (!vertex->Available()) {
    return false;
  }
  // We only disable vertices if they're _completely_ blocked, i.e. with
  // margin = 0.
  if (!BlockageBlocks(blockage,
                      vertex->centre(),
                      vertex->centre(),
                      0)) {
    return false;
  }
  // See note on RoutingGrid::ApplyBlockageToOneVertex: this looks like it's
  // usually duplicate work.
  if (net != "") {
    // TODO(aryap): Put these on temporary mutation plane so that they can
    // be undone.
    vertex->AddBlockingNet(net, is_temporary, layer_);
  } else {
    vertex->SetForcedBlocked(true, is_temporary, layer_);
  }
  return true;
}

void RoutingTrack::ApplyVertexBlockage(
    const RoutingTrackBlockage &blockage,
    const std::string &net,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices) {
  for (const auto &entry : vertices_by_offset_) {
    RoutingVertex *vertex = entry.second;
    bool applied = ApplyVertexBlockageToSingleVertex(
        blockage, net, is_temporary, vertex);
    if (applied && blocked_vertices) {
      blocked_vertices->insert(vertex);
    }
  }
}

bool RoutingTrack::ApplyEdgeBlockageToSingleEdge(
    const RoutingTrackBlockage &blockage,
    const std::string &net,
    bool is_temporary,
    RoutingEdge *edge) {
  if (edge->Blocked()) {
    return false;
  }
  if (!BlockageBlocks(blockage,
                      edge->first()->centre(),
                      edge->second()->centre(),
                      min_separation_to_new_blockages_)) {
    return false;
  }
  edge->SetBlocked(true, is_temporary);
  if (net != "" && !(
        edge->EffectiveNet() && edge->EffectiveNet() != "")) {
    edge->SetNet(net, is_temporary);
  }
  return true;
}

void RoutingTrack::ApplyEdgeBlockage(
    const RoutingTrackBlockage &blockage,
    const std::string &net,
    bool is_temporary,
    std::set<RoutingEdge*> *blocked_edges) {
  for (RoutingEdge *edge : edges_) {
    bool applied = ApplyEdgeBlockageToSingleEdge(
        blockage, net, is_temporary, edge);
    if (applied && blocked_edges) {
      blocked_edges->insert(edge);
    }
  }
}

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track) {
  os << track.Describe();
  return os;
}

}  // namespace bfg
