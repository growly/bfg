#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include "layout.h"
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
// In the short term it will be enough to give users a way to simply connect a
// point to an existing net, I think?

namespace bfg {

using geometry::Layer;
using geometry::PolyLine;

class PossessiveRoutingPath;
class RoutingEdge;
class RoutingTrack;
class RoutingTrackBlockage;
class RoutingPath;
class RoutingVertex;

struct RoutingLayerInfo {
  geometry::Layer layer;
  geometry::Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingTrackDirection direction;
  int64_t pitch;
  int64_t via_width;
};

class RoutingGrid {
 public:
  RoutingGrid(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  ~RoutingGrid();

  // Connecting two layers generates the graph that describes all the paths one
  // can take between them; concretely, it creates a vertex every time a
  // horizontal and vertical routing line cross. (The two described layers must
  // be orthogonal in routing direction.)
  void ConnectLayers(
      const geometry::Layer &first, const geometry::Layer &second);

  bool AddRouteBetween(
      const geometry::Port &begin,
      const geometry::Port &end,
      const std::string &net = "");
  bool AddRouteToNet(
      const geometry::Port &begin, const std::string &net);

  void AddVertex(RoutingVertex *vertex);

  bool RemoveVertex(RoutingVertex *vertex, bool and_delete);

  // Caller takes ownership.
  PolyLineCell *CreatePolyLineCell() const;

  Layout *GenerateLayout() const;

  void AddBlockages(const geometry::ShapeCollection &shapes,
                    int64_t padding = 0);
  void AddBlockage(const geometry::Rectangle &rectangle,
                   int64_t padding = 0);
  void AddBlockage(const geometry::Polygon &polygon,
                   int64_t padding = 0);
  // TODO(aryap): This might be a useful optimisation.
  void RemoveUnavailableVertices();

  void AddRoutingViaInfo(const geometry::Layer &lhs,
                         const geometry::Layer &rhs,
                         const RoutingViaInfo &info);

  const RoutingViaInfo &GetRoutingViaInfo(const AbstractVia &via) const {
    return GetRoutingViaInfo(via.bottom_layer(), via.top_layer());
  }
  const RoutingViaInfo &GetRoutingViaInfo(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  void AddRoutingLayerInfo(const RoutingLayerInfo &info);
  const RoutingLayerInfo &GetRoutingLayerInfo(
      const geometry::Layer &layer) const;

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

  std::vector<RoutingVertex*> &GetAvailableVertices(
      const geometry::Layer &layer);

  RoutingVertex *GenerateGridVertexForPoint(
      const geometry::Point &point, const geometry::Layer &layer);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object.
  RoutingPath *ShortestPath(RoutingVertex *begin, RoutingVertex *end);

  RoutingPath *ShortestPath(RoutingVertex *from, const std::string &to_net);

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

  // All Owned vertices.
  std::vector<RoutingVertex*> vertices_;

  // All routing tracks (we own these).
  std::map<geometry::Layer, std::vector<RoutingTrack*>> tracks_by_layer_;

  // The list of all available vertices per layer.
  std::map<geometry::Layer,
           std::vector<RoutingVertex*>> available_vertices_by_layer_;

  const PhysicalPropertiesDatabase &physical_db_;
};

}  // namespace bfg

#endif  // ROUTING_GRID_H_
