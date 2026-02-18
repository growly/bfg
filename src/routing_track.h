#ifndef ROUTING_TRACK_H_
#define ROUTING_TRACK_H_

#include <algorithm>
#include <set>
#include <shared_mutex>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "equivalent_nets.h"
#include "layout.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/rectangle.h"
#include "routing_blockage_cache.h"
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
  // TODO(aryap): Move to geometry/RoutingTrackDirectionUtility.
  static int64_t ProjectOntoAxis(
      const geometry::Point &point, const RoutingTrackDirection &direction);
  static std::pair<int64_t, int64_t> ProjectOntoAxis(
      const geometry::Point &lhs,
      const geometry::Point &rhs,
      const RoutingTrackDirection &direction);
  // This is only here because I can't put it in the RoutingTrackDirection enum.
  static RoutingTrackDirection OrthogonalDirectionTo(
      const RoutingTrackDirection &direction);
  static RoutingTrackDirection AngleToDirection(double angle_to_horizon_rads);

  RoutingTrack(const geometry::Layer &layer,
               const RoutingTrackDirection &direction,
               int64_t pitch,
               int64_t width,
               int64_t vertex_via_width,
               int64_t vertex_via_length,
               int64_t min_separation,
               int64_t offset,
               int64_t edges_only_to_neighbours = false)
      : layer_(layer),
        direction_(direction),
        offset_(offset),
        pitch_(pitch),
        width_(width),
        vertex_via_width_(vertex_via_width),
        vertex_via_length_(vertex_via_length),
        min_separation_(min_separation),
        edges_only_to_neighbours_(edges_only_to_neighbours) {
    min_separation_between_edges_ = vertex_via_length + min_separation;
    min_separation_to_new_blockages_ = vertex_via_length / 2 + min_separation;
    edges_min_transverse_separation_ = width_ / 2 + min_separation;
    vertices_min_transverse_separation_ = vertex_via_width / 2 + min_separation;
  }

  ~RoutingTrack();

  // Tries to add an edge between the two vertices, returning true if
  // successful and false if no edge could be added (it was blocked).
  bool MaybeAddEdgeBetween(
      RoutingVertex *one,
      RoutingVertex *the_other,
      const RoutingBlockageCache &blockage_cache,
      const std::optional<EquivalentNets> &for_nets = std::nullopt);

  std::set<RoutingEdge*>::iterator RemoveEdge(
      const std::set<RoutingEdge*>::iterator &pos);

  bool RemoveEdge(RoutingEdge *edge, bool and_delete);

  // Adds the given vertex to this track, but does not take ownership of it.
  // Generates an edge from the given vertex to every other vertex in the
  // track, as long as that edge would not be blocked already.
  bool AddVertex(
      RoutingVertex *vertex,
      const RoutingBlockageCache &blockage_cache,
      const std::optional<EquivalentNets> &for_nets = std::nullopt);

  // Checks every vertex to see if a new edge needs to be created to span over
  // it. This is necessary when vertices are blocked on other layers and
  // therefore inadvertently block edges in this track.
  void HealEdges(const RoutingBlockageCache &blockage_cache);

  // Returns true if an edge was added.
  bool HealAroundBlockedVertex(
      const RoutingVertex &vertex,
      const RoutingBlockageCache &blockage_cache);

  RoutingEdge *GetEdgeBetween(
      RoutingVertex *lhs, RoutingVertex *rhs) const;

  // Remove the vertex from this track, and remove any edge that uses it.
  // Thread-safe.
  //
  // The RoutingBlockageCache is used to check for any externally-managed
  // hazards when creating "healing" edges need to be created.
  bool RemoveVertex(
      RoutingVertex *vertex,
      const RoutingBlockageCache &blockage_cache);

  bool ContainsVertex(RoutingVertex *vertex) const;

  bool IsPointOnTrack(const geometry::Point &point) const;
  RoutingVertex *GetVertexAt(const geometry::Point &point) const;
  RoutingVertex *GetVertexAtOffset(int64_t offset) const;

  void MarkEdgeAsUsed(
      RoutingEdge *edge,
      const std::string &net,
      const RoutingBlockageCache &blockage_cache);

  bool IsPerpendicularTo(const RoutingTrackDirection &other) const;

  // Makes sure the target vertex can be connected to a candidate vertex placed
  // at the nearest point on the track to the given target at (O). If
  // successful, the new vertex is returned, otherwise nullptr. The return
  // vertex is property of the caller and any generated edge is property of the
  // track.
  //
  // There are four cases we care about:
  //    ^
  //    |
  // D (+)
  // C (O)
  //    |
  // A (|)---O  Target O need a bridging vertex on the track at the
  //    |       point where the normal meets O (A); this might already be
  //    |       a vertex (B) or O might already be on the track itself (C).
  // B (+)---O  Additionally, O might already be on the track at an existing
  //    |       vertex (D).
  //    v
  //
  // The caller must check:
  //  - returned bool for any failures to connect;
  //  - *connecting_vertex == target, in case the target was used as the
  //  connecting vertex (C);
  //  - *bridging_vertex_is_new, in case the connecting vertex was a new (A) or
  //  existing (B) vertex;
  //  - *target_already_exists, in case the target landed on an existing vertex
  //  and is invalid (D).
  //
  // This is a thread-safe function.
  bool CreateNearestVertexAndConnect(
      const RoutingGrid &grid,
      const RoutingBlockageCache &blockage_cache,
      RoutingVertex *target,
      const geometry::Layer &target_layer,
      const EquivalentNets &for_nets,
      RoutingVertex **connecting_vertex,
      bool *bridging_vertex_is_new,
      bool *target_already_exists);

  RoutingVertex *CreateNewVertexAndConnect(
      const RoutingGrid &grid,
      const RoutingBlockageCache &blockage_cache,
      const geometry::Point &candidate_centre,
      const geometry::Layer &target_layer,
      const EquivalentNets &for_nets);

  // Returned vertices will be in order of their position along the track, but
  // the direction is not guaranteed.
  std::vector<RoutingVertex*> VerticesInSpan(
      const geometry::Point &one_end,
      const geometry::Point &other_end) const;

  void ReportAvailableEdges(std::vector<RoutingEdge*> *edges_out) const;
  void ReportAvailableVertices(std::vector<RoutingVertex*> *vertices_out) const;
  void ExportEdgesAsRectangles(
      const std::string &layer,
      bool available_only,
      Layout *layout) const;

  // Rectangle blockages create single RoutingTrackBlockages or none at all, so
  // we can return one or nullptr here:
  // TODO(aryap): We can be smarter (more efficient) about determining when
  // blockages _don't_ occur, and this can save a lot of cycles. If a shape's
  // bounding-box doesn't fall near us on the offset axis we can immediately
  // nope the blockage.
  void AddBlockage(const geometry::Rectangle &rectangle,
                   int64_t padding,
                   const std::string &net,
                   RoutingTrackBlockage **new_vertex_blockage,
                   RoutingTrackBlockage **new_edge_blockage);
  // By contrast, polygons can create multiple blockages on a single track, and
  // need to return more creatively, if desired.
  void AddBlockage(const geometry::Polygon &polygon,
                   int64_t padding,
                   const std::string &net);

  // Returns the edges, vertices blocked by the given shape, with optional
  // padding, but does not create a permanent Blockage in the list of
  // blockages_.
  void AddTemporaryBlockage(
      const geometry::Rectangle &rectangle,
      int64_t padding,
      const std::string &net,
      RoutingTrackBlockage **new_vertex_blockage,
      RoutingTrackBlockage **new_edge_blockage,
      std::set<RoutingVertex*> *blocked_vertices,
      std::set<RoutingEdge*> *blocked_edges);

  bool RemoveTemporaryBlockage(RoutingTrackBlockage *blockage);

  // Returning an optional here in case it's faster than returning an empty
  // vector. I wonder?
  template<typename T>
  std::optional<std::vector<RoutingEdge*>> EdgesBlockedByShape(
      const T &shape, int64_t padding) const;

  // Get the neighbours on either side of the given vertex. If an existing
  // vertex at the given vertex's offset exists, it is NOT included. If
  // `available_only` is true, the nearest available neighbours are returned.
  std::vector<RoutingVertex*> GetImmediateNeighbours(
      const RoutingVertex &vertex,
      bool available_only = false) const;

  geometry::Line ParallelLineAtOffset(int64_t offset) const;

  geometry::Line AsLine() const;
  std::pair<geometry::Line, geometry::Line> MajorAxisLines(
      int64_t padding) const;

  std::string Describe() const;

  const std::set<RoutingEdge*> &edges() const { return edges_; }

  const geometry::Layer &layer() const { return layer_; }

  RoutingTrackDirection direction() const { return direction_; }

  int64_t offset() const { return offset_; }

  int64_t width() const { return width_; }
  void set_width(int64_t width) { width_ = width; }

 private:
  // TODO(aryap): Maybe we sort edges and vertices by their starting/centre
  // positions?
  //static bool EdgeComp(RoutingEdge *lhs, RoutingEdge *rhs);

  // FIXME(aryap): Why do we distinguish temoprary and permanent blockage
  // groups, when we could just put a "temporary" flag in the
  // RoutingTrackBlockage itself? That's much more convenient, and leads to
  // cleaner code in the last few places I've checked. I wonder if I had a good
  // reason for this. (Probably not.)
  struct BlockageGroup {
    std::vector<RoutingTrackBlockage*> vertex_blockages;
    std::vector<RoutingTrackBlockage*> edge_blockages;
  };

  std::vector<RoutingEdge*> EdgesBlockedByBlockage(
      const RoutingTrackBlockage &blockage, int64_t padding) const;

  // RoutingTrack does not check if any of the layers of these objects match its
  // layer when checking for intersection. It is more efficient for the caller
  // to avoid intersecting shapes with a routing track that is on a separate
  // layer to them, than it is to check every single RoutingTrack. As with
  // other geometric primitives, intersections are tested assuming all shapes
  // are on the same layer.
  bool Intersects(RoutingVertex *vertex) const;

  bool Intersects(const geometry::Rectangle &rectangle,
                  int64_t padding,
                  int64_t min_transverse_separation) const;
  // TODO(growly): This does not account for the fact that a rectangle might be
  // small enough to fit between the keep-outs around two adjacent vertices.
  bool IntersectsVertices(
      const geometry::Rectangle &rectangle,
      int64_t padding) const {
    return Intersects(rectangle, padding, vertices_min_transverse_separation_);
  }

  bool IntersectsEdges(
      const geometry::Rectangle &rectangle,
      int64_t padding) const {
    return Intersects(rectangle, padding, edges_min_transverse_separation_);
  }

  bool Intersects(
      const geometry::Polygon &polygon,
      std::vector<geometry::PointPair> *intersections,
      int64_t padding,
      int64_t min_transverse_separation) const;

  bool IntersectsVertices(
      const geometry::Polygon &polygon,
      std::vector<geometry::PointPair> *intersections,
      int64_t padding) const {
    return Intersects(
        polygon, intersections, padding, vertices_min_transverse_separation_);
  }

  bool IntersectsEdges(
      const geometry::Polygon &polygon,
      std::vector<geometry::PointPair> *intersections,
      int64_t padding) const {
    return Intersects(
        polygon, intersections, padding, edges_min_transverse_separation_);
  }

  void AssignThisTrackToVertex(RoutingVertex *vertex);

  RoutingVertex *MakeAndCheckVertexAt(
      const RoutingGrid &grid,
      const geometry::Point &point,
      const geometry::Layer &target_layer,
      const EquivalentNets &for_nets);
 
  bool BlockageBlocks(
      const RoutingTrackBlockage &blockage,
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin = 0) const;

  // Instead of checking blockages on this track, check that the neighbouring
  // vertices to the given point, if in use (i.e. not Available()), could
  // accomodate a via simultaneously with a via at the given point. This is
  // an optimisation so that the caller can avoid checking all the edges created
  // on other layers by putting a via on this track for collisions.
  bool IsProbablyBlockedForVia(
      const geometry::Point &point, int64_t margin = 0) const;

  bool IsBlocked(const geometry::Point &point,
                 int64_t margin,
                 const std::optional<EquivalentNets> &for_nets) const {
    return IsVertexBlocked(point, margin, for_nets) ||
           IsEdgeBlockedBetween(point, point, margin, for_nets, nullptr, nullptr);
  }

  bool IsVertexBlocked(const geometry::Point &point,
                       int64_t margin,
                       const std::optional<EquivalentNets> &for_nets) const;

  bool IsEdgeBlockedBetween(
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin,
      const std::optional<EquivalentNets> &for_nets,
      std::vector<RoutingTrackBlockage*> *same_net_collisions,
      std::vector<RoutingTrackBlockage*> *temporary_same_net_collisions) const;

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
      int64_t margin,
      const std::string &net,
      std::vector<RoutingTrackBlockage*> *container);

  RoutingTrackBlockage *MergeNewEdgeBlockage(
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin = 0,
      const std::string &net = "") {
    return MergeNewBlockage(
        one_end, other_end, margin, net, &blockages_.edge_blockages);
  }

  RoutingTrackBlockage *MergeNewVertexBlockage(
      const geometry::Point &one_end,
      const geometry::Point &other_end,
      int64_t margin = 0,
      const std::string &net = "") {
    return MergeNewBlockage(
        one_end, other_end, margin, net, &blockages_.vertex_blockages);
  }

  // Return true if the blockage applied to the edge.
  bool ApplyEdgeBlockageToSingleEdge(
      const RoutingTrackBlockage &blockage,
      const std::string &net,
      bool is_temporary,
      RoutingEdge *edge);

  void ApplyEdgeBlockage(
      const RoutingTrackBlockage &blockage,
      const std::string &net = "",
      bool is_temporary = false,
      std::set<RoutingEdge*> *blocked_edges = nullptr);

  bool ApplyVertexBlockageToSingleVertex(
      const RoutingTrackBlockage &blockage,
      const std::string &net,
      bool is_temporary,
      RoutingVertex *vertex);

  void ApplyVertexBlockage(
      const RoutingTrackBlockage &blockage,
      const std::string &net = "",
      bool is_temporary = false,
      std::set<RoutingVertex*> *blocked_vertices = nullptr);

  void SortBlockages(std::vector<RoutingTrackBlockage*> *container);

  // The edges generated for vertices on this track. These are OWNED by
  // RoutingTrack.
  std::set<RoutingEdge*> edges_;

  // The vertices on this track. Vertices are NOT OWNED by RoutingTrack.
  std::set<RoutingVertex*> vertices_;

  std::map<int64_t, RoutingVertex*> vertices_by_offset_;

  geometry::Layer layer_;
  RoutingTrackDirection direction_;

  // The x or y coordinate for this track, when the track runs in the y or x
  // direction.
  int64_t offset_;

  // How far apart vertices are on this track when on the grid.
  int64_t pitch_;

  // The working width of this track.
  int64_t width_;

  // Width of the shape implied by a vertex used on this track. 'Width' means
  // the size of the shape projected onto the axis _orthogonal_ to the track.
  int64_t vertex_via_width_;

  // Length (span) of the shape implied by a vertex used on this track.
  int64_t vertex_via_length_;

  // Minimum separation of shapes on this track's layer.
  int64_t min_separation_;

  // The minimum separation between the ends of two edges on a track is the
  // closest possible spacing of used vertices on the track, i.e. the amount of
  // space required to fit a via at the end of each used edge.  The minimum
  // spacing between two vertices on this track must therefore accommodate 2
  // halves of 'vertex_via_length_' plus this min_separation_.
  int64_t min_separation_between_edges_;

  // New blockages (added through AddBlockage) are static shapes that we do not
  // expect to change. The minimum separation from the end of an existing edge
  // to those is just half 'vertex_via_length_' plus min_separation_.
  int64_t min_separation_to_new_blockages_;

  bool edges_only_to_neighbours_;

  // The minimum distance to shapes measured in the axis perpendicular to the
  // track.
  int64_t edges_min_transverse_separation_;
  int64_t vertices_min_transverse_separation_;

  // We want to keep a sorted list of blockages, but if we keep them as a
  // std::set we can't mutate the objects (since they will not automatically be
  // re-sorted). Instead we keep a vector and make sure to sort it ourselves.
  // We OWN these objects.
  //
  // There are four distinct "planes" of blockages:
  //  - permanent vertex blockages, which prevent vertices being available or
  //  created the track (these also disable edges starting from or ending at any
  //  of the points within the blockage);
  //  - permanent edge blockages, which prevent valid edges on the track; and
  //  - the temporary versions of both of these.
  BlockageGroup blockages_;

  // We need a separate plane of blockages for temporary obstructions, perhaps
  // those that only apply in one path search but not the next (e.g. pins).
  // These blockages might usefully be merged together but not together with the
  // permanent ones. Since there usually aren't many of these it doesn't matter
  // so much.
  //
  // These can be cleared by pointer or all at once. The caller takes ownership
  // of the object, we DO NOT OWN temporary blockages.
  BlockageGroup temporary_blockages_;

  std::shared_mutex lock_;

  FRIEND_TEST(RoutingTrackTest, MergesBlockages);
  FRIEND_TEST(RoutingTrackTest, DoesNotMergeDifferingNets);
};

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track);

}  // namespace bfg

#endif  // ROUTING_TRACK_H_
