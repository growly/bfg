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

class Cell;

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
  // Power and ground nets are copies verbatim so that the references remain.
  static Circuit *FromVLSIRModule(const vlsir::circuit::Module &module_pb);
  static Circuit *FromVLSIRExternalModule(
      const vlsir::circuit::ExternalModule &module_pb);

  Circuit() = default;

  // Merges another circuit into this one, creating copies of all the elements.
  // Use the name_prefix argument to prefix all incoming entities with the given
  // string. If any elements in the other circuit conflicts (by name) with an
  // existing one, this will fail.
  //
  // Note that this is fundamentally different to the schematicaly
  void AddCircuit(const Circuit &other, const std::string &prefix = "");

  // Convenient: Add a width=1 signal and return a wire indexing it. Wires are
  // designed to be ephemeral.
  // TODO(aryap): How to avoid a copy here? Copy elision works?
  circuit::Wire AddSignal(const std::string &name);

  circuit::Signal *GetSignal(const std::string &name);
  circuit::Signal *GetOrAddSignal(const std::string &name, uint64_t width);
  circuit::Signal *AddSignal(const std::string &name, uint64_t width);
  circuit::Instance *AddInstance(
    const std::string &name, const Circuit *template_module);
  circuit::Port *AddPort(
      const circuit::Signal &signal,
      const circuit::Port::PortDirection &direction = circuit::Port::NONE);
  circuit::Port *AddPort(
      const circuit::Wire &wire,
      const circuit::Port::PortDirection &direction = circuit::Port::NONE);

  void AddGlobal(const circuit::Wire &wire);
  void AddGlobal(circuit::Signal *signal);
  void SetParameter(const std::string &name, const Parameter &value);

  const circuit::Signal *GetSignal(const std::string &name) const {
    auto signals_it = signals_by_name_.find(name);
    if (signals_it == signals_by_name_.end()) {
      return nullptr;
    }
    return signals_it->second;
  }

  void set_parent_cell(bfg::Cell *cell) { parent_cell_ = cell; }
  bfg::Cell *parent_cell() const { return parent_cell_; }

  std::vector<std::unique_ptr<circuit::Signal>> &signals() { return signals_; }

  std::string Describe() const;

  ::vlsir::circuit::Module ToVLSIRModule() const;
  ::vlsir::circuit::ExternalModule ToVLSIRExternalModule() const;

  const std::string &NameOrParentName() const;
  const std::string &DomainOrParentDomain() const;

  const std::string &domain() const { return domain_; }
  void set_domain(const std::string &domain) { domain_ = domain; }

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  const std::vector<std::unique_ptr<circuit::Instance>> &instances() const {
    return instances_;
  };

  const std::unordered_map<std::string, Parameter> &parameters() const {
    return parameters_;
  }

 protected:
  bool IsGlobal(const circuit::Signal &signal) const;
  bool IsPowerOrGround(const circuit::Signal &signal) const;

 private:
  static constexpr std::string_view kDefaultNetPrefix = "n_";

  std::string GenerateDefaultName();

  bfg::Cell *parent_cell_;

  Type type_;

  std::string domain_;
  std::string name_;
  std::string description_;

  // We own these objects but we don't want their address to change when the
  // vectors resize (right?).
  std::vector<std::unique_ptr<circuit::Signal>> signals_;
  std::vector<std::unique_ptr<circuit::Port>> ports_;
  std::vector<std::unique_ptr<circuit::Instance>> instances_;

  // Pointers to signals that should be treated as power and ground,
  // respectively.
  std::set<circuit::Signal*> power_signals_;
  std::set<circuit::Signal*> ground_signals_;

  // Global signals are treated differently when resolving signals names in
  // hierarchies, since they are available everywhere. They tend to miss out on
  // any prefixes being added.
  //
  // Global signals must outlive this object.
  std::set<const circuit::Signal*> global_signals_;

  std::unordered_map<std::string, circuit::Signal*> signals_by_name_;
  std::unordered_map<std::string, circuit::Instance*> instances_by_name_;

  std::unordered_map<std::string, Parameter> parameters_;

  uint64_t unnamed_net_count_;
};


}  // namespace bfg

#endif  // CIRCUIT_H_
