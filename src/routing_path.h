#ifndef ROUTING_PATH_H_
#define ROUTING_PATH_H_

#include <deque>
#include <sstream>
#include <vector>
#include <memory>

#include "geometry/layer.h"
#include "geometry/poly_line.h"
#include "geometry/port.h"
#include "routing_vertex.h"
#include "routing_edge.h"
#include "equivalent_nets.h"

namespace bfg {

class AbstractVia;
class RoutingGrid;

// Edges are NOT directed.
class RoutingPath {
 public:
  RoutingPath(
      RoutingVertex *start,
      const std::deque<RoutingEdge*> edges,
      RoutingGrid *routing_grid);

  RoutingVertex *Begin() const {
    return Empty() ? nullptr : vertices_.front();
  }
  RoutingVertex *End() const {
    return Empty() ? nullptr : vertices_.back();
  }

  double Cost() const;

  void Legalise();

  void ToPolyLinesAndVias(
      std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines,
      std::vector<std::unique_ptr<AbstractVia>> *vias) const;

  void ToPointsAndLayers(
      std::vector<geometry::Point> *points,
      std::vector<geometry::Layer> *layers) const;

  bool Empty() const { return edges_.empty(); }

  const geometry::Port *start_port() const { return start_port_; }
  void set_start_port(const geometry::Port *port) { start_port_ = port; }
  const geometry::Port *end_port() const { return end_port_; }
  void set_end_port(const geometry::Port *port) { end_port_ = port; }

  std::set<geometry::Layer> &start_access_layers() {
    return start_access_layers_;
  }
  const std::set<geometry::Layer> &start_access_layers() const {
    return start_access_layers_;
  }
  std::set<geometry::Layer> &end_access_layers() {
    return end_access_layers_;
  }
  const std::set<geometry::Layer> &end_access_layers() const {
    return end_access_layers_;
  }

  void set_encap_start_port(bool encap_start_port) {
    encap_start_port_ = encap_start_port;
  }
  bool encap_start_port() const { return encap_start_port_; }

  void set_encap_end_port(bool encap_end_port) {
    encap_end_port_ = encap_end_port;
  }
  bool encap_end_port() const { return encap_end_port_; }

  bool legalised() const { return legalised_; }

  void set_nets(const EquivalentNets &nets) { nets_ = nets; }
  const EquivalentNets &nets() const { return nets_; }

  const std::vector<RoutingVertex*> &vertices() const { return vertices_; }
  const std::vector<RoutingEdge*> &edges() const { return edges_; }

  std::string Describe() const;

 private:
  void BuildVias(
      geometry::PolyLine *from_poly_line,
      const geometry::Point &at_point,
      const geometry::Layer &to_layer,
      bool encap_last_layer,
      RoutingTrackDirection encap_direction,
      std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
      std::vector<std::unique_ptr<AbstractVia>> *vias) const;

  geometry::Layer PickAccessLayer(
      const geometry::Layer &source_layer,
      const std::set<geometry::Layer> &layers) const;

  void CheckEdgeInPolyLineForIncidenceOfOtherPaths(
      geometry::PolyLine *last,
      RoutingEdge *edge,
      std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const;

  // A nice property of the routing-grid is that paths should not be able to
  // conflict with themselves during the shortest-path search. This is
  // violated by off-grid vertices, since using them precludes the use of nearby
  // vertices on the grid. This is normally not a problem because long edges are
  // used to get away from the off-grid site to some far-away target point.
  // There are, however, exceptions:
  //
  //        +     +     +     +
  //                    |
  //        +-----+-----+     +
  //       B|
  //        +-----+--+ A+ +---+
  //                O+    |C
  //
  // In this example, the path B could be constructed to connect to the off-grid
  // point O given that on-grid vertex A is invalidated by an existing path C.
  //
  // We do a "peephole optimisation" here and remove any cases of
  // obviously-simplifiable edges cases. We're looking for overlapping edges in
  // the same direction:
  //      +-------+
  //           +--+
  // We will restrict ourselves to the case where they are separated by one
  // orthogonal edge. And we will replace that edge if we find the overlapping
  // case.
  bool AbbreviateOnce();
  bool MaybeAbbreviate(size_t starting_index);

  // Continue to Abbreviate from the start until it is not possible.
  inline void Abbreviate() { while (AbbreviateOnce()); }

  // For internal methods that implement the abbreviation refer to this diagram
  // for the names of objects:
  //         +     +     +     +
  //                  a  |
  // track c +-----+--x--+ d   +      v
  //         |                        | separation
  //         +-----+--+  + +---+      ^
  //                  b    |
  //
  geometry::Layer DetermineLayerForAbbreviation(
      size_t starting_index,
      RoutingVertex *vertex_b,
      RoutingVertex *vertex_d,
      RoutingTrack *track_c);

  std::optional<std::pair<int64_t, int64_t>> GetOverlapOrSkipAbbreviation(
      size_t starting_index);

  RoutingEdge *MaybeMakeAbbreviatingEdge(
      RoutingVertex *bridging_vertex,
      RoutingVertex *vertex_b,
      const geometry::Layer &target_layer);

  void InstallAbbreviation(size_t starting_index,
                           bool new_edge_first,
                           RoutingVertex *bridging_vertex,
                           RoutingEdge *new_edge,
                           RoutingEdge *edge_a_d);

  // Remove illegal (and inefficient) jogs between tracks.
  void Flatten();

  // TODO(aryap): I don't think these port objects are needed? We get most of
  // the info from start/end layer. Possibly if these are provided are they are
  // non-standard we need to provide bigger pours on the layers that connect to
  // them. Then again, the port defines the access region, not the via, that is
  // defined by the RoutingViaInfo object given to RoutingGrid. So no, these are
  // basically useless?
  const geometry::Port *start_port_;
  const geometry::Port *end_port_;
  // The start and end layers inform where vias need to be created in order for
  // the start and end points to be reachable.
  std::set<geometry::Layer> start_access_layers_;
  std::set<geometry::Layer> end_access_layers_;

  EquivalentNets nets_;

  // If true, join paths that are too short directly on the preceding/succeeding
  // layer (which must be the same).
  bool abbreviate_paths_;

  bool encap_start_port_;
  bool encap_end_port_;

  bool legalised_;

  // The ordered list of vertices making up the path. The edges alone, since
  // they are undirected, do not yield this directional information.
  // These vertices are NOT OWNED by RoutingPath.
  std::vector<RoutingVertex*> vertices_;

  // The list of edges. Edge i connected vertices_[j] and vertices_[j+1].
  // These edges are NOT OWNED by RoutingPath.
  std::vector<RoutingEdge*> edges_;

  // Convenient bookkeeping: the set of vertices from vertices_ that we do not
  // expect to have a via (since they don't represent a change in layer).
  std::set<RoutingVertex*> skipped_vias_;

  RoutingGrid *routing_grid_;
};

std::ostream &operator<<(std::ostream &os, const RoutingPath &path);

}  // namespace bfg

#endif  // ROUTING_PATH_H_
