#ifndef ROUTER_SESSION_H_
#define ROUTER_SESSION_H_

#include <memory>

#include "routing_grid.h"
#include "physical_properties_database.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

namespace bfg {

class RouterSession {
 public:
  RouterSession(RoutingGrid *routing_grid)
      : routing_grid_(routing_grid) {}

  RoutingGrid *routing_grid() { return routing_grid_.get(); }

  bool AddRoutes(const router_service::AddRoutesRequest &request);

  router_service::Status SetUpRoutingGrid(
      const router_service::RoutingGridDefinition &grid_definition);

 private:
  std::unique_ptr<RoutingGrid> routing_grid_;
};

}  // namespace bfg

#endif  // ROUTER_SESSION_H_
