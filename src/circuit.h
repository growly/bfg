#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <glog/logging.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "parameter.h"
#include "circuit/instance.h"
#include "circuit/signal.h"
#include "circuit/port.h"
#include "circuit/wire.h"

#include "vlsir/circuit.pb.h"

namespace bfg {

// A "Circuit" is the same thing as a "Module" in the VLSIR schema. It collects
// other circuit primitives into a template definition of one circuit which can
// be instantiated elsehwere.
class Circuit {
 public:
  enum Type {
    INTERNAL,
    EXTERNAL
  };

  // Makes a new Circuit from a VLSIR Module message. Caller takes ownership.
  static Circuit *FromVLSIRModule(const vlsir::circuit::Module &module_pb);

  Circuit() = default;

  // Convenient: Add a width=1 signal and return a wire indexing it. Wires are
  // designed to be ephemeral.
  // TODO(aryap): How to avoid a copy here? Copy elision works?
  circuit::Wire AddSignal(const std::string &name);

  circuit::Signal *AddSignal(const std::string &name, uint64_t width);
  circuit::Instance *AddInstance(
    const std::string &name, const Circuit *template_module);
  circuit::Port *AddPort(
      const circuit::Signal &signal,
      const circuit::Port::PortDirection &direction = circuit::Port::NONE);
  circuit::Port *AddPort(
      const circuit::Wire &wire,
      const circuit::Port::PortDirection &direction = circuit::Port::NONE);

  const circuit::Signal *GetSignal(const std::string &name) const {
    auto signals_it = signals_by_name_.find(name);
    if (signals_it == signals_by_name_.end()) {
      return nullptr;
    }
    return signals_it->second;
  }

  std::vector<std::unique_ptr<circuit::Signal>> &signals() { return signals_; }

  std::string Describe() const;

  ::vlsir::circuit::Module ToVLSIRCircuit() const;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }
 private:
  Type type_;

  std::string name_;
  std::string description_;

  // We own these objects but we don't want their address to change when the
  // vectors resize (right?).
  std::vector<std::unique_ptr<circuit::Signal>> signals_;
  std::vector<std::unique_ptr<circuit::Port>> ports_;
  std::vector<std::unique_ptr<circuit::Instance>> instances_;

  // Pointers to signals that should be treated as power and ground,
  // respectively.
  std::vector<circuit::Signal*> power_signals_;
  std::vector<circuit::Signal*> ground_signals_;

  std::unordered_map<std::string, circuit::Signal*> signals_by_name_;
  std::unordered_map<std::string, circuit::Instance*> instances_by_name_;

  std::unordered_map<std::string, Parameter> parameters_;
};


}  // namespace bfg

#endif  // CIRCUIT_H_
