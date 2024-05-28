#include <memory>

#include <google/protobuf/text_format.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "routing_grid.h"
#include "physical_properties_database.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

namespace bfg {

using router_service::CreateRoutingGridRequest;
using router_service::CreateRoutingGridReply;
using router_service::AddRoutesRequest;
using router_service::AddRoutesReply;
using router_service::QueryRoutingGridRequest;
using router_service::QueryRoutingGridReply;
using router_service::DeleteRoutingGridRequest;
using router_service::DeleteRoutingGridReply;

class RouterSession {
 public:
  RouterSession(RoutingGrid *routing_grid)
      : routing_grid_(routing_grid) {}

  RoutingGrid *routing_grid() { return routing_grid_.get(); }

  bool AddRoutes(const AddRoutesRequest &request) {
    // TODO(aryap):
    // We will have a list of nets to route with 2+ points:
    //  - Connect first two points with shortest path AddRouteBetween(...),
    //  give them the net label.
    //  - Connect successive points to the existing net.
    //  - Pray.
    return true;
  }

 private:
  std::unique_ptr<RoutingGrid> routing_grid_;
};

// Manages RoutingGrids.
class RouterServiceImpl final :
  public bfg::router_service::RouterService::Service {
 public:
  // TODO(aryap): Can we find a portable UUID library?
  typedef int64_t UUID;

  // gRPC handlers.
  grpc::Status CreateRoutingGrid(
      grpc::ServerContext *context,
      const CreateRoutingGridRequest *request,
      CreateRoutingGridReply *reply) override;

  grpc::Status AddRoutes(
      grpc::ServerContext *context,
      const AddRoutesRequest *request,
      AddRoutesReply *reply) override;

  grpc::Status QueryRoutingGrid(
      grpc::ServerContext *context,
      const QueryRoutingGridRequest *request,
      QueryRoutingGridReply *reply) override;

  grpc::Status DeleteRoutingGrid(
      grpc::ServerContext *context,
      const DeleteRoutingGridRequest *request,
      DeleteRoutingGridReply *reply) override;

  // Other.
  RouterSession *GetSession(const UUID &uuid);
  RoutingGrid *GetGrid(const UUID &uuid);

 private:
  const UUID NextUUID() const;

  router_service::Status SetUpRoutingGrid(
      const router_service::RoutingGridDefinition &grid_definition,
      RoutingGrid *grid);

  UUID highest_index_;

  std::map<UUID, std::unique_ptr<RouterSession>> sessions_;
};

}  // namespace bfg
