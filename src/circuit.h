#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <string>
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

  std::string Describe() const;

 private:
  std::string description_;

  std::vector<circuit::Port> ports_;
  std::vector<circuit::Signal> signals_;
  std::vector<circuit::Instance> instances_;

  std::unordered_map<std::string, Parameter> parameters_;
};


}  // namespace bfg

#endif  // CIRCUIT_H_
