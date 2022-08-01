#include "instance.h"

#include "../circuit.h"

#include "vlsir/circuit.pb.h"

#include <unordered_map>
#include <glog/logging.h>

#include "connection.h"

namespace bfg {
namespace circuit {

bool Instance::Disconnect(const std::string &port_name) {
  return connections_.erase(port_name) == 1;
}

void Instance::Connect(
    const std::string &port_name, const Wire &wire) {
  LOG_IF(FATAL, Disconnect(port_name))
      << "Instance \"" << name_ << "\" port \"" << port_name
      << "\" was already connected when trying to Connect.";
  Connection connection;
  connection.set_slice(wire);
  connections_.insert({port_name, connection});
}

void Instance::Connect(
    std::initializer_list<std::pair<const std::string, const Wire&>> connect) {
  for (const auto &entry : connect) {
    Connect(entry.first, entry.second);
  }
}

::vlsir::circuit::Instance Instance::ToVLSIRInstance() const {
  ::vlsir::circuit::Instance instance_pb;
  instance_pb.set_name(name_);
  
  // TODO(aryap): This is where externally-referenced modules are recorded.
  if (module_) {
    instance_pb.mutable_module()->set_local(module_->name());
  }

  for (const auto &entry : connections_) {
    ::vlsir::circuit::Connection *connection_pb =
        instance_pb.add_connections();
    connection_pb->set_portname(entry.first);
    *connection_pb->mutable_target() = entry.second.ToVLSIRConnection();
  }

  LOG_IF(FATAL, parameters_.size() > 0)
      << "Unimplemented: serialising parameters";

  return instance_pb;
}

}  // namespace circuit
}  // namespace bfg
