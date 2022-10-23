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

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

  const std::vector<RoutingVertex*> vertices() const { return vertices_; }
  const std::vector<RoutingEdge*> edges() const { return edges_; }

 private:
  // If these ports are provided, a via will be generated or the edge on the
  // given layer extended to correctly connect to them.
  const geometry::Port *start_port_;
  const geometry::Port *end_port_;

  std::string net_;

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
