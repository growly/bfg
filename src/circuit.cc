#include "circuit.h"

#include "circuit/instance.h"
#include "circuit/signal.h"
#include "circuit/wire.h"

#include <sstream>

namespace bfg {

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

std::string Circuit::Describe() const {
  std::stringstream ss;

  ss << "circuit" << std::endl;

  return ss.str();
};

}  // namespace bfg
