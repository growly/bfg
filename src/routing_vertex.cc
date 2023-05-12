#include "routing_vertex.h"

#include "geometry/compass.h"
#include "geometry/point.h"
#include "routing_edge.h"

namespace bfg {

bool RoutingVertex::RemoveEdge(RoutingEdge *edge) {
  size_t erased = edges_.erase(edge) > 0;
  return erased > 0;
}

uint64_t RoutingVertex::L1DistanceTo(const geometry::Point &point) {
  // The L-1 norm, or Manhattan distance.
  int64_t dx = point.x() - centre_.x();
  int64_t dy = point.y() - centre_.y();
  return std::abs(dx) + std::abs(dy);
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

}  // namespace bfg
