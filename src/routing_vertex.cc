#include "routing_vertex.h"

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

}  // namespace bfg
