#include "routing_vertex_kd_tree.h"

#include "routing_vertex.h"
#include "geometry/point.h"

namespace bfg {

void RoutingVertexKDTree::Add(RoutingVertex *vertex) {
  tree_.insert(vertex);
  needs_optimise_ = true;
}

void RoutingVertexKDTree::Erase(RoutingVertex *vertex) {
  tree_.erase(vertex);
  needs_optimise_ = true;
}

std::vector<RoutingVertex*> RoutingVertexKDTree::FindNearby(
    const geometry::Point &reference, int64_t radius) const {
  Optimise();

  std::unique_ptr<RoutingVertex> ref_vertex(new RoutingVertex(reference));
  RoutingVertexKDNode ref_node(ref_vertex.get());

  std::vector<RoutingVertexKDNode> nearby_nodes;
  tree_.find_within_range(ref_node, radius, std::back_inserter(nearby_nodes));

  std::vector<RoutingVertex*> nearby;
  for (const auto &node : nearby_nodes) {
    nearby.push_back(node.vertex());
  }
  return nearby;
}

void RoutingVertexKDTree::Optimise() const {
  if (!needs_optimise_) {
    return;
  }
  tree_.optimise();
  needs_optimise_ = false;
}

}  // namespace bfg
