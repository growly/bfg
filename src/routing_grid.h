#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include "geometry/layer.h"
#include "physical_properties_database.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "poly_line_cell.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"

#include <map>
#include <set>
#include <deque>
#include <vector>

namespace bfg {

using geometry::Layer;
using geometry::PolyLine;

class RoutingEdge;
class RoutingTrack;
class RoutingGrid;

struct RoutingLayerInfo {
  geometry::Layer layer;
  geometry::Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingTrackDirection direction;
  int64_t pitch;
};

class RoutingVertex {
 public:
  RoutingVertex(const geometry::Point &centre)
      : available_(true), horizontal_track_(nullptr), vertical_track_(nullptr),
        centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.insert(edge); }
  bool RemoveEdge(RoutingEdge *edge);

  //const std::set<RoutingEdge*> &edges() { return edges_; }

  uint64_t L1DistanceTo(const geometry::Point &point);

  // This is the cost of connecting through this vertex (i.e. a via).
  double cost() const { return 1.0; }

  void AddConnectedLayer(const geometry::Layer &layer) {
    connected_layers_.push_back(layer);
  }
  const std::vector<geometry::Layer> &connected_layers() { return connected_layers_; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::set<RoutingEdge*> edges() const { return edges_; }

  const geometry::Point &centre() const { return centre_; }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_horizontal_track(RoutingTrack *track) { horizontal_track_ = track; }
  RoutingTrack *horizontal_track() const { return horizontal_track_; }
  void set_vertical_track(RoutingTrack *track) { vertical_track_ = track; }
  RoutingTrack *vertical_track() const { return vertical_track_; }

 private:
  bool available_;
  RoutingTrack *horizontal_track_;
  RoutingTrack *vertical_track_;

  // This is a minor optimisation to avoid having to key things by pointer.
  // This index should be unique within the RoutingGrid that owns this
  // RoutingVertex for the duration of whatever process requires it.
  size_t contextual_index_;

  geometry::Point centre_;
  std::vector<geometry::Layer> connected_layers_;
  std::set<RoutingEdge*> edges_;
};

// Edges are NOT directed.
class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : available_(true),
      track_(nullptr),
      layer_(0),
      first_(first),
      second_(second),
      cost_(1.0) {}

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() { return first_; }
  RoutingVertex *second() { return second_; }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_layer(const geometry::Layer &layer) { layer_ = layer; }

  const geometry::Layer &ExplicitOrTrackLayer() const;

  // Off-grid edges do not have tracks.
  void set_track(RoutingTrack *track);
  RoutingTrack *track() const { return track_; }

  std::vector<RoutingVertex*> VertexList() const;

 private:
  bool available_;

  RoutingTrack *track_;
  geometry::Layer layer_;

  RoutingVertex *first_;
  RoutingVertex *second_;

  // Need some function of the distance between the two vertices (like of
  // length, sheet resistance). This also needs to be computed only once...
  double cost_;
};

class RoutingPath {
 public:
  // We seize ownership of the edges and vertices given to this path.
  //
  // TODO(aryap): Maybe we should just leave ownership up to the RoutingGrid
  // and just make sure all the referenced edges and vertices in paths are
  // ultimately deleted anyway.
  //
  // TODO(aryap): Yeah, this makes me nervous. In order to avoid simply keeping
  // all routing resources, something you'd call a premature optimisation, I
  // now have to make sure that ownership of used edges is transferred from
  // wherever into the paths. This better not bite me in the ass like it
  // absolutely is going to.
  RoutingPath(RoutingVertex *start, const std::deque<RoutingEdge*> edges);
  ~RoutingPath() {
    for (RoutingVertex *vertex : vertices_) { delete vertex; }
    for (RoutingEdge *edge : edges_) { delete edge; }
  }

  RoutingVertex *Begin() const {
    return Empty() ? nullptr : vertices_.front();
  }
  RoutingVertex *End() const {
    return Empty() ? nullptr : vertices_.back();
  }

  void ToPolyLinesAndVias(
      const RoutingGrid &routing_grid,
      std::vector<std::unique_ptr<PolyLine>> *poly_lines,
      std::vector<std::unique_ptr<AbstractVia>> *vias) const;

  bool Empty() const { return edges_.empty(); }

