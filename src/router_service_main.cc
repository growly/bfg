#include <gflags/gflags.h>
#include <glog/logging.h>
#include <absl/strings/str_join.h>

#include <memory>
#include <fstream>

#include <absl/strings/str_format.h>

#include <google/protobuf/text_format.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "design_database.h"
#include "routing_grid.h"
#include "physical_properties_database.h"

#include "c_make_header.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_int32(port, 8222, "Listen port for the service");

namespace bfg {

using router_service::CreateRoutingGridRequest;
using router_service::CreateRoutingGridReply;
using router_service::AddRoutesRequest;
using router_service::AddRoutesReply;
using router_service::QueryRoutingGridRequest;
using router_service::QueryRoutingGridReply;

class RouterSession {
 public:
  RouterSession(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

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
  PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<RoutingGrid> routing_grid_;
};

// Manages RoutingGrids.
class RouterServiceImpl final :
  public bfg::router_service::RouterService::Service {
 public:
  // TODO(aryap): Use a real UUID.
  typedef int64_t UUID;

  // gRPC handlers.
  grpc::Status CreateRoutingGrid(
      grpc::ServerContext *context,
      const CreateRoutingGridRequest *request,
      CreateRoutingGridReply *reply) override {
    LOG(INFO) << "CreateRoutingGrid request";
    std::string tech_proto_source;
    switch (request->predefined_technology()) {
      case router_service::SKY130:
        LOG(INFO) << "Loading sky130";
        tech_proto_source = "../sky130.technology.pb";
        break;
      case router_service::GF180MCU:
        // Fallthrough intended.
      case router_service::NONE:
        // Fallthrough intended.
      default:
        // Do nothing.
        break;
    }

    // Get PhysicalPropertiesDatabase.
    PhysicalPropertiesDatabase physical_db;
    vlsir::tech::Technology tech_pb;
    std::fstream technology_input(
        tech_proto_source, std::ios::in | std::ios::binary);
    LOG_IF(FATAL, !technology_input)
        << "Could not open technology protobuf, "
        << tech_proto_source;
    if (!tech_pb.ParseFromIstream(&technology_input)) {
      LOG(FATAL) << "Could not parse technology protobuf, "
                 << tech_proto_source;
    }
    physical_db.LoadTechnology(tech_pb);

    UUID next_uuid = NextUUID();
    sessions_.insert({
        next_uuid,
        // new RouterSession(physical_db)
        std::make_unique<RouterSession>(physical_db)
    });
    reply->set_grid_id(next_uuid);

    // Define grid with router and ConnectLayers().

    reply->mutable_status()->set_code(router_service::StatusCode::OK);
    return grpc::Status::OK;
  }

  grpc::Status AddRoutes(
      grpc::ServerContext *context,
      const AddRoutesRequest *request,
      AddRoutesReply *reply) override {
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

    reply->mutable_status()->set_code(router_service::StatusCode::OK);
    return grpc::Status::OK;
  }

  grpc::Status QueryRoutingGrid(
      grpc::ServerContext *context,
      const QueryRoutingGridRequest *request,
      QueryRoutingGridReply *reply) override {
    RouterSession *session = GetSession(request->grid_id());
    if (!session) {
      reply->mutable_status()->set_code(
          router_service::StatusCode::GRID_NOT_FOUND);
      return grpc::Status::OK;
    }

    reply->mutable_status()->set_code(router_service::StatusCode::OK);
    return grpc::Status::OK;
  }

  // Other.
  RouterSession *GetSession(const UUID &uuid) {
    auto it = sessions_.find(uuid);
    if (it == sessions_.end()) {
      return nullptr;
    }
    return it->second.get();
  }
  RoutingGrid *GetGrid(const UUID &uuid) {
    RouterSession *session = GetSession(uuid);
    if (!session) {
      return nullptr;
    }
    return session->routing_grid();
  }

  const UUID NextUUID() const { 
    UUID next = highest_index_ + 1;
    while (sessions_.find(next) != sessions_.end()) {
      next++;
    }
    return next;
  }

 private:
  UUID highest_index_;

  std::map<UUID, std::unique_ptr<RouterSession>> sessions_;
};

}  // namespace bfg

void RunServer() {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", FLAGS_port);
  bfg::RouterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::string version =
      "BFG Router Service v" xstr(bfg_VERSION_MAJOR) "." xstr(bfg_VERSION_MINOR);
  std::cout << version << std::endl;
  LOG(INFO) << version << " start";

  RunServer();
}
