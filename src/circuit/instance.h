#ifndef CIRCUIT_INSTANCE_H_
#define CIRCUIT_INSTANCE_H_

#include <initializer_list>
#include <memory>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

#include "vlsir/circuit.pb.h"

#include "../cell_reference.h"
#include "../parameter.h"
#include "connection.h"
#include "qualified_name.h"
#include "wire.h"

namespace bfg {

class Circuit;

namespace geometry {
class Instance;
}  // namespace geometry

namespace circuit {

class Instance {
 public:
  // TODO(aryap): Don't need this to be explicit since no other constructor is
  // available.
  Instance() = default;

  static Instance *FromVLSIRInstance(
      const Circuit &context,
      const vlsir::circuit::Instance &instance_pb);

  // Disconnects the port named "port_name". Returns true iff the port was
  // connected and is no longer.
  bool Disconnect(const std::string &port_name);

  // Connects the port named "port_name" to the given signal.
  void Connect(const std::string &port_name, const Slice &slice);

  // Accepts repeating pairs of (port_name, wire) to invoke Connect on.
  // TODO(aryap): Compare variadic arguments, variadic templates, and
  // std::initializer_list for this:
  // TODO(aryap): How to use temporaries without a copy in the std::pair?
  void Connect(std::initializer_list<
      std::pair<const std::string, const Wire&>> connect);

  void Connect(const std::string &port_name, const Signal &signal);

  std::optional<Connection> GetConnection(const std::string &port_name) const;

  void SetParameter(const std::string &name, const Parameter &value);

  ::vlsir::circuit::Instance ToVLSIRInstance() const;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  const CellReference &reference() const { return reference_; }
  void set_reference(const CellReference &reference) { reference_ = reference; }

  void set_module(Circuit *template_module) { module_ = template_module; }
  void set_module(const Circuit *template_module) {
    module_ = const_cast<Circuit*>(template_module);
  }
  Circuit *const module() const { return module_; }

  void set_geometry_instance(geometry::Instance *geometry_instance) {
    geometry_instance_ = geometry_instance;
  }
  geometry::Instance *geometry_instance() {
    return geometry_instance_;
  }

  const std::unordered_map<std::string, Parameter> &parameters() const {
    return parameters_; }
  const std::unordered_map<std::string, Connection> &connections() const {
    return connections_; }

 private:
  std::string name_;

  // A (possibly qualified) string reference to the Cell describing the Module
  // pointed to below. Used for bookeeping at import/export.
  CellReference reference_;

  // The template circuit object. Other databases call this "Master" or
  // "Module".
  Circuit *module_;

  // The corresponding geometry (layout) instance, if available.
  geometry::Instance *geometry_instance_;

  std::unordered_map<std::string, Parameter> parameters_;
  std::unordered_map<std::string, Connection> connections_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_INSTANCE_H_
