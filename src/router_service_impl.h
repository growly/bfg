#ifndef ROUTER_SERVICE_IMPL_H_
#define ROUTER_SERVICE_IMPL_H_

#include <memory>

#include <google/protobuf/text_format.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "router_session.h"
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

// Manages RoutingGrids.
class RouterServiceImpl final :
  public bfg::router_service::RouterService::Service {
 public:
  RouterServiceImpl()
      : highest_index_(0) {}

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

  UUID highest_index_;

  std::map<UUID, std::unique_ptr<RouterSession>> sessions_;
};

}  // namespace bfg

#endif  // ROUTER_SERVICE_IMPL_H_
