#include "instance.h"

#include "../circuit.h"

#include "vlsir/circuit.pb.h"

#include <unordered_map>
#include <glog/logging.h>

#include "connection.h"

namespace bfg {
namespace circuit {

Instance *Instance::FromVLSIRInstance(
    const Circuit &context,
    const vlsir::circuit::Instance &instance_pb) {
  std::unique_ptr<Instance> instance(new Instance());
  instance->set_name(instance_pb.name());
  instance->set_reference(
      CellReference::FromVLSIRReference(instance_pb.module()));
  for (const auto &param_pb : instance_pb.parameters()) {
    Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
    instance->parameters_.insert({parameter.name, parameter});
  }
  for (const auto &connection_pb : instance_pb.connections()) {
    //Connection connection = 
  }
  return instance.release();
}

bool Instance::Disconnect(const std::string &port_name) {
  return connections_.erase(port_name) == 1;
}

void Instance::Connect(
    const std::string &port_name, const Signal &signal) {
  LOG_IF(FATAL, Disconnect(port_name))
      << "Instance \"" << name_ << "\" port \"" << port_name
      << "\" was already connected when trying to Connect.";
  Connection connection;
  connection.set_signal(&signal);
  connections_.insert({port_name, connection});
}

void Instance::Connect(
    const std::string &port_name, const Slice &slice) {
  LOG_IF(FATAL, Disconnect(port_name))
      << "Instance \"" << name_ << "\" port \"" << port_name
      << "\" was already connected when trying to Connect.";

  Connection connection;

  // Special case: if the slice references the entire signal, just connect to
  // the signal instead and discard the Slice adapter.
  if (slice.Width() == slice.signal().width()) {
    connection.set_signal(&slice.signal());
  } else {
    connection.set_slice(slice);
  }
  connections_.insert({port_name, connection});
}

void Instance::Connect(
    std::initializer_list<std::pair<const std::string, const Wire&>> connect) {
  for (const auto &entry : connect) {
    Connect(entry.first, entry.second);
  }
}

void Instance::SetParameter(const std::string &name, const Parameter &value) {
  parameters_[name] = value;
}

::vlsir::circuit::Instance Instance::ToVLSIRInstance() const {
  ::vlsir::circuit::Instance instance_pb;
  instance_pb.set_name(name_);
  
  // TODO(aryap): This is where externally-referenced modules are recorded.
  if (module_) {
    const std::string &name = module_->NameOrParentName();
    if (module_->domain() != "") {
      instance_pb.mutable_module()->mutable_external()->set_domain(
          module_->DomainOrParentDomain());
      instance_pb.mutable_module()->mutable_external()->set_name(name);
    } else {
      instance_pb.mutable_module()->set_local(name);
    }
  }

  for (const auto &entry : connections_) {
    ::vlsir::circuit::Connection *connection_pb =
        instance_pb.add_connections();
    connection_pb->set_portname(entry.first);
    *connection_pb->mutable_target() = entry.second.ToVLSIRConnection();
  }


  for (const auto &entry : parameters_) {
    *instance_pb.add_parameters() = entry.second.ToVLSIRParameter();
  }

  return instance_pb;
}

}  // namespace circuit
}  // namespace bfg
