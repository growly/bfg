#include "instance.h"

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

}  // namespace circuit
}  // namespace bfg
