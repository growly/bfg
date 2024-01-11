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

void RoutingTrack::MarkEdgeAsUsed(RoutingEdge *edge, const std::string &net) {
  edge->set_in_use_by_net(net);
  //LOG(INFO) << "assigning edge " << *edge;

  if (edges_.find(edge) == edges_.end())
    // Possible off-grid edge?
    return;

  // TODO(aryap): This could be a problem because if the current edge merges
  // with an existing blockage, we will treat that blockage as touching this
  // net!
  RoutingTrackBlockage *current_blockage = MergeNewBlockage(
      edge->first()->centre(), edge->second()->centre());

  // Since we add a new blockage of strictly edge's size without any keep-out
  // padding, we are testing for edges that touch this one. Those edges must be
  // marked as 'in use' by the same net as this one, since they can still be
  // used to connect to the given net.
  for (RoutingEdge *other_edge : edges_) {
    if (other_edge == edge)
      continue;
    // FIXME: THIS IS NOT THE SAME AS "IS BLOCKED BY edge THAT WE JUST GOT"
    if (BlockageBlocks(
          *current_blockage,
          other_edge->first()->centre(),
          other_edge->second()->centre())) {
      other_edge->set_in_use_by_net(net);
    }
  }

  // Remove other vertices that are blocked by this.
  for (RoutingVertex *vertex : vertices_) {
    // NOTE: This will set the in- and out-edge of the vertex even if the vertex
    // is the start or end vertex of the edge; something else (the caller) must
    // correct this if the edge participates in a RoutingPath or if the in- and
    // out-edges must otherwise be adjusted.
    if (EdgeSpansVertex(*edge, *vertex)) {
      vertex->set_available(false);
      vertex->set_in_edge(edge);
      vertex->set_out_edge(edge);
      vertex->set_net(net);
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
      [](RoutingEdge* edge) { return edge->Available(); });
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

std::pair<geometry::Line, geometry::Line> RoutingTrack::MajorAxisLines(
    int64_t padding) const {
  geometry::Line low;
  geometry::Line high;
  switch (direction_) {
    case RoutingTrackDirection::kTrackHorizontal:
      low = geometry::Line({0, offset_ - padding}, {1, offset_ - padding});
      high = geometry::Line({0, offset_ + padding}, {1, offset_ + padding});
      break;
    case RoutingTrackDirection::kTrackVertical:
      low = geometry::Line({offset_ - padding, 0}, {offset_ - padding, 1});
      high = geometry::Line({offset_ + padding, 0}, {offset_ + padding, 1});
      break;
    default:
      LOG(FATAL) << "RoutingTrack has unknown direction " << direction_;
      break;
  }
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
  int64_t low = ProjectOntoTrack(lhs);
  int64_t high = ProjectOntoTrack(rhs);
  if (low > high)
    std::swap(low, high);
  return {low, high};
}

std::pair<int64_t, int64_t> RoutingTrack::ProjectOntoOffset(
    const geometry::Point &lhs, const geometry::Point &rhs) const {
  int64_t low = ProjectOntoOffset(lhs);
  int64_t high = ProjectOntoOffset(rhs);
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

bool RoutingTrack::BlockageBlocks(
    const RoutingTrackBlockage &blockage,
    const geometry::Point &one_end,
    const geometry::Point &other_end) const {
  auto low_high = ProjectOntoTrack(one_end, other_end);
  return blockage.Blocks(low_high.first, low_high.second);
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

  for (RoutingTrackBlockage *blockage : temporary_blockages_) {
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

// Get the x- or y-coordinate of the given point if this is a vertical or
// horizontal, respectively.
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

// Given a rectangle and this track (the line):
//
//        +-------------+             y = offset_axis_high
//        |             |
//        +-------------+             y = offset_axis_low
//
//    -------------------------       y = high
//    -------------------------       y = low
//
// The y-axis is the offset axis because the track runs horizontally.
//
// TODO(growly): These lines aren't actually infinite. We need to make sure
// shapes outside of the routing grid are not accidentally counted as
// intersections.
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

  // FIXME(aryap): I think adding the halo again here is double counting it:
  int64_t low = offset_ - (width_ - width_ / 2) - within_halo;
  int64_t high = offset_ + width_ / 2 + within_halo;

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
//
//    -------------------------       y = high
//    -------------------------       y = low
//
// This is not a generic way to determine if a polygon and a rectangle
// intersect. That would be more sophisticated. This is a rudimentary way to
// tell if, for our purposes, there is an intersection issue between the too.
// Practically that means we only check the major axis of the track for
// intersection with the polygon, and we assume the track is never full
// contained by or fully contains the polygon.
bool RoutingTrack::Intersects(
    const geometry::Polygon &polygon,
    std::vector<geometry::PointPair> *intersections,
    int64_t within_halo) const {
  int64_t boundary_from_offset = width_ + within_halo;
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

  std::vector<geometry::PointPair> intersections_low;
  polygon.IntersectingPoints(major_axis_lines.first, &intersections_low);
 
  std::vector<geometry::PointPair> intersections_high;
  polygon.IntersectingPoints(major_axis_lines.second, &intersections_high);

  auto comp = [](const geometry::PointPair &lhs,
                 const geometry::PointPair &rhs) {
    if (lhs.first == rhs.first) {
      return lhs.first < rhs.first;
    }
    return lhs.second < rhs.second;
  };
  auto deduped = std::set<
    std::pair<geometry::Point, geometry::Point>, decltype(comp)>(comp);
  deduped.insert(intersections_low.begin(), intersections_low.end());
  deduped.insert(intersections_high.begin(), intersections_high.end());
  intersections->insert(intersections->end(), deduped.begin(), deduped.end());
  return !deduped.empty();
}

RoutingTrackBlockage *RoutingTrack::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding) {
  if (Intersects(rectangle, padding)) {
    RoutingTrackBlockage *blockage = MergeNewBlockage(
        rectangle.lower_left(), rectangle.upper_right());
    if (blockage) {
      ApplyBlockage(*blockage);
      return blockage;
    }
  }
  return nullptr;
}

void RoutingTrack::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding) {
  //LOG(INFO) << "Adding polygon blockage to routing track " << offset_ << " padding="
  //          << padding << ": " << polygon.Describe();
  geometry::Line track = AsLine();
  std::vector<geometry::PointPair> intersections;
  Intersects(polygon, &intersections, padding);

  for (const auto &pair : intersections) {
    RoutingTrackBlockage *blockage = MergeNewBlockage(pair.first, pair.second);
    if (blockage) {
      ApplyBlockage(*blockage);
    }
  }
}

RoutingTrackBlockage *RoutingTrack::AddTemporaryBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  if (Intersects(rectangle, padding)) {
    std::pair<int64_t, int64_t> low_high = ProjectOntoTrack(
        rectangle.lower_left(), rectangle.upper_right());

    RoutingTrackBlockage *temporary_blockage = new RoutingTrackBlockage(
        low_high.first, low_high.second);
    temporary_blockages_.push_back(temporary_blockage);
    ApplyBlockage(*temporary_blockage, blocked_vertices, blocked_edges);
    return temporary_blockage;
  }
  return nullptr;
}

RoutingTrackBlockage *RoutingTrack::MergeNewBlockage(
    const geometry::Point &one_end, const geometry::Point &other_end) {
  std::pair<int64_t, int64_t> low_high = ProjectOntoTrack(one_end, other_end);
  int64_t low = low_high.first;
  int64_t high = low_high.second;

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
  // blockage spanned by the new one. We rely on the sorted order of the
  // blockages.
  RoutingTrackBlockage *blockage = new RoutingTrackBlockage(
      std::min(low, (*first)->start()),
      std::max(high, (*last)->end()));

  // Delete the old elements.
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

bool RoutingTrack::RemoveTemporaryBlockage(RoutingTrackBlockage *blockage) {
  auto it = std::find(
      temporary_blockages_.begin(), temporary_blockages_.end(), blockage);
  if (it == temporary_blockages_.end()) {
    return false;
  }
  temporary_blockages_.erase(it);
  return true;
}

void RoutingTrack::ClearTemporaryBlockages() {
  // We don't own these, so we just clear them.
  temporary_blockages_.clear();
}

void RoutingTrack::ApplyBlockage(
    const RoutingTrackBlockage &blockage,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  for (RoutingVertex *vertex : vertices_) {
    if (!vertex->available())
      continue;
    if (BlockageBlocks(blockage, vertex->centre(), vertex->centre())) {
      vertex->set_available(false);
      if (blocked_vertices)
        blocked_vertices->insert(vertex);
    }
  }
  for (RoutingEdge *edge : edges_) {
    if (edge->blocked())
      continue;
    if (BlockageBlocks(
          blockage, edge->first()->centre(), edge->second()->centre())) {
      edge->set_blocked(true);
      if (blocked_edges)
        blocked_edges->insert(edge);
    }
  }
}

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track) {
  os << track.Debug();
  return os;
}

}  // namespace bfg
