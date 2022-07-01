#ifndef CIRCUIT_INSTANCE_H_
#define CIRCUIT_INSTANCE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../parameter.h"
#include "connection.h"
#include "qualified_name.h"
#include "wire.h"

namespace bfg {

class Circuit;

namespace circuit {

class Instance {
 public:
  Instance() = default;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  void set_module(Circuit *template_module) { module_ = template_module; }
  Circuit *const module() const { return module_; }

  // Disconnects the port named "port_name". Returns true iff the port was
  // connected and is no longer.
  bool Disconnect(const std::string &port_name);

  // Connects the port named "port_name" to the given signal.
  void Connect(const std::string &port_name, const Wire &wire);

 private:
  std::string name_;

  // The template circuit object. Other databases call this "Master" or
  // "Module".
  Circuit *module_;

  std::unordered_map<std::string, Parameter> parameters_;
  std::unordered_map<std::string, Connection> connections_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_INSTANCE_H_
