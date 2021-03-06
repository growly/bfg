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

namespace bfg {

// A "Circuit" is the same thing as a "Module" in the VLSIR schema. It collects
// other circuit primitives into a template definition of one circuit which can
// be instantiated elsehwere.
class Circuit {
 public:
  Circuit() = default;

  // Convenient: Add a width=1 signal and return a wire indexing it. Wires are
  // designed to be ephemeral.
  // TODO(aryap): How to avoid a copy here? Copy elision works?
  circuit::Wire AddSignal(const std::string &name);

  circuit::Signal *AddSignal(const std::string &name, uint64_t width);
  circuit::Instance *AddInstance(
    const std::string &name, const Circuit *template_module);

  std::vector<std::unique_ptr<circuit::Signal>> &signals() { return signals_; }

  std::string Describe() const;

 private:
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
