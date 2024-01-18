#ifndef ROUTING_PATH_H_
#define ROUTING_PATH_H_

#include <deque>
#include <vector>
#include <memory>

#include "routing_vertex.h"
#include "routing_edge.h"

namespace bfg {

namespace geometry {

class Port;
class PolyLine;

}  // namespace geometry

class AbstractVia;
class RoutingGrid;

// Edges are NOT directed.
class RoutingPath {
 public:
  RoutingPath(RoutingVertex *start, const std::deque<RoutingEdge*> edges);

  RoutingVertex *Begin() const {
    return Empty() ? nullptr : vertices_.front();
  }
  RoutingVertex *End() const {
    return Empty() ? nullptr : vertices_.back();
  }

  void ToPolyLinesAndVias(
      const RoutingGrid &routing_grid,
      std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines,
      std::vector<std::unique_ptr<AbstractVia>> *vias) const;

  bool Empty() const { return edges_.empty(); }

  const geometry::Port *start_port() const { return start_port_; }
  void set_start_port(const geometry::Port *port) { start_port_ = port; }
  const geometry::Port *end_port() const { return end_port_; }
  void set_end_port(const geometry::Port *port) { end_port_ = port; }

  const std::optional<geometry::Layer> &start_access_layer() const {
    return start_access_layer_;
  }
  void set_start_access_layer(const std::optional<geometry::Layer> &layer) {
    start_access_layer_ = layer;
  }

  const std::optional<geometry::Layer> &end_access_layer() const {
    return end_access_layer_;
  }
  void set_end_access_layer(const std::optional<geometry::Layer> &layer) {
    end_access_layer_ = layer;
  }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

  const std::vector<RoutingVertex*> &vertices() const { return vertices_; }
  const std::vector<RoutingEdge*> &edges() const { return edges_; }

 private:
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
  std::optional<geometry::Layer> start_access_layer_;
  std::optional<geometry::Layer> end_access_layer_;

  std::string net_;

  // If true, join paths that are too short directly on the preceding/succeeding
  // layer (which must be the same).
  bool abbreviate_paths_;

  // The ordered list of vertices making up the path. The edges alone, since
  // they are undirected, do not yield this directional information.
  // These vertices are NOT OWNED by RoutingPath.
  std::vector<RoutingVertex*> vertices_;

  // The list of edges. Edge i connected vertices_[j] and vertices_[j+1].
  // These edges are NOT OWNED by RoutingPath.
  std::vector<RoutingEdge*> edges_;
};

std::ostream &operator<<(std::ostream &os, const RoutingPath &path);

}  // namespace bfg

#endif  // ROUTING_PATH_H_
