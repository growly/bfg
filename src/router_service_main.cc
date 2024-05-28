#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>

#include <absl/strings/str_format.h>

#include "c_make_header.h"

#include "router_service_impl.h"

DEFINE_string(technology, "technology.pb", "Path to binary technology proto");
DEFINE_int32(port, 8222, "Listen port for the service");

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
