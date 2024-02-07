#include "routing_vertex.h"

#include "geometry/compass.h"
#include "geometry/point.h"
#include "routing_edge.h"

namespace bfg {

bool RoutingVertex::RemoveEdge(RoutingEdge *edge) {
  size_t erased = edges_.erase(edge) > 0;
  return erased > 0;
}

int64_t RoutingVertex::L1DistanceTo(const geometry::Point &point) {
  return centre_.L1DistanceTo(point);
}

void RoutingVertex::AddNeighbour(const geometry::Compass &position,
                                 RoutingVertex *vertex) {
  if (!vertex) {
    return;
  }
  neighbours_.push_back({
      .position = position,
      .vertex = vertex
  });
}

std::set<RoutingVertex*> RoutingVertex::GetNeighbours(
    const geometry::Compass &position) const {
  std::set<RoutingVertex*> neighbours;
  for (const auto &neighbour : neighbours_) {
    if (neighbour.position == position) {
      neighbours.insert(neighbour.vertex);
    }
  }
  return neighbours;
}

std::set<RoutingVertex*> RoutingVertex::GetNeighbours() const {
  std::set<RoutingVertex*> neighbours;
  for (const auto &neighbour : neighbours_) {
    neighbours.insert(neighbour.vertex);
  }
  return neighbours;
}

std::ostream &operator<<(std::ostream &os, const RoutingVertex &vertex) {
  os << vertex.centre();
  if (!vertex.available()) {
    os << " net: \"" << vertex.net() << "\"";
  }
  return os;
}

}  // namespace bfg
