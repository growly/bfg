#ifndef ROUTING_TRACK_H_
#define ROUTING_TRACK_H_

#include <set>
#include <utility>
#include <vector>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_vertex.h"
#include "physical_properties_database.h"

namespace bfg {

// FIXME(growly): Instead of assigning spanned/spanned by sets (expensive given
// model we have), assign net labels when a path is installed to all all
// spanned vertices in the track.

class RoutingTrackBlockage;
class RoutingGrid;

// RoutingTracks keep track of the edges, which are physical spans, that could
// fall on them. When such an edge is used for a route, the RoutingTrack
// determines which other edges must be invalidated.
//
// RoutingTracks do not own anything, but keep track of which vertices and
// edges associated with them. They invalidate those objects when they are used
// up.
class RoutingTrack {
 public:
  RoutingTrack(const geometry::Layer &layer,
               const RoutingTrackDirection &direction,
               int64_t min_separation,
               int64_t offset)
      : layer_(layer),
        direction_(direction),
        offset_(offset),
        min_separation_(min_separation) {}

  ~RoutingTrack();

  // Tries to add an edge between the two vertices, returning true if
  // successful and false if no edge could be added (it was blocked).
  bool MaybeAddEdgeBetween(RoutingVertex *one, RoutingVertex *the_other);

  std::set<RoutingEdge*>::iterator RemoveEdge(
      const std::set<RoutingEdge*>::iterator &pos);

  bool RemoveEdge(RoutingEdge *edge, bool and_delete);

  // Adds the given vertex to this track, but does not take ownership of it.
  // Generates an edge from the given vertex to every other vertex in the
  // track, as long as that edge would not be blocked already.
  bool AddVertex(RoutingVertex *vertex);

  // Remove the vertex from this track, and remove any edge that uses it.
  bool RemoveVertex(RoutingVertex *vertex);

  void MarkEdgeAsUsed(RoutingEdge *edge, const std::string &net);

  // Tries to connect the target vertex to a candidate vertex placed at the
  // nearest point on the track to the given point (O). If successful, the new
  // vertex is returned, otherwise nullptr. The return vertex is property of
  // the caller and any generated edge is property of the track.
  //
  //
  //          O
  //          |
  //          v
  // ---X--------X---
  //          ^  ^ target 
  //          |
  //          created vertex position
  RoutingVertex *CreateNearestVertexAndConnect(
      const RoutingGrid &grid,
      const geometry::Point &point,
      const geometry::Layer &target_layer,
      RoutingVertex *target);

  std::vector<RoutingVertex*> VerticesInSpan(
      const geometry::Point &one_end,
      const geometry::Point &other_end) const;

  void ReportAvailableEdges(std::vector<RoutingEdge*> *edges_out);
  void ReportAvailableVertices(std::vector<RoutingVertex*> *vertices_out);

  bool Intersects(const geometry::Rectangle &rectangle,
                  int64_t within_halo = 0) const;
  bool Intersects(
      const geometry::Polygon &polygon,
      std::vector<geometry::PointPair> *intersections,
      int64_t within_halo = 0) const;

  // Rectangle blockages create single RoutingTrackBlockages or none at all, so
  // we can return one or nullptr here:
  RoutingTrackBlockage *AddBlockage(const geometry::Rectangle &rectangle,
                                    int64_t padding = 0);
  // By contrast, polygons can create multiple blockages on a single track, and
  // need to return more creatively, if desired.
  void AddBlockage(const geometry::Polygon &polygon,
                   int64_t padding = 0);

  // Returns the edges, vertices blocked by the given shape, with optional
  // padding, but does not create a permanent Blockage in the list of
  // blockages_.
  RoutingTrackBlockage *AddTemporaryBlockage(
      const geometry::Rectangle &rectangle,
      int64_t padding = 0,
      std::set<RoutingVertex*> *blocked_vertices = nullptr,
      std::set<RoutingEdge*> *blocked_edges = nullptr);

  bool RemoveTemporaryBlockage(RoutingTrackBlockage *blockage);

  void ClearTemporaryBlockages();

  geometry::Line AsLine() const;
  std::pair<geometry::Line, geometry::Line> MajorAxisLines(
      int64_t padding) const;

  std::string Debug() const;

  const std::set<RoutingEdge*> &edges() const { return edges_; }

  const geometry::Layer &layer() const { return layer_; }

  int64_t offset() const { return offset_; }

  int64_t width() const { return width_; }
  void set_width(int64_t width) { width_ = width; }

 private:
  // TODO(aryap): Maybe we sort edges and vertices by their starting/centre
  // positions?
  //static bool EdgeComp(RoutingEdge *lhs, RoutingEdge *rhs);
 
  bool BlockageBlocks(
      const RoutingTrackBlockage &blockage,
      const geometry::Point &one_end,
      const geometry::Point &other_end) const;

  bool IsBlocked(const geometry::Point &point, int64_t margin = 0) const {
    return IsBlockedBetween(point, point, margin);
  }
  bool IsBlockedBetween(
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin = 0) const;

  bool EdgeSpansVertex(
      const RoutingEdge &edge, const RoutingVertex &vertex) const;

  std::pair<int64_t, int64_t> ProjectOntoTrack(
      const geometry::Point &lhs, const geometry::Point &rhs) const;
  int64_t ProjectOntoTrack(const geometry::Point &point) const;
  std::pair<int64_t, int64_t> ProjectOntoOffset(
      const geometry::Point &lhs, const geometry::Point &rhs) const;
  int64_t ProjectOntoOffset(const geometry::Point &point) const;

  geometry::Point PointOnTrack(int64_t projection_onto_track) const;

  RoutingTrackBlockage *MergeNewBlockage(
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin = 0);
  void ApplyBlockage(const RoutingTrackBlockage &blockage,
                     std::set<RoutingVertex*> *blocked_vertices = nullptr,
                     std::set<RoutingEdge*> *blocked_edges = nullptr);

  void SortBlockages();

  // The edges generated for vertices on this track. These are OWNED by
  // RoutingTrack.
  std::set<RoutingEdge*> edges_;

  // The vertices on this track. Vertices are NOT OWNED by RoutingTrack.
  std::set<RoutingVertex*> vertices_;

  geometry::Layer layer_;
  RoutingTrackDirection direction_;

  // The x or y coordinate for this track, when the track runs in the y or x
  // direction.
  int64_t offset_;

  // The working width of this track.
  int64_t width_;

  // Minimum separation of wires on this track.
  int64_t min_separation_;

  // We want to keep a sorted list of blockages, but if we keep them as a
  // std::set we can't mutate the objects (since they will not automatically be
  // re-sorted). Instead we keep a vector and make sure to sort it ourselves.
  std::vector<RoutingTrackBlockage*> blockages_;

  // We need a separate plane of blockages for temporary obstructions, perhaps
  // those that only apply in one path search but not the next (e.g. pins).
  // These blockages might usefully be merged together but not together with the
  // permanent ones. Since there usually aren't many of these it doesn't matter
  // so much.
  //
  // These can be cleared by pointer or all at once. The caller takes ownership
  // of the object.
  std::vector<RoutingTrackBlockage*> temporary_blockages_;
};

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track);

}  // namespace bfg

#endif  // ROUTING_TRACK_H_
