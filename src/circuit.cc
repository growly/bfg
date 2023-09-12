#include "circuit.h"

#include <sstream>
#include <string>
#include <absl/strings/str_cat.h>

#include "cell.h"
#include "circuit/instance.h"
#include "circuit/signal.h"
#include "circuit/wire.h"

#include "vlsir/circuit.pb.h"

namespace bfg {

Circuit *Circuit::FromVLSIRModule(const vlsir::circuit::Module &module_pb) {
  std::unique_ptr<Circuit> circuit(new Circuit());

  circuit->set_domain("");
  circuit->set_name(module_pb.name());

  for (const auto &signal_pb : module_pb.signals()) {
    circuit->AddSignal(signal_pb.name(), signal_pb.width());
  }
  for (const auto &port_pb : module_pb.ports()) {
    const circuit::Signal *signal = circuit->GetSignal(port_pb.signal());
    if (signal == nullptr) {
      LOG(WARNING) << "Port references unknown signal: \""
                   << port_pb.signal() << "\"";
      continue;
    }
    circuit->AddPort(
        *signal, circuit::Port::FromVLSIRPortDirection(port_pb.direction()));
  }
  for (const auto &instance_pb : module_pb.instances()) {
    // FIXME(growly): This is missing.
    LOG(WARNING) << "VLSIR circuit has instances which we ignore.";
  }
  for (const auto &param_pb : module_pb.parameters()) {
    Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
    circuit->parameters_.insert({parameter.name, parameter});
  }
  return circuit.release();
}

Circuit *Circuit::FromVLSIRExternalModule(
    const vlsir::circuit::ExternalModule &module_pb) {
  std::unique_ptr<Circuit> circuit(new Circuit());

  circuit->set_domain(module_pb.name().domain());
  circuit->set_name(module_pb.name().name());

  for (const auto &signal_pb : module_pb.signals()) {
    circuit->AddSignal(signal_pb.name(), signal_pb.width());
  }
  for (const auto &port_pb : module_pb.ports()) {
    const circuit::Signal *signal = circuit->GetSignal(port_pb.signal());
    if (signal == nullptr) {
      LOG(WARNING) << "Port references unknown signal: \""
                   << port_pb.signal() << "\"";
      continue;
    }
    circuit->AddPort(
        *signal, circuit::Port::FromVLSIRPortDirection(port_pb.direction()));
  }
  for (const auto &param_pb : module_pb.parameters()) {
    Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
    circuit->parameters_.insert({parameter.name, parameter});
  }
  return circuit.release();
}

const std::string &Circuit::NameOrParentName() const {
  if (name_ == "" && parent_cell_ != nullptr) {
    return parent_cell_->name();
  }
  return name_;
}

const std::string &Circuit::DomainOrParentDomain() const {
  if (domain_ == "" && parent_cell_ != nullptr) {
    return parent_cell_->domain();
  }
  return domain_;
}

namespace {

// TODO(aryap): Functions to map new circuit element names when adding, find
// equivalents in the circuit that have been added, etc, for recreating
// connectivity?
std::string MapSignalNameForAdd(
    const std::string &name_prefix, const std::string &name) {
  if (name_prefix == "") {
    return name;
  }
  return absl::StrCat(name_prefix, "__", name);
}

}   // namespace

void Circuit::AddCircuit(const Circuit &other, const std::string &prefix) {
  // Global signals do not have any prefixes added.
  for (const circuit::Signal *other_signal : other.global_signals_) {
    circuit::Signal *signal = GetOrAddSignal(
        other_signal->name(), other_signal->width());
    global_signals_.insert(signal);
  }

  for (const auto &other_signal : other.signals_) {
    if (other.global_signals_.find(
          other_signal.get()) != other.global_signals_.end()) {
      continue;
    }
    circuit::Signal *signal = AddSignal(
        MapSignalNameForAdd(prefix, other_signal->name()),
        other_signal->width());
  }

  for (const auto &other_port : other.ports_) {
    std::string signal_name = other.IsGlobal(other_port->signal()) ?
        other_port->signal().name() :
        MapSignalNameForAdd(prefix, other_port->signal().name());
    circuit::Signal *signal = GetSignal(signal_name);
    LOG_IF(FATAL, signal == nullptr)
      << "Should be able to find signal " << signal_name << "for other port; "
      << "signals should have been added already.";
     AddPort(*signal, other_port->direction()); 
  }

  for (const auto &other_instance : other.instances_) {
    std::string instance_name = MapSignalNameForAdd(
        prefix, other_instance->name());
    circuit::Instance *instance = AddInstance(
        instance_name, other_instance->module());
    instance->set_reference(other_instance->reference());

    for (const auto &entry : other_instance->parameters()) {
      instance->SetParameter(entry.first, entry.second);
    }

    for (const auto &entry : other_instance->connections()) {
      // We have to prefix the connected port name and the connected signal.
      const circuit::Connection &other_connection = entry.second;
      switch (other_connection.connection_type()) {
        case circuit::Connection::SIGNAL: {
          const circuit::Signal &other_signal = *other_connection.signal();
          std::string merged_name = other.IsGlobal(other_signal) ?
              other_signal.name() :
              MapSignalNameForAdd(prefix, other_signal.name());
          std::string port_name = entry.first;
          circuit::Signal *signal = GetSignal(merged_name);
          LOG_IF(FATAL, !signal) << "Signal " << merged_name << " not found.";
          instance->Connect(port_name, *signal);
          break;
        }
        case circuit::Connection::SLICE: {
          const circuit::Slice &other_slice = *other_connection.slice();
          const circuit::Signal &other_signal = other_slice.signal();
          std::string merged_name = other.IsGlobal(other_signal) ?
              other_signal.name() :
              MapSignalNameForAdd(prefix, other_signal.name());
          std::string port_name = entry.first;
          circuit::Signal *signal = GetSignal(merged_name);
          LOG_IF(FATAL, !signal) << "Signal " << merged_name << " not found.";
          circuit::Slice slice(*signal,
                               other_slice.low_index(),
                               other_slice.high_index());
          instance->Connect(port_name, slice);
          break;
        }
        case circuit::Connection::CONCATENATION:
        default:
          LOG(FATAL) << "Unknown Connection type: "
                     << other_connection.connection_type();
      }
    }
  }
}

void Circuit::AddGlobal(const circuit::Wire &wire) {
  global_signals_.insert(&wire.signal());
}

void Circuit::AddGlobal(circuit::Signal *signal) {
  global_signals_.insert(signal);
}

bool Circuit::IsGlobal(const circuit::Signal &signal) const {
  circuit::Signal *ptr = const_cast<circuit::Signal*>(&signal);
  return global_signals_.find(ptr) != global_signals_.end();
}

bool Circuit::IsPowerOrGround(const circuit::Signal &signal) const {
  circuit::Signal *ptr = const_cast<circuit::Signal*>(&signal);
  if (power_signals_.find(ptr) != power_signals_.end())
    return true;
  if (ground_signals_.find(ptr) != ground_signals_.end())
    return true;
  return false;
}

circuit::Wire Circuit::AddSignal(const std::string &name) {
  circuit::Signal *signal = AddSignal(name, 1);
  return bfg::circuit::Wire(*signal, 0);
}

circuit::Signal *Circuit::GetOrAddSignal(
  const std::string &name, uint64_t width) {
  auto it = signals_by_name_.find(name);
  if (it == signals_by_name_.end()) {
    return AddSignal(name, width);
  }
  return it->second;
}

void Circuit::SetParameter(const std::string &name, const Parameter &value) {
  parameters_[name] = value;
}

circuit::Signal *Circuit::GetSignal(const std::string &name) {
  auto it = signals_by_name_.find(name);
  if (it == signals_by_name_.end()) {
    return nullptr;
  }
  return it->second;
}

circuit::Signal *Circuit::AddSignal(const std::string &name, uint64_t width) {
  LOG_IF(FATAL, signals_by_name_.find(name) != signals_by_name_.end())
      << "Duplicate signal name: " << name;
  circuit::Signal *signal = new circuit::Signal(name, width);
  signals_.emplace_back(signal);
  signals_by_name_.insert({name, signal});
  return signal;
}

circuit::Instance *Circuit::AddInstance(
    const std::string &name, const Circuit *template_module) {
  LOG_IF(FATAL, instances_by_name_.find(name) != instances_by_name_.end())
      << "Duplicate instance name: " << name;
  circuit::Instance *instance = new circuit::Instance();
  instance->set_name(name);
  instance->set_module(template_module);
  instances_.emplace_back(instance);
  instances_by_name_.insert({name, instance});
  return instance;
}

circuit::Port *Circuit::AddPort(
    const circuit::Wire &wire,
    const circuit::Port::PortDirection &direction) {
  return AddPort(wire.signal(), direction);
}

circuit::Port *Circuit::AddPort(
    const circuit::Signal &signal,
    const circuit::Port::PortDirection &direction) {
  // Ports map 1-1 to signals?
  circuit::Port *port = new circuit::Port(signal, direction);
  ports_.emplace_back(port);
  return port;
}

std::string Circuit::Describe() const {
  std::stringstream ss;

  ss << "circuit " << domain_ << "/" << name_ << ": "
                   << signals_.size() << " signals, "
                   << ports_.size() << " ports, "
                   << instances_.size() << " instances" << std::endl;

  return ss.str();
};

::vlsir::circuit::Module Circuit::ToVLSIRModule() const {
  ::vlsir::circuit::Module mod_pb;
  mod_pb.set_name(NameOrParentName());

  for (const auto &signal : signals_) {
    *mod_pb.add_signals() = signal->ToVLSIRSignal();
  }

  for (const auto &port : ports_) {
    *mod_pb.add_ports() = port->ToVLSIRPort();
  }

  for (const auto &instance : instances_) {
    *mod_pb.add_instances() = instance->ToVLSIRInstance();
  }

  // TODO(aryap): Add parameters.

  return mod_pb;
}

::vlsir::circuit::ExternalModule Circuit::ToVLSIRExternalModule() const {
  ::vlsir::circuit::ExternalModule mod_pb;

  mod_pb.mutable_name()->set_domain(DomainOrParentDomain());
  mod_pb.mutable_name()->set_name(NameOrParentName());
  mod_pb.set_desc(parent_cell_ ? parent_cell_->description() : description_);

  for (const auto &signal : signals_) {
    *mod_pb.add_signals() = signal->ToVLSIRSignal();
  }

  for (const auto &port : ports_) {
    *mod_pb.add_ports() = port->ToVLSIRPort();
  }

  // TODO(aryap): Add parameters.
  // TODO(aryap): Add spicetype.

  return mod_pb;
}

}  // namespace bfg
