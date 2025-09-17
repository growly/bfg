#include "route_manager.h"

#include <glog/logging.h>

#include "layout.h"

namespace bfg {

absl::StatusOr<int64_t> RouteManager::Connect(
    const geometry::Port &from,
    const geometry::Port &to,
    const EquivalentNets &as_nets) {

}

absl::StatusOr<int64_t> RouteManager::ConnectMultiplePorts(
    const std::vector<geometry::Port*> &ports,
    const EquivalentNets &nets,
    const std::optional<int64_t> priority) {
  NetRouteOrder order = {
    .net = nets
  };
  for (geometry::Port *port : ports) {
    order.nodes.emplace_back(std::set<const geometry::Port*>{port});
  }

  int64_t position = orders_.size();
  orders_.push_back(order);
  return position;
}

// Ok this is nice and is exactly what RouterSession does, but what I think I
// wanted in Interconnect::RouteComplete was to be able specify ports by
// instance/name and for something to automatically figure out whether those had
// been routed, and if not, route them and store the nets; and if so, just route
// to the nets. That isn't compatible with multithreading exactly, because order
// really matters. But if the first step is to factor out what I do there, maybe
// that's what this should be?
//
// Are there different kinds of order? A higher level function that converts the
// instance/name things into these orders. That makes sense, since that is just
// what's happening inline in the RouteComplete function.
//
// Seems like there are many different strategies for running a particular
// multi-point route request (a NetRouteOrder), and a suite of these types of
// functions should implement them. Then, at dispatch time, we pick which one.
// Intermixing them requires multiple NetRouteOrders.
absl::Status RouteManager::RunOrder(const NetRouteOrder &order) {
  if (order.nodes.size() < 2) {
    return absl::FailedPreconditionError("Not enough nodes in NetRouteOrder");
  }

  // Copy.
  EquivalentNets usable_nets = order.net;
  for (const auto &node : order.nodes) {
    // Add a net from the first port in each set, since the set of ports at each
    // node is considered equivalent.
    usable_nets.Add((*node.begin())->net());
  }

  RoutingBlockageCache child_blockage_cache(*routing_grid_,
                                            parent_blockage_cache_);

  // Another copy, so we can extract the shapes that aren't blocked.
  EquivalentNets ok_nets = usable_nets;
  // Not sure why I'm doing this:
  ok_nets.Add(layout_->global_nets());
  geometry::ShapeCollection ok_shapes;
  layout_->CopyConnectableShapesOnNets(ok_nets, &ok_shapes);
  child_blockage_cache.CancelBlockages(ok_shapes);

  // Targets are the set of nets that have already been routed, as opposed to
  // usable nets, which are the set of all the nets that will *be* routed.
  EquivalentNets target_nets;

  bool first_pair_routed = false;
  for (size_t i = 0; i < order.nodes.size() - 1; ++i) {
    const geometry::Port *from = *order.nodes[i + 1].begin();
    if (!first_pair_routed) {
      const geometry::Port *to = *order.nodes[i].begin();
      auto result = routing_grid_->AddRouteBetween(*from,
                                                   *to,
                                                   child_blockage_cache,
                                                   usable_nets);
      if (result.ok()) {
        first_pair_routed = true;
        target_nets.Add(from->net());
        target_nets.Add(to->net());

      } else {
        // Save for later? Come back and attempt at the end?
      }
    } else {
      auto result = routing_grid_->AddRouteToNet(*from,
                                                 target_nets,
                                                 usable_nets,
                                                 child_blockage_cache);
      if (result.ok()) {
        target_nets.Add(from->net());
      } else {
        // Save for later? Come back and attempt at the end?
      }
    }
  }
}

}  // namespace bfg
