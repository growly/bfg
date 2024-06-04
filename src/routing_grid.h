#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

#include <absl/status/status.h>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"
#include "layout.h"
#include "physical_properties_database.h"
#include "poly_line_cell.h"
#include "routing_edge.h"
#include "routing_grid_geometry.h"
#include "routing_track_blockage.h"
#include "routing_layer_info.h"
#include "routing_vertex.h"
#include "routing_via_info.h"

// TODO(aryap): Another version of this RoutingGrid should exist that uses a
// more standard model of the routing fabric. Instead of generating 1 edge for
// every possible wire length, represent every wire segment as an edge and vias
// as an edge to a vertex on an upper layer. Via cost is captured by the edge
// between layers. This will reduce memory demand but possibly will increase
// compute time cost as the breadth-first fan out from a starting node will
// include a lot more false paths, I'm not sure. The disadvantage of this (and
// I think why I avoided it the first time) is that it will be harder to assign
// non-linear cost to edges based on their length.
//
// TODO(aryap): Multi-point routing asks us to find the lowest overall cost for
// a tree that connects N different points.
//    - It's not minimum spanning tree because we have (and should use) the
//    option of crossing intermediary vertices to shorten the path.
//    - It's not Travelling Salesman because, again, we can use intermediary
//    vertices and we can revisit them (also no cycle but whatever).
// So what is it?  One immediately-useful option is to find the longest route
// first and then find paths to that route from the others (this is effectively
// the max-cut of the subgraph?) This will be easier with the 2.5D model but
// possible with this model with modifications: every vertex needs to know
// which edges span it so we can check which net it will belong to, and each
// edge needs to know which vertex it spans.
//
// This is the (rectilinear) steiner tree problem:
//    https://en.wikipedia.org/wiki/Rectilinear_Steiner_tree
//
// In the short term it will be enough to give users a way to simply connect a
// point to an existing net, I think?

