#include "route_manager.h"

#include <sstream>
#include <glog/logging.h>

#include "layout.h"
#include "geometry/shape_collection.h"

namespace bfg {

std::string NetRouteOrder::Describe() const {
  std::stringstream ss;
  ss << "Nets: " << net_ << std::endl;
  for (size_t i = 0; i < nodes_.size(); ++i) {
    const auto &node = nodes_[i];
    ss << "Step " << i << ": " << std::endl;
    for (const geometry::Port *port : node) {
      ss << "  " << *port << std::endl;
    }
  }
  return ss.str();
}

std::string RouteManager::DescribeOrders() const {
  std::stringstream ss;
  for (const auto &order : orders_) {
    ss << order.Describe();
  }
  return ss.str();
}

absl::StatusOr<int64_t> RouteManager::Connect(
    const geometry::Port &from,
    const geometry::Port &to,
    const EquivalentNets &as_nets) {
  NetRouteOrder order(as_nets);
  order.nodes().emplace_back(std::set<const geometry::Port*>{&from});
  order.nodes().emplace_back(std::set<const geometry::Port*>{&to});
  orders_.push_back(order);

  return 0;
}

absl::StatusOr<int64_t> RouteManager::ConnectMultiplePorts(
    const std::vector<geometry::Port*> &ports,
    const EquivalentNets &nets,
    const std::optional<int64_t> priority) {
  NetRouteOrder order(nets);
  for (geometry::Port *port : ports) {
    order.nodes().emplace_back(std::set<const geometry::Port*>{port});
  }

  int64_t position = orders_.size();
  orders_.push_back(order);
  return position;
}

absl::Status RouteManager::RunOrdersSequential() {
  for (const NetRouteOrder &order : orders_) {
    RunOrder(order).IgnoreError();
  }
  return absl::OkStatus();
}

// Ok this is nice and is exactly what RouterSession does, but what I think I
// wanted in Interconnect::RouteComplete was to be able specify ports by
// instance/name and for something to automatically figure out whether those had
// been routed, and if not, route them and store the nets; and if so, just route
// to the nets. That isn't compatible with multithreading exactly, because order
// really matters. But if the first step is to factor out what I do there, maybe
// that's what this should b
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
  if (order.nodes().size() < 2) {
    return absl::FailedPreconditionError("Not enough nodes in NetRouteOrder");
  }

  // Copy.
  EquivalentNets usable_nets = order.net();
  for (const auto &node : order.nodes()) {
    // Add a net from the first port in each set, since the set of ports at each
    // node is considered equivalent.
    usable_nets.Add((*node.begin())->net());
  }

  RoutingBlockageCache child_blockage_cache(*routing_grid_,
                                            root_blockage_cache_);

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
  for (size_t i = 0; i < order.nodes().size() - 1; ++i) {
    const geometry::Port *from = *order.nodes()[i + 1].begin();
    if (!first_pair_routed) {
      const geometry::Port *to = *order.nodes()[i].begin();
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

  return absl::OkStatus();
}

// The default configuration of the RoutingBlockageCache is to stage all
// connectable shapes as blockages, so that each NetRouteOrder can operate under
// a child RoutingBlockageCache with its net objects as exceptions.
void RouteManager::ConfigureRoutingBlockageCache() {
  geometry::ShapeCollection connectables;
  layout_->CopyConnectableShapes(&connectables);
  root_blockage_cache_.AddBlockages(connectables);
}

EquivalentNets *RouteManager::GetRoutedNetsByPort(
    const geometry::Port *port) const {
  auto it = routed_nets_by_port_.find(port);
  if (it == routed_nets_by_port_.end()) {
    return nullptr;
  }
  return it->second;
}

void RouteManager::MergeAndReplaceEquivalentNets(
    const std::set<EquivalentNets*> to_replace,
    EquivalentNets *replacement) {
  for (EquivalentNets *nets : to_replace) {
    replacement->Add(*nets);
  }

  // This being slow is solved by maintaining a structure with the reverse
  // relationship:
  for (auto &entry : routed_nets_by_port_) {
    EquivalentNets *existing = entry.second;
    if (to_replace.find(existing) == to_replace.end()) {
      continue;
    }
    entry.second = replacement;
  }
  // Delete the old objects, now that they have been merged into the
  // replacement.
  for (auto it = routed_nets_.begin(); it != routed_nets_.end();) {
    if (to_replace.find(it->get()) == to_replace.end()) {
      ++it;
    } else {
      it = routed_nets_.erase(it);
    }
  }
}

absl::Status RouteManager::Solve() {
  ConsolidateOrders().IgnoreError();

  RunOrdersSequential();

  return absl::OkStatus();
}

absl::Status RouteManager::ConsolidateOrders() {
  CollectConnectedNets().IgnoreError();

  std::vector<NetRouteOrder> consolidated;
  consolidated.reserve(orders_.size());

  // We only really need to store a reference to the NetRouteOrder here, but
  // for that to happen the order needs to exist somewhere with a stable
  // position. If we size the consolidated order vector so that it never grows,
  // we can rely on the pointers into that.
  std::map<EquivalentNets*, NetRouteOrder*> orders_by_net;
  std::set<const geometry::Port*> included_in_order;

  for (NetRouteOrder &order : orders_) {
    for (const auto &node : order.nodes()) {
      // We can consider all the ports of a node as equivalent for the point of
      // finding the EquivalentNets, if CollectConnectedNets did its job.
      const geometry::Port *port = *node.begin();

      if (included_in_order.find(port) != included_in_order.end()) {
        // This node was already moved to a new NetRouteOrder.
        continue;
      }

      auto nets_it = routed_nets_by_port_.find(port);
      LOG_IF(FATAL, nets_it == routed_nets_by_port_.end())
          << "By this stage all ports must appear in the "
          << "routed_nets_by_port_ map.";
      EquivalentNets *nets = nets_it->second;

      auto order_it = orders_by_net.find(nets);
      NetRouteOrder *replacement_order;
      if (order_it == orders_by_net.end()) {
        replacement_order = &consolidated.emplace_back();
        replacement_order->set_net(*nets);
        orders_by_net[nets] = replacement_order;
      } else {
        replacement_order = order_it->second;
      }
      replacement_order->nodes().push_back(node);

      included_in_order.insert(node.begin(), node.end());
    }
  }

  // Replace!
  orders_ = consolidated;

  return absl::OkStatus();
}

absl::Status RouteManager::CollectConnectedNets() {
  routed_nets_by_port_.clear();
  routed_nets_.clear();

  // Join all connections that are on the same net into one NetRouteOrder.
  for (NetRouteOrder &order : orders_) {
    // Start by assuming that none of the ports in this order have been routed
    // before. Create a single union of all of their nets.
    EquivalentNets *nets = routed_nets_.emplace_back(new EquivalentNets()).get();
    nets->Add(order.net());
    std::set<EquivalentNets*> to_merge;

    for (const auto &node : order.nodes()) {
      // The ports in a node should all be the same net. But for completeness:
      for (const geometry::Port *port : node) {
        nets->Add(port->net());

        // If the port is found to take part in some nets already, mark them for
        // merger:
        EquivalentNets *existing = GetRoutedNetsByPort(port);
        if (existing) {
          to_merge.insert(existing);
        } else {
          routed_nets_by_port_[port] = nets;
        }
      }
    }

    // If any of the ports in the are associated with an existing merger, we
    // have to merge them, and delete all but one.
    MergeAndReplaceEquivalentNets(to_merge, nets);
  }

  return absl::OkStatus();
}

}  // namespace bfg
