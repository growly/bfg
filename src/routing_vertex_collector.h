#ifndef ROUTING_VERTEX_COLLECTOR_H_
#define ROUTING_VERTEX_COLLECTOR_H_

#include <deque>
#include <vector>
#include <memory>

#include "geometry/layer.h"
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
      : previous_offer_(nullptr),
        needs_new_group_(true),
        same_group_(same_group) {}

  void Offer(RoutingVertex *vertex);

  std::string Describe() const;

  const std::vector<std::vector<RoutingVertex*>> &groups() const {
    return groups_;
  }

 private:
  RoutingVertex *previous_offer_;
  bool needs_new_group_;
  std::function<bool(RoutingVertex*, RoutingVertex*)> same_group_;
  std::vector<std::vector<RoutingVertex*>> groups_;
};

class LayeredRoutingVertexCollectors {
 public:
  LayeredRoutingVertexCollectors(
      std::function<bool(const geometry::Layer &, RoutingVertex*, RoutingVertex*)>
          same_group)
      : same_group_(same_group) {}
  
  void Offer(const geometry::Layer &layer, RoutingVertex *vertex);

  std::map<geometry::Layer, std::vector<std::vector<RoutingVertex*>>>
      GroupsByLayer() const;

  const std::map<geometry::Layer, RoutingVertexCollector> &collectors_by_layer()
      const {
    return collectors_by_layer_;
  }

 private:
  std::function<bool(const geometry::Layer &, RoutingVertex*, RoutingVertex*)>
      same_group_;
  std::map<geometry::Layer, RoutingVertexCollector> collectors_by_layer_;
};

}  // namespace bfg

#endif  // ROUTING_VERTEX_COLLECTOR_H_