  const geometry::Port *start_port() const { return start_port_; }
  void set_start_port(const geometry::Port *port) { start_port_ = port; }
  const geometry::Port *end_port() const { return end_port_; }
  void set_end_port(const geometry::Port *port) { end_port_ = port; }

  const std::vector<RoutingVertex*> vertices() const { return vertices_; }
  const std::vector<RoutingEdge*> edges() const { return edges_; }

 private:
  // If these ports are provided, a via will be generated or the edge on the
  // given layer extended to correctly connect to them.
  const geometry::Port *start_port_;
  const geometry::Port *end_port_;

  // The ordered list of vertices making up the path. The edges alone, since
  // they are undirected, do not yield this directional information.
  // These vertices are OWNED by RoutingPath.
  std::vector<RoutingVertex*> vertices_;

  // The list of edges. Edge i connected vertices_[j] and vertices_[j+1].
  // These edges are OWNED by RoutingPath.
  std::vector<RoutingEdge*> edges_;
};

std::ostream &operator<<(std::ostream &os, const RoutingPath &path);

class RoutingTrackBlockage {
 public:
  RoutingTrackBlockage(int64_t start, int64_t end)
      : start_(start), end_(end) {
    LOG_IF(FATAL, end_ <= start_)
        << "RoutingTrackBlockage start must be before end.";
  }

  bool Contains(int64_t position);
  bool IsAfter(int64_t position);
  bool IsBefore(int64_t position);

  bool Blocks(int64_t low, int64_t high);

  void set_start(int64_t start) { start_ = start; }
  void set_end(int64_t end) { end_ = end; }

  int64_t start() const { return start_; }
  int64_t end() const { return end_; }

 private:
  int64_t start_;
  int64_t end_;
};

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
               int64_t offset)
      : layer_(layer), direction_(direction), offset_(offset) {}

  ~RoutingTrack() {
    for (RoutingEdge *edge : edges_) { delete edge; }
    for (RoutingTrackBlockage *blockage : blockages_) { delete blockage; }
  }

  // Tries to add an edge between the two vertices, returning true if
  // successful and false if no edge could be added (it was blocked).
  bool MaybeAddEdgeBetween(RoutingVertex *one, RoutingVertex *the_other);

  bool RemoveEdge(RoutingEdge *edge, bool and_delete);

  // Adds the given vertex to this track, but does not take ownership of it.
  // Generates an edge from the given vertex to every other vertex in the
  // track, as long as that edge would not be blocked already.
  bool AddVertex(RoutingVertex *vertex);

  // Remove the vertex from this track, and remove any edge that uses it.
  bool RemoveVertex(RoutingVertex *vertex);

  // 
  void MarkEdgeAsUsed(RoutingEdge *edge,
                      std::set<RoutingVertex*> *removed_vertices);

  // Triest to connect the target vertex to a canidate vertex placed at the
  // nearest point on the track to the given point. If successful, the new
  // vertex is returned, otherwise nullptr. The return vertex is property of
  // the caller and any generated edge is property of the track.
  RoutingVertex *CreateNearestVertexAndConnect(
      const geometry::Point &point,
      RoutingVertex *target);

  void ReportAvailableEdges(std::vector<RoutingEdge*> *edges_out);
  void ReportAvailableVertices(std::vector<RoutingVertex*> *vertices_out);

  std::string Debug() const;

  const std::set<RoutingEdge*> &edges() const { return edges_; }

  const geometry::Layer &layer() const { return layer_; }

 private:
  bool IsBlocked(const geometry::Point &point) const {
    return IsBlockedBetween(point, point);
  }
  bool IsBlockedBetween(const geometry::Point &one_end, const geometry::Point &other_end) const;

  int64_t ProjectOntoTrack(const geometry::Point &point) const;

  RoutingTrackBlockage *CreateBlockage(
      const geometry::Point &one_end, const geometry::Point &other_end);

  void SortBlockages();

  // The edges generated for vertices on this track. These are OWNED by
  // RoutingTrack.
  std::set<RoutingEdge*> edges_;

  // The vertices on this track. Vertices are NOT OWNED by RoutingTrack.
  std::set<RoutingVertex*> vertices_;

  geometry::Layer layer_;
  RoutingTrackDirection direction_;

  // The x or y coordinate for this track.
  int64_t offset_;

  // We want to keep a sorted list of blockages, but if we keep them as a
  // std::set we can't mutate the objects (since then no resorting is
  // performed). Instead we keep a vector and make sure to sort it ourselves.
  std::vector<RoutingTrackBlockage*> blockages_;
};

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track);

