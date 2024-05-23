#include <gflags/gflags.h>
#include <glog/logging.h>
#include <absl/strings/str_join.h>

#include <absl/strings/str_format.h>

#include <google/protobuf/text_format.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "design_database.h"
#include "physical_properties_database.h"

#include "c_make_header.h"

#include "services/router_service.grpc.pb.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_int32(port, 8222, "Listen port for the service");

class RouterServiceImpl final :
  public bfg::router_service::RouterService::Service {
  grpc::Status SayHello(grpc::ServerContext *context,
                        const bfg::router_service::SayHelloRequest *request,
                        bfg::router_service::SayHelloReply *reply) override {
    reply->set_message("lol");
    return grpc::Status::OK;
  }
};

void RunServer() {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", FLAGS_port);
  RouterServiceImpl service;

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
