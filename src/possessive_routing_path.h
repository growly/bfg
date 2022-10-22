#ifndef OWNING_ROUTING_PATH_H_
#define OWNING_ROUTING_PATH_H_

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
class PossessiveRoutingPath {
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
  //
  // Update: It bit me in the ass. Is this even a useful thing to have?
  PossessiveRoutingPath(
      RoutingVertex *start, const std::deque<RoutingEdge*> edges);
  ~PossessiveRoutingPath();

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

std::ostream &operator<<(std::ostream &os, const PossessiveRoutingPath &path);

}  // namespace bfg

#endif  // OWNING_ROUTING_PATH_H_
