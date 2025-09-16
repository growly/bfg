#ifndef ROUTE_MANAGER_H_
#define ROUTE_MANAGER_H_

#include <memory>
#include <optional>
#include <vector>

#include <absl/status/statusor.h>

#include "equivalent_nets.h"
#include "geometry/port.h"
#include "geometry/shape_collection.h"
#include "routing_blockage_cache.h"
#include "routing_grid.h"
#include "routing_path.h"

namespace bfg {


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
class RouteManager {
 public:

 private:
  RoutingGrid* routing_grid_;  // Not owned

  std::unique_ptr<RoutingBlockageCache> blockage_cache_;
};

}  // namespace bfg

#endif  // ROUTE_MANAGER_H_
