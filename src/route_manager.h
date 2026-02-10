#ifndef ROUTE_MANAGER_H_
#define ROUTE_MANAGER_H_

#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include <absl/status/statusor.h>

#include "equivalent_nets.h"
#include "geometry/port.h"
#include "geometry/shape_collection.h"
#include "routing_blockage_cache.h"
#include "routing_grid.h"
#include "routing_path.h"

namespace bfg {

// TODO(aryap): There's a chance this repeats some of the work of RouterSession,
// which presents a similar interface over RPC. You should consolidate them.

  // UNRELATED TODO
  //  - shared_mutex in RoutingGrid
  //  - shared lock for reads
  //  - exclusive lock for installs
  //  - part of install should be a check that the path remains valid, because
  //  components on the path could have been invalidated by another install
  //  - on failure, need to indicate; sometimes a retry is in order, sometimes
  //  not

// The point of RouteManager is to take the burden of finding a collection of
// routes simultaneously, so that clients need only specify ports, nets, etc,
// and perhaps an ordering for the routes, then let the RouteManager figure it
// out.
//
// Typical usage modes:
// - Adding a multi-point route. User specifies list of (instance, port) by name
// that must be connected to the same net. Or they specify explicit port
// objects. The order should not matter, but it is up to our discretion to
// determine the best route. The best version of this will be a rectilinear
// steiner tree or something.
// - Connect a pair of (instance, port). If either pair is involved a route,
// connect to the existing net.
//
// Some routing can be specified as equal-priority, so it occurs in any order
// (probably at the same time). Otherwise a priority is implied (or explicit) to
// indicate the order in which the route should be attempted. This can help
// avoid congestion when the designer is aware of where it is likely. It is also
// largely obviated by more sophisticated meta-routing, like with simulated
// annealing, an ILP, etc.

struct NetRouteOrder {
 public:
  NetRouteOrder() = default;
  NetRouteOrder(const EquivalentNets &net)
      : net_(net) {}

  std::string Describe() const;

  void set_net(const EquivalentNets &net) { net_ = net; }

  EquivalentNets &net() { return net_; }
  const EquivalentNets &net() const { return net_; }

  std::vector<std::set<const geometry::Port*>> &nodes() { return nodes_; }
  const std::vector<std::set<const geometry::Port*>> &nodes() const {
    return nodes_;
  }

 private:
  // All of the equivalent nets on this route. The net.primary() string is used
  // as a canonical ID.
  EquivalentNets net_;

  // Each node is a set of equivalent ports. In principle any node from a set
  // can be used to connect, but in practice it should only be one.
  std::vector<std::set<const geometry::Port*>> nodes_;
};

// Maybe "RouteGovernor"?
class RouteManager {
 public:
  RouteManager(Layout *layout,
               RoutingGrid *routing_grid)
      : layout_(layout),
        routing_grid_(routing_grid),
        root_blockage_cache_(*routing_grid) {
    ConfigureRoutingBlockageCache();
  }

  // Stage required routes:
  absl::StatusOr<int64_t> ConnectMultiplePorts(
      const std::vector<geometry::Port*> &ports,
      const EquivalentNets &nets,
      const std::optional<int64_t> priority = std::nullopt);

  //absl::StatusOr<int64_t> ConnectMultiplePorts(
  //    const std::vector<std::pair<std::string, std::string>> port_names,
  //    const EquivalentNets &nets,
  //    const std::optional<int64_t> priority = std::nullopt);

  //absl::StatusOr<int64_t> ConnectPair(
  //    const geometry::Port &from,
  //    const geometry::Port &to,
  //    const EquivalentNets &nets,
  //    const std::optional<int64_t> priority = std::nullopt);

  //absl::StatusOr<int64_t> ConnectPair(
  //    const std::pair<std::string, std::string> &from,
  //    const std::pair<std::string, std::string> &to,
  //    const EquivalentNets &nets,
  //    const std::optional<int64_t> priority = std::nullopt);

  //absl::StatusOr<int64_t> ConnectToNet(
  //    const std::vector<std::set<geometry::Port*>> &ports,
  //    const EquivalentNets &net);

  absl::StatusOr<int64_t> Connect(
      const geometry::Port &from,
      const geometry::Port &to,
      const EquivalentNets &as_nets = {});

  absl::StatusOr<int64_t> Connect(
      const std::set<geometry::Port*> &from_ports,
      const std::set<geometry::Port*> &to_ports,
      const EquivalentNets &as_nets = {});

  // Solve for required routes:
  absl::Status Solve();

  // Inspect:
  std::string DescribeOrders() const;

 private:
  int32_t GetConcurrency() const;

  void ConfigureRoutingBlockageCache();

  EquivalentNets *GetRoutedNetsByPort(const geometry::Port *port) const;
  void MergeAndReplaceEquivalentNets(
      const std::set<EquivalentNets*> to_replace,
      EquivalentNets *replacement);

  absl::Status ConsolidateOrders();
  absl::Status CollectConnectedNets();

  absl::Status RunOrder(const NetRouteOrder &order);

  absl::Status RunAllSerial();
  absl::Status RunAllParallel();

  Layout *layout_;
  RoutingGrid *routing_grid_;
  RoutingBlockageCache root_blockage_cache_;

  // Tracks which ports belong to which routed nets. The EquivalentNets are
  // owned in routed_nets_.
  std::map<const geometry::Port*, EquivalentNets*> routed_nets_by_port_;

  // Tracks which nets have been routed.
  std::vector<std::unique_ptr<EquivalentNets>> routed_nets_;

  std::vector<NetRouteOrder> orders_;

  static constexpr size_t kNumRetries = 2;

  FRIEND_TEST(RouteManagerTest, ConsolidateOrders);
  FRIEND_TEST(RouteManagerTest, MergeAndReplaceEquivalentNets);
};

}  // namespace bfg

#endif  // ROUTE_MANAGER_H_
