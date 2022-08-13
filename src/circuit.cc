#include "circuit.h"

#include "circuit/instance.h"
#include "circuit/signal.h"
#include "circuit/wire.h"

#include "vlsir/circuit.pb.h"

#include <sstream>

namespace bfg {

Circuit *Circuit::FromVLSIRModule(const vlsir::circuit::Module &module_pb) {
  std::unique_ptr<Circuit> circuit(new Circuit());

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
  }
  for (const auto &param_pb : module_pb.parameters()) {
    Parameter parameter = Parameter::FromVLSIRParameter(param_pb);
    circuit->parameters_.insert({parameter.name, parameter});
  }
  return circuit.release();
}

circuit::Wire Circuit::AddSignal(const std::string &name) {
  circuit::Signal *signal = AddSignal(name, 1);
  return bfg::circuit::Wire(*signal, 0);
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

  ss << "circuit: " << signals_.size() << " signals, "
                    << ports_.size() << " ports, "
                    << instances_.size() << " instances" << std::endl;

  return ss.str();
};

::vlsir::circuit::Module Circuit::ToVLSIRCircuit() const {
  ::vlsir::circuit::Module mod_pb;
  mod_pb.set_name(name_);

  for (const auto &signal : signals_) {
    *mod_pb.add_signals() = signal->ToVLSIRSignal();
  }

  for (const auto &port : ports_) {
    *mod_pb.add_ports() = port->ToVLSIRPort();
  }

  for (const auto &instance : instances_) {
    *mod_pb.add_instances() = instance->ToVLSIRInstance();
  }

  return mod_pb;
}

}  // namespace bfg
