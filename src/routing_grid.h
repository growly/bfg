#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "equivalent_nets.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"
#include "geometry/group.h"
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
  absl::Status ConnectLayers(
      const geometry::Layer &first, const geometry::Layer &second);

  // Convenient form AddMultiPointRoute which determines all net aliases for
  // the given ports and uses the given Layout to find all off-net ports to
  // use as temporary obstacles.
  absl::Status AddMultiPointRoute(
      const Layout &layout,
      const std::vector<std::vector<geometry::Port*>> ports,
      const std::optional<std::string> &primary_net_Name);

  // Connect multiple ports on the same net. One port from each successive set
  // of ports is connected.
  // 
  // Routing is successful if a route is found that connects at least one port
  // from each set.
  absl::Status AddMultiPointRoute(
      const std::vector<std::vector<geometry::Port*>> ports,
      const geometry::ShapeCollection &avoid,
      const EquivalentNets &nets);

  absl::Status AddBestRouteBetween(
      const geometry::PortSet &begin_ports,
      const geometry::PortSet &end_ports,
      const geometry::ShapeCollection &avoid,
      const EquivalentNets &nets);

  absl::Status AddRouteBetween(
      const geometry::Port &begin,
      const geometry::Port &end,
      const geometry::ShapeCollection &avoid,
      const EquivalentNets &nets);

  // target_nets: the nets we're trying to reach with the path.
  // usable_nets: the nets whose objects we can use to form the path. This
  // should be a superset of target_nets.
  absl::Status AddRouteToNet(
      const geometry::Port &begin,
      const EquivalentNets &target_nets,
      const EquivalentNets &usable_nets,
      const geometry::ShapeCollection &avoid);

  void AddVertex(RoutingVertex *vertex);

  void AddOffGridVertex(RoutingVertex *vertex);

  void AddOffGridEdge(RoutingEdge *edge);

  bool RemoveVertex(RoutingVertex *vertex, bool and_delete);

  bool ContainsVertex(RoutingVertex *vertex) const;

  // Caller takes ownership.
  PolyLineCell *CreatePolyLineCell() const;

  Layout *GenerateLayout() const;

  // Add permanent blockages. Ports need special consideration and are not
  // added by default.
  //
  // This also works for T = geometry::Group, since the code needed to use the
  // interface looks the same. The interface is not type-compatible because
  // ShapeCollection contains std::unique_ptr and Group contains straight
  // pointers. It would be nicer to simply provide converting accessors and use
  // a standard interface. As is this is kind of gross, sorry.
  //
  // Be careful when adding connectable shapes (i.e. pins) as "blockages"!
  template<typename T>
  void AddBlockages(
      const T &shapes,
      int64_t padding = 0,
      bool is_temporary = false,
      std::set<RoutingVertex*> *changed_out = nullptr) {
    for (const auto &rectangle : shapes.rectangles()) {
      AddBlockage(*rectangle, padding, is_temporary, changed_out);
    }
    for (const auto &polygon : shapes.polygons()) {
      AddBlockage(*polygon, padding, is_temporary, changed_out);
    }
    for (const auto &port : shapes.ports()) {
      AddBlockage(*port, padding);
    }
  }

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
  std::vector<RoutingGridBlockage<geometry::Rectangle>*> AddBlockage(
      const geometry::Port &port,
      int64_t padding = 0,
      bool is_temporary = false,
      std::set<RoutingVertex*> *blocked_vertices = nullptr,
      std::set<RoutingEdge*> *blocked_edges = nullptr);

  void ClearAllBlockages();

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
      bool is_temporary,
      std::set<RoutingVertex*> *blocked_vertices = nullptr);

  absl::Status ValidAgainstHazards(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt) const;

  // Check if the given routing vertex or edge clears all known explicit
  // blockages.
  absl::Status ValidAgainstKnownBlockages(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt)
      const;

  absl::Status ValidAgainstKnownBlockages(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt) const;

  absl::Status ValidAgainstInstalledPaths(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &for_nets = std::nullopt) const;

  absl::Status ValidAgainstInstalledPaths(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &for_nets = std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt) const;

  std::set<RoutingTrackDirection> ValidAccessDirectionsForVertex(
      const RoutingVertex &vertex,
      const EquivalentNets &for_nets) const;

  std::set<RoutingTrackDirection> ValidAccessDirectionsAt(
      const geometry::Point &point,
      const geometry::Layer &other_layer,
      const geometry::Layer &footprint_layer,
      const EquivalentNets &for_nets) const;

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

  absl::Status AddRoutingViaInfo(const geometry::Layer &lhs,
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

  absl::Status AddRoutingLayerInfo(const RoutingLayerInfo &info);
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

  // This is useful when you don't have a RoutingVertex connecting the 2 layers
  // you're interested in, such as the start/end of some RoutingPaths (which
  // might need a via stack):
  template<typename T>
  bool ViaWouldIntersect(
      const geometry::Point &centre,
      const geometry::Layer &other_layer,
      const geometry::Layer &footprint_layer,
      const T &obstruction,
      int64_t padding = 0,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
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
        centre,
        other_layer,
        obstruction.layer(),
        std::max(padding - 1, 0L),
        access_direction);
    if (!keep_out) {
      // Vertex is not a valid via:
      return false;
    }
    return obstruction.Overlaps(keep_out.value());
  }

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
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
    // Note that we subtract 1 from the padding, as above.
    if (!vertex.ConnectsLayer(obstruction.layer())) {
      return false;
    }
    std::optional<geometry::Rectangle> keep_out = VertexFootprint(
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

  // Gathers on-grid vertices and off-grid vertices within a given radius,
  // where the radius is given by the number of horizontal/vertical pitches on
  // on the routing grid geometry. Blockages are checked on the layer() of the
  // given shape.
  template<typename T>
  std::set<RoutingVertex*> BlockingOffGridVertices(const T &shape) const {
    int64_t min_separation = physical_db_.Rules(shape.layer()).min_separation;
    std::set<RoutingVertex*> vertices;
    for (RoutingVertex *off_grid : off_grid_vertices_) {
      if (ViaWouldIntersect(*off_grid,
                            shape,
                            min_separation,
                            std::nullopt)) {
        vertices.insert(off_grid);
      }
    }
    return vertices;
  }

  template<typename T>
  void ApplyBlockage(const RoutingGridBlockage<T> &blockage);

  template<typename T>
  void AddOffGridVerticesForBlockage(
      const RoutingGridGeometry &grid_geometry,
      const RoutingGridBlockage<T> &blockage,
      bool is_temporary);

  std::set<RoutingVertex*> BlockingOffGridVertices(
      const RoutingVertex &vertex,
      const geometry::Layer &layer,
      const std::optional<RoutingTrackDirection> direction) const;

  absl::StatusOr<RoutingPath*> FindRouteBetween(
      const geometry::Port &begin,
      const geometry::Port &end,
      const geometry::ShapeCollection &avoid,
      const EquivalentNets &nets);

  absl::StatusOr<RoutingPath*> FindRouteToNet(
      const geometry::Port &begin,
      const EquivalentNets &target_nets,
      const EquivalentNets &usable_nets,
      const geometry::ShapeCollection &avoid);

  absl::Status ConnectToSurroundingTracks(
      const RoutingGridGeometry &grid_geometry,
      const geometry::Layer &access_layer,
      const EquivalentNets &connectable_nets,
      const std::optional<
          std::reference_wrapper<
              const std::set<RoutingTrackDirection>>> &directions,
      RoutingVertex *off_grid);

  absl::Status ValidAgainstHazards(
      const geometry::Rectangle &footprint,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt)
      const;

  absl::Status ValidAgainstInstalledPaths(
      const geometry::Rectangle &footprint,
      const std::optional<EquivalentNets> &for_nets = std::nullopt) const;

  absl::Status ValidAgainstKnownBlockages(
      const geometry::Rectangle &footprint,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt)
      const;

  std::optional<geometry::Rectangle> ViaFootprint(
      const geometry::Point &centre,
      const geometry::Layer &first_layer,
      const geometry::Layer &second_layer,
      int64_t padding = 0,
      const std::optional<RoutingTrackDirection> &direction =
          std::nullopt) const;

  // FIXME(aryap): The RoutingVertex should contain enough information to figure
  // this out. It should at *least* always have its bottom and top layers
  // (ordered). the "connected_layers_" field is hard to use.
  std::optional<geometry::Rectangle> VertexFootprint(
      const RoutingVertex &vertex,
      const geometry::Layer &layer,
      int64_t padding = 0,
      const std::optional<RoutingTrackDirection> &direction =
          std::nullopt) const;

  std::optional<geometry::Rectangle> TrackFootprint(
      const RoutingEdge &edge, int64_t padding = 0) const;
  std::optional<geometry::Rectangle> EdgeFootprint(
      const RoutingEdge &edge, int64_t padding = 0) const;
  std::optional<geometry::Rectangle> EdgeWireFootprint(
      const RoutingEdge &edge, int64_t padding = 0) const;

  // Returns the via layer (first entry) and cost (second entry) if a via is
  // necessary/available to connect the given layers.
  std::optional<std::pair<geometry::Layer, double>> ViaLayerAndCost(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  std::vector<RoutingVertex*> &GetAvailableVertices(
      const geometry::Layer &layer);

  absl::StatusOr<VertexWithLayer> ConnectToGrid(
      const geometry::Port &port,
      const EquivalentNets &connectable_nets = EquivalentNets());

  absl::StatusOr<VertexWithLayer> AddAccessVerticesForPoint(
      const geometry::Point &point,
      const geometry::Layer &layer,
      const EquivalentNets &connectable_nets);

  absl::StatusOr<VertexWithLayer> ConnectToNearestAvailableVertex(
      const geometry::Port &port,
      const EquivalentNets &connectable_nets);

  absl::StatusOr<RoutingVertex*> ConnectToNearestAvailableVertex(
      const geometry::Point &point,
      const geometry::Layer &target_layer,
      const EquivalentNets &connectable_nets);

  absl::Status AddRoutingGridGeometry(
      const geometry::Layer &lhs, const geometry::Layer &rhs,
      const RoutingGridGeometry &grid_geometry);

  std::optional<std::reference_wrapper<RoutingGridGeometry>>
      GetRoutingGridGeometry(
          const geometry::Layer &lhs, const geometry::Layer &rhs);

  // FIXME(aryap): This addresses two problems that manifest in the same way:
  // 1) We not associate a target layer with a target net when labelling a
  // vertex; this means that we can't reason about which of two connected
  // layers we should use to connect to a vertex that's already on a net.
  // Sometimes that means we connect legally on e.g. met2, but create illegal
  // met1 vias too close to each other.
  //
  //             +-----+
  //            +|-----|+
  //   ---------+|     ||
  //   ---------+|     ||
  //            +|-----|+
  //             +-----+
  //            +|-----|+
  //            ||     |+--------
  //            ||     |+--------
  //            +|-----|+
  //             ++   ++
  //              |   |
  //               ^met2 vertical path existed, and the new path connects
  //               from the top left.
  //
  //  But now the vias on met1 are too close.
  //
  // 2) A similar situation happens when we try to beat a path to a net
  // attached to imported blockages that overlap on multiple layers. We don't
  // know which layer to connect to and so by connecting to any good one we
  // might cause problems on the others.
  //
  // If we did equip the RoutingVertex with knowledge of the layers on which
  // connects to nets, I think the most straightforward way to fix the problem
  // would still be to disable nearby vertices that could no longer accommodate
  // vias. But that can result in needlessly long paths (by a few pitches).
  //
  // What we do here is try to find these cases and patch them on the
  // unconnected layer, to avoid DRC violation.
  //
  // Let's see if this is a dumb idea...
  void ApplyDumbHackToPatchNearbyVerticesOnSameNetButDifferentLayer();

  // Hands out pointers to RoutingGridGeometries that have tracks in either the
  // horizontal or vertical direction on the given layer.
  std::vector<RoutingGridGeometry*> FindRoutingGridGeometriesUsingLayer(
      const geometry::Layer &layer);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object.
  absl::StatusOr<RoutingPath*> ShortestPath(
      RoutingVertex *begin, RoutingVertex *end);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object. Places the actual target eventually decided on into
  // *discovered_target.
  absl::StatusOr<RoutingPath*> ShortestPath(
      RoutingVertex *from,
      const EquivalentNets &to_nets,
      RoutingVertex **discovered_target);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object. Places the actual target eventually decided on into
  // *discovered_target. The various lambdas control what counts as a target and
  // which vertices and edges are valid for traversal.
  absl::StatusOr<RoutingPath*> ShortestPath(
      RoutingVertex *start,
      std::function<bool(RoutingVertex*)> is_target,
      RoutingVertex **discovered_target,
      std::function<bool(RoutingVertex*)> usable_vertex,
      std::function<bool(RoutingEdge*)> usable_edge,
      bool target_must_be_usable);

  absl::Status InstallPath(RoutingPath *path);

  void InstallVertexInPath(RoutingVertex *vertex, const std::string &net);

  void AddTrackToLayer(RoutingTrack *track, const geometry::Layer &layer);

  bool PointsAreTooCloseForVias(
      const geometry::Layer &shared_layer,
      const geometry::Point &lhs,
      const geometry::Layer &lhs_connectee,
      const geometry::Point &rhs,
      const geometry::Layer &rhs_connectee) const;
  bool VerticesAreTooCloseForVias(
      const RoutingVertex &lhs, const RoutingVertex &rhs) const;

  // FIXME(aryap): We need a 'plane' of blockages that are referred to even when
  // the routing grid is read-only, i.e. in a parallel search of the graph
  // according to slightly different blockages.
  //
  // This requires changing the 'available()' checks everywhere to also check
  // for membership in the blockage set.
  //
  // AddBlockages but it records the results to an external object?
  // Then we can multithread.
  void SetUpTemporaryBlockages(
      const geometry::ShapeCollection &avoid,
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

  // The vertices we know about which are off-grid. This container does not own
  // the pointers.
  std::set<RoutingVertex*> off_grid_vertices_;

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

  // Tests intersection of the blockage with a given point. This differs from
  // the RoutingVertex and the RoutingEdge tests because no footprint is
  // assumed: the point is either in the shape or not. (Margin is applied to the
  // shape before testing and can be negative, if the point needs to be "more
  // inside" to be considered a hit.)
  bool IntersectsPoint(const geometry::Point &point, int64_t margin) const;

  bool BlocksWithoutPadding(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets =
          std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
    return Blocks(vertex, 0, exceptional_nets, access_direction);
  }
  bool BlocksWithoutPadding(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt)
      const {
    return Blocks(edge, 0, exceptional_nets);
  }

  bool Blocks(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets =
          std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
    return Blocks(vertex, padding_, exceptional_nets, access_direction);
  }
  bool Blocks(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &exceptional_nets) const {
    return Blocks(edge, padding_, exceptional_nets);
  }
  bool Blocks(
      const geometry::Rectangle &footprint,
      const std::optional<EquivalentNets> &exceptional_nets) const {
    return Blocks(footprint, padding_, exceptional_nets);
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
      const std::optional<EquivalentNets> &exceptional_nets,
      const std::optional<RoutingTrackDirection> &access_direction) const;
  bool Blocks(
      const RoutingEdge &edge,
      int64_t padding,
      const std::optional<EquivalentNets> &exceptional_nets) const;
  bool Blocks(
      const geometry::Rectangle &footprint,
      int64_t padding,
      const std::optional<EquivalentNets> &exceptional_nets) const;

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
