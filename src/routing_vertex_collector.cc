#include <sstream>

#include "routing_vertex_collector.h"

#include "absl/strings/str_join.h"

namespace bfg {

void RoutingVertexCollector::Offer(RoutingVertex *offer) {
  LOG_IF(FATAL, !offer) << "Offering nullptr doesn't make any sense.";
  if (offer == previous_offer_) {
    // Ignore duplicates because that's easier than tracking offers at the
    // client end:
    return;
  }
  if (!previous_offer_ || !same_group_(previous_offer_, offer)) {
    // Initially no previous offers are made (and needs_new_group_ should
    // already be true). So we just have to record the first offer. Otherwise,
    // there is a break in consecutive grouping and we need to mark that a new
    // group is needed for next time. In all cases we must rotate the offer to
    // the previous.
    // TODO(aryap): Good candidate for a "defer". Maybe absl has the thing that
    // does this at end of scope? Or Maybe it's in c++ now?
    previous_offer_ = offer;
    needs_new_group_ = true;
    return;
  }
  if (needs_new_group_) {
    groups_.push_back({previous_offer_, offer});
    needs_new_group_ = false;
  } else {
    groups_.back().push_back(offer);
  }
  previous_offer_ = offer;
  return;
}

void LayeredRoutingVertexCollectors::Offer(
    const geometry::Layer &layer, RoutingVertex *vertex) {
  auto it = collectors_by_layer_.find(layer);
  if (it == collectors_by_layer_.end()) {
    auto insertion_it = collectors_by_layer_.insert(
        {layer, RoutingVertexCollector(
            [=](RoutingVertex *lhs, RoutingVertex *rhs) {
              return same_group_(layer, lhs, rhs);
            })});
    LOG_IF(FATAL, !insertion_it.second) << "Insertion into a map failed?!";
    it = insertion_it.first;
  }
  it->second.Offer(vertex);
}

std::map<geometry::Layer, std::vector<std::vector<RoutingVertex*>>>
    LayeredRoutingVertexCollectors::GroupsByLayer() const {
  std::map<geometry::Layer, std::vector<std::vector<RoutingVertex*>>>
      groups_by_layer;
  for (const auto &entry : collectors_by_layer_) {
    groups_by_layer[entry.first] = entry.second.groups();
  }
  return groups_by_layer;
}

std::string RoutingVertexCollector::Describe() const {
  std::stringstream ss;
  if (groups_.empty()) {
    ss << "No groups";
    return ss.str();
  }
  for (size_t i = 0; i < groups_.size(); ++i) {
    const std::vector<RoutingVertex*> &list = groups_.at(i);
    std::vector<std::string> vertex_centres;
    for (RoutingVertex *const vertex : list) {
      vertex_centres.push_back(vertex->centre().Describe());
    }
    ss << "Group " << i << ": [" << absl::StrJoin(vertex_centres, ", ") << "]";
    if (i < groups_.size() - 1) {
      ss << " ";
    }
  }
  return ss.str();
}

}  // namespace bfg
