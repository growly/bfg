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

namespace bfg {

// A "Circuit" is the same thing as a "Module" in the VLSIR schema. It collects
// other circuit primitives into a template definition of one circuit which can
// be instantiated elsehwere.
class Circuit {
 public:
  Circuit() = default;

  circuit::Signal *AddSignal(const std::string &name) {
    auto name_it = signals_by_name_.find(name);
    LOG_IF(FATAL, name_it != signals_by_name_.end())
        << "Duplicate signal name: " << name;
    signals_.emplace_back(name);
    circuit::Signal *signal = &signals_.back();
    signals_by_name_.insert({name, signal});
    return signal;
  }

  std::vector<circuit::Signal> &signals() { return signals_; }

  std::string Describe() const;

 private:
  std::string description_;

  std::vector<circuit::Port> ports_;
  std::vector<circuit::Signal> signals_;
  std::vector<circuit::Instance> instances_;

  // Pointers to signals that should be treated as power and ground,
  // respectively.
  std::vector<circuit::Signal*> power_signals_;
  std::vector<circuit::Signal*> ground_signals_;

  std::unordered_map<std::string, circuit::Signal*> signals_by_name_;

  std::unordered_map<std::string, Parameter> parameters_;
};


}  // namespace bfg

#endif  // CIRCUIT_H_
