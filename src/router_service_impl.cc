#include "router_service_impl.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>
#include <iomanip>
#include <memory>

#include <google/protobuf/text_format.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "dev_pdk_setup.h"

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_grid.h"
#include "routing_layer_info.h"
#include "physical_properties_database.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

namespace bfg {

grpc::Status RouterServiceImpl::CreateRoutingGrid(
    grpc::ServerContext *context,
    const CreateRoutingGridRequest *request,
    CreateRoutingGridReply *reply) {
  PhysicalPropertiesDatabase physical_db;

  LOG(INFO) << "CreateRoutingGrid request";
  std::string tech_proto_source;
  switch (request->predefined_technology()) {
    case router_service::TECHNOLOGY_SKY130:
      LOG(INFO) << "Loading sky130";
      physical_db.LoadTechnologyFromFile("../sky130.technology.pb");
      SetUpSky130(&physical_db);
      break;
    case router_service::TECHNOLOGY_GF180MCU:
      LOG(INFO) << "Loading gf180mcu";
      physical_db.LoadTechnologyFromFile("../gf180mcu.technology.pb");
      SetUpGf180Mcu(&physical_db);
      break;
    case router_service::TECHNOLOGY_OTHER:
      // Fallthrough intended.
    default:
      // Do nothing.
      break;
  }

  RouterSession *session = new RouterSession(physical_db);

  // Define grid with router and ConnectLayers().
  router_service::Status set_up =
      session->SetUpRoutingGrid(request->grid_definition());

  reply->mutable_status()->CopyFrom(set_up);
  if (set_up.code() != router_service::StatusCode::OK) {
    delete session;
    return grpc::Status::OK;
  }

  RouterServiceImpl::UUID next_uuid = NextUUID();
  auto insertion_it = sessions_.insert({
      next_uuid,
      std::move(std::unique_ptr<RouterSession>(session))
  });
  reply->set_grid_id(next_uuid);

  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::AddRoutes(
    grpc::ServerContext *context,
    const AddRoutesRequest *request,
    AddRoutesReply *reply) {
  RouterSession *session = GetSession(request->grid_id());
  if (!session) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }
  if (!session->AddRoutes(*request)) {
    // Some error.
    reply->mutable_status()->set_code(
        router_service::StatusCode::OTHER_ERROR);
    return grpc::Status::OK;
  }
  session->ExportRoutes(reply);

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::QueryRoutingGrid(
    grpc::ServerContext *context,
    const QueryRoutingGridRequest *request,
    QueryRoutingGridReply *reply) {
  RouterSession *session = GetSession(request->grid_id());
  if (!session) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::DeleteRoutingGrid(
    grpc::ServerContext *context,
    const DeleteRoutingGridRequest *request,
    DeleteRoutingGridReply *reply) {
  size_t num_removed = sessions_.erase(request->grid_id());
  if (num_removed < 1) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }

  sessions_.erase(request->grid_id());

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

// Other.
RouterSession *RouterServiceImpl::GetSession(
    const RouterServiceImpl::UUID &uuid) {
  auto it = sessions_.find(uuid);
  if (it == sessions_.end()) {
    return nullptr;
  }
  return it->second.get();
}

RoutingGrid *RouterServiceImpl::GetGrid(
    const RouterServiceImpl::UUID &uuid) {
  RouterSession *session = GetSession(uuid);
  if (!session) {
    return nullptr;
  }
  return session->routing_grid();
}

const RouterServiceImpl::UUID RouterServiceImpl::NextUUID() const { 
  RouterServiceImpl::UUID next = highest_index_ + 1;
  while (sessions_.find(next) != sessions_.end()) {
    next++;
  }
  return next;
}

}  // namespace bfg
