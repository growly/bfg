#ifndef ROUTER_SESSION_H_
#define ROUTER_SESSION_H_

#include <memory>
#include <optional>

#include "geometry/port.h"

#include "routing_grid.h"
#include "physical_properties_database.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

namespace bfg {

class RouterSession {
 public:
  RouterSession(PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {
    routing_grid_.reset(new RoutingGrid(physical_db_));
  }

  RoutingGrid *routing_grid() { return routing_grid_.get(); }

  bool AddRoutes(const router_service::AddRoutesRequest &request);

  void ExportRoutes(router_service::AddRoutesReply *reply) const;

  bool PerformNetRouteOrder(const router_service::NetRouteOrder &request);

  router_service::Status SetUpRoutingGrid(
      const router_service::RoutingGridDefinition &grid_definition);

 private:
  std::optional<geometry::Port> PointAndLayerToPort(
      const std::string &net,
      const router_service::PointOnLayer &point_on_layer) const;

  PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<RoutingGrid> routing_grid_;
};

}  // namespace bfg

#endif  // ROUTER_SESSION_H_