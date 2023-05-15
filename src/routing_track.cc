#include "routing_track.h"

#include <set>
#include <vector>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_vertex.h"
#include "routing_track_blockage.h"
#include "physical_properties_database.h"

namespace bfg {

RoutingTrack::~RoutingTrack() {
  for (RoutingEdge *edge : edges_) { delete edge; }
  for (RoutingTrackBlockage *blockage : blockages_) { delete blockage; }
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

bool RoutingTrack::MaybeAddEdgeBetween(
    RoutingVertex *one, RoutingVertex *the_other) {
  if (IsBlockedBetween(one->centre(), the_other->centre()))
    return false;
  RoutingEdge *edge = new RoutingEdge(one, the_other);
  edge->set_track(this);
  edge->set_layer(layer_);
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

void RoutingTrack::MarkEdgeAsUsed(RoutingEdge *edge, const std::string *net) {
  edge->set_available(true);

  if (edges_.find(edge) == edges_.end())
    // Possible off-grid edge?
    return;

  CreateBlockage(edge->first()->centre(), edge->second()->centre());

  // Mark other edges that are blocked by this as used.
  for (RoutingEdge *edge : edges_) {
    if (IsBlockedBetween(edge->first()->centre(), edge->second()->centre())) {
      // Ownership of other blocked edges is not transferred; they are just
      // removed.
      edge->set_available(false);
    }
  }

  // Remove other vertices that are blocked by this.
  for (RoutingVertex *vertex : vertices_) {
    if (EdgeSpansVertex(*edge, *vertex)) {
      vertex->set_available(false);
      vertex->set_in_edge(edge);
      vertex->set_out_edge(edge);
      if (net) vertex->set_net(*net);
    }
  }
}

RoutingVertex *RoutingTrack::CreateNearestVertexAndConnect(
    const geometry::Point &point,
    RoutingVertex *target) {
  // Candidate position:
  geometry::Point candidate_centre;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      candidate_centre = geometry::Point(point.x(), offset_);
      break;
    case RoutingTrackDirection::kTrackVertical:
      candidate_centre = geometry::Point(offset_, point.y());
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
    delete bridging_vertex;
    LOG(FATAL) << "I thought we made sure this couldn't happen already.";
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

std::pair<int64_t, int64_t> RoutingTrack::ProjectOntoTrack(
    const geometry::Point &lhs, const geometry::Point &rhs) const {
  int64_t low = ProjectOntoTrack(lhs);
  int64_t high = ProjectOntoTrack(rhs);
  if (low > high)
    std::swap(low, high);
  return {low, high};
}

bool RoutingTrack::EdgeSpansVertex(
    const RoutingEdge &edge, const RoutingVertex &vertex) const {
  int64_t pos = ProjectOntoTrack(vertex.centre());
  auto points = ProjectOntoTrack(edge.first()->centre(), edge.second()->centre());
  int64_t low = points.first;
  int64_t high = points.second;

  return low <= pos && pos <= high;
}

bool RoutingTrack::IsBlockedBetween(
    const geometry::Point &one_end, const geometry::Point &other_end) const {
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

int64_t RoutingTrack::ProjectOntoTrack(const geometry::Point &point) const {
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

// Get the x- or y-coordinate of this track if it is vertical or horizontal,
// respectively.
int64_t RoutingTrack::ProjectOntoOffset(const geometry::Point &point) const {
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      return point.y();
    case RoutingTrackDirection::kTrackVertical:
      return point.x();
    default:
      LOG(FATAL) << "This RoutingTrack has an unrecognised "
                 << "RoutingTrackDirection: " << direction_;
  }
  return 0;
}

bool RoutingTrack::Intersects(
    const geometry::Rectangle &rectangle,
    int64_t within_halo) const {
  // First check that the minor direction falls on this offset:
  int64_t offset_axis_low = ProjectOntoOffset(rectangle.lower_left());
  int64_t offset_axis_high = ProjectOntoOffset(rectangle.upper_right());

  if (offset_axis_low > offset_axis_high)
    std::swap(offset_axis_low, offset_axis_high);

  offset_axis_low -= within_halo;
  offset_axis_high += within_halo;

  int64_t low = offset_ - (width_ - width_ / 2) - within_halo;
  int64_t high = offset_ + width_ / 2 + within_halo;

  // There is no intersection if both the track edges are on the low or the
  // high side of the blockage. Otherwise if one of the edges is straddled or
  // we're entirely within the shape, there is:
  return !((low < offset_axis_low && high < offset_axis_low) ||
           (low > offset_axis_high && high > offset_axis_high));
}

RoutingTrackBlockage *RoutingTrack::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding) {
  if (Intersects(rectangle, padding)) {
    RoutingTrackBlockage *blockage = CreateBlockage(
        rectangle.lower_left(), rectangle.upper_right());
    if (blockage) {
      ApplyBlockage(*blockage);
    }
  }
  return nullptr;
}

void RoutingTrack::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding) {
  geometry::Line track = AsLine();
  std::vector<std::pair<geometry::Point, geometry::Point>> intersections;
  polygon.IntersectingPoints(track, &intersections);
  for (const auto &pair : intersections) {
    RoutingTrackBlockage *blockage = CreateBlockage(
        pair.first, pair.second);
    if (blockage) {
      ApplyBlockage(*blockage);
    }
  }
}

RoutingTrackBlockage *RoutingTrack::CreateBlockage(
    const geometry::Point &one_end, const geometry::Point &other_end) {
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

void RoutingTrack::ApplyBlockage(const RoutingTrackBlockage &blockage) {
  for (RoutingVertex *vertex : vertices_) {
    if (IsBlockedBetween(vertex->centre(), vertex->centre())) {
      vertex->set_available(false);
    }
  }
  for (RoutingEdge *edge : edges_) {
    if (IsBlockedBetween(edge->first()->centre(), edge->second()->centre())) {
      edge->set_available(false);
    }
  }
}

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track) {
  os << track.Debug();
  return os;
}

}  // namespace bfg
