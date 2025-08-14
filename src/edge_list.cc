#include "edge_list.h"

#include <fstream>
#include <string>
#include <vector>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>
#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "proto/edge_list.pb.h"

namespace bfg {

std::string EdgeSpec::Endpoint::Describe() const {
  return absl::StrCat(
      instance_name, " {", absl::StrJoin(port_names, ", "), "}");
}

void EdgeSpec::FromProto(const proto::EdgeSpec &edge_spec_pb) {
  from_.instance_name = edge_spec_pb.from().instance_name();
  from_.port_names = std::vector<std::string>(
      edge_spec_pb.from().port_names().begin(),
      edge_spec_pb.from().port_names().end());
  to_.instance_name = edge_spec_pb.to().instance_name();
  to_.port_names = std::vector<std::string>(
      edge_spec_pb.to().port_names().begin(),
      edge_spec_pb.to().port_names().end());
}

proto::EdgeSpec EdgeSpec::ToProto() const {
  proto::EdgeSpec pb;
  pb.mutable_from()->set_instance_name(from_.instance_name);
  for (const std::string &port_name : from_.port_names) {
    pb.mutable_from()->add_port_names(port_name);
  }
  pb.mutable_to()->set_instance_name(to_.instance_name);
  for (const std::string &port_name : to_.port_names) {
    pb.mutable_to()->add_port_names(port_name);
  }
  return pb;
}

std::string EdgeSpec::Describe() const {
  return absl::StrCat(from_.Describe(), " -> ", to_.Describe());
}

void EdgeSpec::set_from(
    const std::string &instance_name,
    const std::vector<std::string> &port_names) {
  from_.instance_name = instance_name;
  from_.port_names = std::vector<std::string>(
      port_names.begin(), port_names.end());
}

void EdgeSpec::set_to(
    const std::string &instance_name,
    const std::vector<std::string> &port_names) {
  to_.instance_name = instance_name;
  to_.port_names = std::vector<std::string>(
      port_names.begin(), port_names.end());
}

void EdgeList::FromProto(const proto::EdgeList &pb) {
  edges_.clear();
  for (const proto::EdgeSpec &edge_pb : pb.edges()) {
    EdgeSpec edge;
    edge.FromProto(edge_pb);
    edges_.push_back(edge);
  }
}

proto::EdgeList EdgeList::ToProto() const {
  proto::EdgeList pb;
  for (const EdgeSpec &spec : edges_) {
    *pb.add_edges() = spec.ToProto();
  }
  return pb;
}

void EdgeList::FromCSVOrDie(const std::string &path) {
  std::ifstream file(path);
  LOG_IF(FATAL, !file.is_open())
      << "Could not open CSV file " << path;
  std::string line;
  size_t i = 0;
  while (std::getline(file, line)) {
    std::vector<std::string> fields = absl::StrSplit(line, ",");

    if (fields.size() != 4) {
      LOG(WARNING) << "Skipping malformed CSV line " << i << ": "
                   << line;
    }

    std::vector<std::string> from_ports = absl::StrSplit(fields[1], "/");
    std::vector<std::string> to_ports = absl::StrSplit(fields[3], "/");

    AddEdge(fields[0], from_ports, fields[2], to_ports);

    ++i;
  }
}

void EdgeList::AddEdge(
    const std::string &from,
    const std::vector<std::string> &from_ports,
    const std::string &to,
    const std::vector<std::string> &to_ports) {
  EdgeSpec edge;
  edge.set_from(from, from_ports);
  edge.set_to(to, to_ports);
  edges_.push_back(edge);
}

}  // namespace bfg
