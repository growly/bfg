#ifndef ROUTING_VERTEX_COLLECTOR_H_
#define ROUTING_VERTEX_COLLECTOR_H_

#include <deque>
#include <vector>
#include <memory>

#include "routing_vertex.h"

namespace bfg {

// This class automatically creates groups of vertices from successively added
// vertices based on some function that determines whether a given vertex
// belongs in the same group as the previous one. Any break in a sequence of
// grouped vertices yields a new group. Groups retain the order in which the
// vertices were offered.
class RoutingVertexCollector {
 public:
  RoutingVertexCollector(
      std::function<bool(RoutingVertex*, RoutingVertex*)> same_group)
    : same_group_(same_group) {}

  void Offer(RoutingVertex *vertex);

  const std::vector<std::vector<RoutingVertex*>> &groups() const {
    return groups_;
  }

 private:
  std::function<bool(RoutingVertex*, RoutingVertex*)> same_group_;
  std::vector<std::vector<RoutingVertex*>> groups_;
};

}  // namespace bfg

#endif  // ROUTING_VERTEX_COLLECTOR_H_