namespace bfg {

using geometry::Layer;
using geometry::PolyLine;

class PossessiveRoutingPath;
class RoutingTrack;
class RoutingPath;

template<typename T>
class RoutingGridBlockage;

class RoutingGrid {
 public:
  RoutingGrid(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  ~RoutingGrid();

  // Connecting two layers generates the graph that describes all the paths one
  // can take between them; concretely, it creates a vertex every time a
  // horizontal and vertical routing line cross. (The two described layers must
  // be orthogonal in routing direction.)
  bool ConnectLayers(
      const geometry::Layer &first, const geometry::Layer &second);

  absl::Status AddRouteBetween(
      const geometry::Port &begin,
      const geometry::Port &end,
      const std::set<geometry::Port*> &avoid,
      const std::string &net = "");
  absl::Status AddRouteToNet(
      const geometry::Port &begin,
      const std::string &net,
      const std::set<geometry::Port*> &avoid);

  void AddVertex(RoutingVertex *vertex);

  bool RemoveVertex(RoutingVertex *vertex, bool and_delete);

  bool ContainsVertex(RoutingVertex *vertex) const;

  // Caller takes ownership.
  PolyLineCell *CreatePolyLineCell() const;

  Layout *GenerateLayout() const;

  // Add permanent blockages. Ports need special consideration and are not
  // added from the ShapeCollection by default.
  void AddBlockages(const geometry::ShapeCollection &shapes,
                    int64_t padding = 0,
                    bool is_temporary = false,
                    std::set<RoutingVertex*> *changed_out = nullptr);
  RoutingGridBlockage<geometry::Rectangle> *AddBlockage(
      const geometry::Rectangle &rectangle,
      int64_t padding = 0,
      bool is_temporary = false,
      std::set<RoutingVertex*> *blocked_vertices = nullptr,
      std::set<RoutingEdge*> *blocked_edges = nullptr);
  RoutingGridBlockage<geometry::Polygon> *AddBlockage(
      const geometry::Polygon &polygon,
      int64_t padding = 0,
      bool is_temporary = false,
       std::set<RoutingVertex*> *blocked_vertices = nullptr);

  // Removes the blockage from the list of known blockages, but does not undo
  // any effects of the blockage if they've already been applied.
  template <typename T>
  void ForgetBlockage(RoutingGridBlockage<T> *blockage);

  // TODO(aryap): Why do I always use templates instead of polymorphism? This
  // isn't what CS101 taught me. Am I sick?
  //
  // Edges blockages are managed by RoutingTracks. Directionality of access is
  // therefore implemented by RoutingTracks.
  template<typename T>
  void ApplyBlockage(
      const RoutingGridBlockage<T> &blockage,
      std::set<RoutingVertex*> *blocked_vertices = nullptr);

  // Check if the given routing vertex or edge clears all known explicit
  // blockages.
  bool ValidAgainstKnownBlockages(const RoutingEdge &edge) const;

  bool ValidAgainstKnownBlockages(
      const RoutingVertex &vertex,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt)
      const;

  bool ValidAgainstInstalledPaths(const RoutingEdge &edge) const;

  bool ValidAgainstInstalledPaths(
      const RoutingVertex &vertex,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt)
      const;

  std::optional<double> FindViaStackCost(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;
  std::optional<std::vector<RoutingViaInfo>> FindViaStack(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  void RemoveUnavailableVertices();

  void ExportVerticesAsSquares(
      const std::string &layer,
      bool available_only,
      Layout *layout) const;
  void ExportEdgesAsRectangles(
      const std::string &layer,
      bool available_only,
      Layout *layout) const;

  void AddRoutingViaInfo(const geometry::Layer &lhs,
                         const geometry::Layer &rhs,
                         const RoutingViaInfo &info);

  const RoutingViaInfo &GetRoutingViaInfoOrDie(const AbstractVia &via) const {
    return GetRoutingViaInfoOrDie(via.bottom_layer(), via.top_layer());
  }
  const RoutingViaInfo &GetRoutingViaInfoOrDie(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  std::optional<std::reference_wrapper<const RoutingViaInfo>>
  GetRoutingViaInfo(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  void AddRoutingLayerInfo(const RoutingLayerInfo &info);
  const RoutingLayerInfo &GetRoutingLayerInfoOrDie(
      const geometry::Layer &layer) const;
  std::optional<std::reference_wrapper<const RoutingLayerInfo>>
      GetRoutingLayerInfo(const geometry::Layer &layer) const;

  const std::vector<RoutingPath*> &paths() const { return paths_; }
  const std::set<RoutingEdge*> &off_grid_edges() const {
    return off_grid_edges_;
  }
  const std::vector<RoutingVertex*> &vertices() const { return vertices_; }

  const PhysicalPropertiesDatabase &physical_db() const { return physical_db_; }

  std::pair<std::reference_wrapper<const RoutingLayerInfo>,
            std::reference_wrapper<const RoutingLayerInfo>>
      PickHorizontalAndVertical(
          const geometry::Layer &lhs, const geometry::Layer &rhs) const;

 private:
  struct CostedVertex {
    uint64_t cost;
    geometry::Layer layer;
    RoutingVertex *vertex;
  };

  struct CostedLayer {
    geometry::Layer layer;
    double cost;
  };

  struct VertexWithLayer {
    RoutingVertex *vertex;
    geometry::Layer layer;
  };

  struct TemporaryBlockageInfo {
    std::vector<RoutingGridBlockage<geometry::Rectangle>*> pin_blockages;
    std::set<RoutingVertex*> blocked_vertices;
    std::set<RoutingEdge*> blocked_edges;
  };

  // FIXME(aryap): We take a via's footprint to the max of the metal overlap on
  // either side, in either direction. The test for whether we can access a via
  // actually depends on the metal layer we're connecting to it from. Given a
  // blockage on a layer, we need to know what the rules are for metal cover of
  // the via on that layer. We also need to be able to force connection to a via
  // in a certain direction to allow certain configuration and disallow others.
  // E.g. we might be forced to only allow horizontal connections at a via
  // because of a nearby blockage vertically. This means we have to give
  // vertices allowable directions of connection, which removes edges in other
  // directions. We store directions of neighbours so this is half done. More
  // annoyingly, a Via "footprint" is no longer a simple function, and this
  // function must return allowable directions. The obstruction encodes the
  // layer we need to connect form already so that's good.

  // TODO(aryap): It feels awkward putting these geometric functions here...?
  // Maybe move them into RoutingVertex? That requires giving RoutingVertex
  // awareness of geometry, which is like a loss of innocence y'know?
  //
  // Test if a given obstruction overlaps an appropriately-sized via at the
  // location of the given RoutingVertex.
  template<typename T>
  bool ViaWouldIntersect(
      const RoutingVertex &vertex,
      const T &obstruction,
      int64_t padding = 0,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt)
      const {
    if (!vertex.ConnectsLayer(obstruction.layer())) {
      return false;
    }
    // Note that we subtract 1 from the padding. This is because spacing rules
    // between objects seem to implicitly be inclusive of the end points.  Given
    // two rectangles with boundaries at x = 5 and x = 10, for example:
    //
    //        x=5     x=10
    //         v       v
    // --------+       +--------
    //         |       |
    //         |       |
    //         |       |
    // --------+       +--------
    //
    // The difference between them is 10 - 5 = 5, which is how the minimum
    // separation is calculated. But there are actually only 4 pixels
    // (unit-area-positions) between them! Testing for collisions within the
    // keep out requires testing for the rectangle inflated by 5 - 1 = 4, not by
    // just 5.
    std::optional<geometry::Rectangle> keep_out = ViaFootprint(
        vertex,
        obstruction.layer(),
        std::max(padding - 1, 0L),
        access_direction);
    if (!keep_out) {
      // Vertex is not a valid via:
      return false;
    }
    return obstruction.Overlaps(keep_out.value());
  }

  template<typename T>
  bool WireWouldIntersect(const RoutingEdge &edge,
                          const T &obstruction,
                          int64_t padding = 0) const {
    if (edge.layer() != obstruction.layer()) {
      return false;
    }
    // Consider the edge as a rectangle of the appropriate width for that edge
    // (i.e. given the wire width rules for its layer), and see if it collides
    // with the obstruction.
    std::optional<geometry::Rectangle> keep_out =
        EdgeFootprint(edge, padding > 0 ? padding - 1: padding);
    if (!keep_out) {
      return false;
    }
    return obstruction.Overlaps(keep_out.value());
  }

  template<typename T>
  void ApplyBlockage(const RoutingGridBlockage<T> &blockage);

  bool ConnectToSurroundingTracks(
      const RoutingGridGeometry &grid_geometry,
      const geometry::Layer &access_layer,
      std::optional<
          std::reference_wrapper<
              const std::set<RoutingTrackDirection>>> directions,
      RoutingVertex *off_grid);

  std::optional<geometry::Rectangle> ViaFootprint(
      const geometry::Point &centre,
      const geometry::Layer &first_layer,
      const geometry::Layer &second_layer,
      int64_t padding = 0,
      std::optional<RoutingTrackDirection> direction = std::nullopt) const;
  // FIXME(aryap): The RoutingVertex should contain enough information to figure
  // this out. It should at *least* always have its bottom and top layers
  // (ordered). the "connected_layers_" field is hard to use.
  std::optional<geometry::Rectangle> ViaFootprint(
      const RoutingVertex &vertex,
      const geometry::Layer &layer,
      int64_t padding = 0,
      std::optional<RoutingTrackDirection> direction = std::nullopt) const;
  std::optional<geometry::Rectangle> TrackFootprint(
      const RoutingEdge &edge, int64_t padding = 0) const;
  std::optional<geometry::Rectangle> EdgeFootprint(
      const RoutingEdge &edge, int64_t padding = 0) const;

  // Returns the via layer (first entry) and cost (second entry) if a via is
  // necessary/available to connect the given layers.
  std::optional<std::pair<geometry::Layer, double>> ViaLayerAndCost(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  std::vector<RoutingVertex*> &GetAvailableVertices(
      const geometry::Layer &layer);

  std::optional<VertexWithLayer> ConnectToGrid(const geometry::Port &port);

  std::optional<VertexWithLayer> AddAccessVerticesForPoint(
      const geometry::Point &point, const geometry::Layer &layer);

  std::optional<VertexWithLayer> ConnectToNearestAvailableVertex(
      const geometry::Port &port);

  RoutingVertex *ConnectToNearestAvailableVertex(
      const geometry::Point &point, const geometry::Layer &layer);

  void AddRoutingGridGeometry(
      const geometry::Layer &lhs, const geometry::Layer &rhs,
      const RoutingGridGeometry &grid_geometry);

  std::optional<std::reference_wrapper<RoutingGridGeometry>>
      GetRoutingGridGeometry(
          const geometry::Layer &lhs, const geometry::Layer &rhs);

  // Hands out pointers to RoutingGridGeometries that have tracks in either the
  // horizontal or vertical direction on the given layer.
  std::vector<RoutingGridGeometry*> FindRoutingGridGeometriesUsingLayer(
      const geometry::Layer &layer);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object.
  RoutingPath *ShortestPath(RoutingVertex *begin, RoutingVertex *end);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object. Places the actual target eventually decided on into
  // *discovered_target.
  RoutingPath *ShortestPath(
      RoutingVertex *from,
      const std::string &to_net,
      RoutingVertex **discovered_target);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object. Places the actual target eventually decided on into
  // *discovered_target. The various lambdas control what counts as a target and
  // which vertices and edges are valid for traversal.
  RoutingPath *ShortestPath(
      RoutingVertex *start,
      std::function<bool(RoutingVertex*)> is_target,
      RoutingVertex **discovered_target,
      std::function<bool(RoutingVertex*)> usable_vertex,
      std::function<bool(RoutingEdge*)> usable_edge,
      bool target_must_be_usable);

  void InstallPath(RoutingPath *path);

  void InstallVertexInPath(RoutingVertex *vertex);

  void AddTrackToLayer(RoutingTrack *track, const geometry::Layer &layer);

  bool PointsAreTooCloseForVias(
      const geometry::Layer &shared_layer,
      const geometry::Point &lhs,
      const geometry::Layer &lhs_connectee,
      const geometry::Point &rhs,
      const geometry::Layer &rhs_connectee) const;
  bool VerticesAreTooCloseForVias(
      const RoutingVertex &lhs, const RoutingVertex &rhs) const;

  void SetUpTemporaryBlockages(
      const std::set<geometry::Port*> &avoid,
      TemporaryBlockageInfo *blockage_info);

  void TearDownTemporaryBlockages(const TemporaryBlockageInfo &blockage_info);

  std::vector<CostedLayer> LayersReachableByVia(
      const geometry::Layer &from_layer) const;

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

  // All Owned vertices.
  std::vector<RoutingVertex*> vertices_;

  // All routing tracks (we own these).
  std::map<geometry::Layer, std::vector<RoutingTrack*>> tracks_by_layer_;

  // The list of all available vertices per layer.
  std::map<geometry::Layer,
           std::vector<RoutingVertex*>> available_vertices_by_layer_;

  // The lesser layer is always indexed first to save space; make sure to order
  // layer pairs before doing any lookups (same as with via_infos_).
  std::map<geometry::Layer, std::map<geometry::Layer, RoutingGridGeometry>>
      grid_geometry_by_layers_;

  std::vector<std::unique_ptr<RoutingGridBlockage<geometry::Rectangle>>>
      rectangle_blockages_;
  std::vector<std::unique_ptr<RoutingGridBlockage<geometry::Polygon>>>
      polygon_blockages_;

  // TODO(aryap): We need to track which directions a vertex can be used in,
  // since sometimes a horizontal via encap will fit but a vertical one will
  // not.
  std::set<RoutingTrackDirection> connectable_directions_;

  const PhysicalPropertiesDatabase &physical_db_;

  template<typename T>
  friend class RoutingGridBlockage;

  friend class RoutingPath;
};

template<typename T>
class RoutingGridBlockage {
 public:
  RoutingGridBlockage(
      const RoutingGrid &routing_grid, const T& shape, int64_t padding)
      : routing_grid_(routing_grid),
        shape_(shape),
        padding_(padding) {}

  ~RoutingGridBlockage();

  bool BlocksWithoutPadding(
      const RoutingVertex &vertex,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt)
      const {
    return Blocks(vertex, 0, access_direction);
  }
  bool BlocksWithoutPadding(const RoutingEdge &edge) const {
    return Blocks(edge, 0);
  }

  bool Blocks(
      const RoutingVertex &vertex,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt)
      const {
    return Blocks(vertex, padding_, access_direction);
  }
  bool Blocks(const RoutingEdge &edge) const {
    return Blocks(edge, padding_);
  }

  // Takes ownership of the given RoutingTrackBlockage. Store the RoutingTrack
  // so that we can remove the blockage from the track if we need do.
  void AddChildTrackBlockage(
      RoutingTrack *track, RoutingTrackBlockage *blockage);

  void ClearChildTrackBlockages();

  const T& shape() const { return shape_; }
  const int64_t &padding() const { return padding_; }

 private:
  bool Blocks(
      const RoutingVertex &vertex,
      int64_t padding,
      std::optional<RoutingTrackDirection> access_direction) const;
  bool Blocks(const RoutingEdge &edge, int64_t padding) const;

  const RoutingGrid &routing_grid_;
  // We store a copy of the shape. We can't store a reference because callers
  // can do cowboy shit.
  const T shape_;
  int64_t padding_;

  std::vector<std::pair<RoutingTrack*, std::unique_ptr<RoutingTrackBlockage>>>
      child_track_blockages_;
};

}  // namespace bfg

#endif  // ROUTING_GRID_H_
