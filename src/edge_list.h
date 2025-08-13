#ifndef EDGE_LIST_H_
#define EDGE_LIST_H_

#include <string>
#include <vector>

#include "proto/edge_list.pb.h"

namespace bfg {

// An EdgeList captures intended connectivity within a RoutingGraph (or more
// generally). Each edge is described by an EdgeSpec, which conatins a  pair of
// endpoints that at the very least nominate the name of the instance from
// which the connection emerge (first entry), or on which it terminates (second
// entry).
//
// Each endpoint also includes a list of equally-weighted ports, by name,
// connecting to any of which will satisfy the connection.
class EdgeSpec {
 public:
  struct Endpoint {
    std::string instance_name;
    std::vector<std::string> port_names;
  };

  void FromProto(const proto::EdgeSpec &edge_spec_pb);
  proto::EdgeSpec ToProto() const;

  EdgeSpec() {}

  void set_from(
    const std::string &instance_name,
    const std::vector<std::string> &port_names);

  void set_to(
    const std::string &instance_name,
    const std::vector<std::string> &port_names);

  const Endpoint &from() const { return from_; }
  const Endpoint &to() const { return to_; }

 private:
  Endpoint from_;
  Endpoint to_;
};

class EdgeList {
 public:
  EdgeList() {}

  void FromProto(const proto::EdgeList &pb);
  proto::EdgeList ToProto() const;

  void FromCSV(const std::string &path);

  void AddEdge(
      const std::string &from,
      const std::vector<std::string> &from_ports,
      const std::string &to,
      const std::vector<std::string> &to_ports);

  const std::vector<EdgeSpec> &edges() const { return edges_; }
  std::vector<EdgeSpec> &edges() { return edges_; }

 private:
  std::vector<EdgeSpec> edges_;
};

}  // namespace bfg

#endif  // EDGE_LIST_H_