class RoutingGrid {
 public:
  RoutingGrid(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  ~RoutingGrid() {
    for (auto entry : tracks_by_layer_) {
      for (RoutingTrack *track : entry.second) {
        delete track;
      }
    }
    for (RoutingPath *path : paths_) { delete path; }
    for (RoutingEdge *edge : off_grid_edges_) { delete edge; }
    for (RoutingVertex *vertex : vertices_) { delete vertex; }
  }

  // Connecting two layers generates the graph that describes all the paths one
  // can take between them; concretely, it creates a vertex every time a
  // horizontal and vertical routing line cross. (The two described layers must
  // be orthogonal in routing direction.)
  void ConnectLayers(
      const geometry::Layer &first, const geometry::Layer &second);

  bool AddRouteBetween(
      const geometry::Port &begin, const geometry::Port &end);

  void AddVertex(RoutingVertex *vertex);

  void DeleteEdge(RoutingEdge *edge);
  bool RemoveVertex(RoutingVertex *vertex, bool and_delete);

  // Caller takes ownership.
  PolyLineCell *CreatePolyLineCell() const;

  void AddRoutingViaInfo(const geometry::Layer &lhs,
                  const geometry::Layer &rhs,
                  const RoutingViaInfo &info);

  const RoutingViaInfo &GetRoutingViaInfo(const AbstractVia &via) const {
    return GetRoutingViaInfo(via.bottom_layer(), via.top_layer());
  }
  const RoutingViaInfo &GetRoutingViaInfo(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  void AddRoutingLayerInfo(const RoutingLayerInfo &info);
  const RoutingLayerInfo &GetRoutingLayerInfo(const geometry::Layer &layer) const;

  const std::vector<RoutingPath*> &paths() const { return paths_; }
  const std::set<RoutingEdge*> &off_grid_edges() const {
    return off_grid_edges_;
  }
  const std::vector<RoutingVertex*> &vertices() const { return vertices_; }

  const PhysicalPropertiesDatabase &physical_db() const { return physical_db_; }

 private:
  std::pair<std::reference_wrapper<const RoutingLayerInfo>,
            std::reference_wrapper<const RoutingLayerInfo>>
      PickHorizontalAndVertical(
          const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  std::vector<RoutingVertex*> &GetAvailableVertices(const geometry::Layer &layer);

  RoutingVertex *GenerateGridVertexForPoint(
      const geometry::Point &point, const geometry::Layer &layer);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object.
  RoutingPath *ShortestPath(
      RoutingVertex *begin, RoutingVertex *end);

  // Takes ownership of the given object and accounts for the path's resources
  // as used.
  void InstallPath(RoutingPath *path);

  void AddTrackToLayer(RoutingTrack *track, const geometry::Layer &layer);

  // Stores the connection info between the ith (first index) and jth (second
  // index) layers. The "lesser" layer (std::less) should always be used to
  // index first, so that half of the matrix can be avoided.
  std::map<geometry::Layer, std::map<
      geometry::Layer, RoutingViaInfo>> via_infos_;

  std::map<geometry::Layer, RoutingLayerInfo> routing_layer_info_;

  // All installed paths (which we also own).
  std::vector<RoutingPath*> paths_;

  // Edges that do not fall on tracks, so we own them (until they are contained
  // in a RoutingPath).
  std::set<RoutingEdge*> off_grid_edges_;

  // All owned vertices.
  std::vector<RoutingVertex*> vertices_;

  // All routing tracks (we own these).
  std::map<geometry::Layer, std::vector<RoutingTrack*>> tracks_by_layer_;

  // The list of all available vertices per layer.
  std::map<geometry::Layer, std::vector<RoutingVertex*>> available_vertices_by_layer_;

  const PhysicalPropertiesDatabase &physical_db_;
};

}  // namespace bfg

#endif  // ROUTING_GRID_H_
