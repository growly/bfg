#include "circuit_primitives_database.h"

#include <string>
#include <sstream>

#include "vlsir/circuit.pb.h"

namespace bfg {

Circuit *CircuitPrimitivesDatabase::GetCircuit(const std::string &name) const {
  auto circuit_it = circuits_by_name_.find(name);
  if (circuit_it == circuits_by_name_.end())
    return nullptr;
  return circuit_it->second;
}

void CircuitPrimitivesDatabase::LoadPackage(
    const vlsir::circuit::Package &package) {
  LOG(FATAL) << "Don't use this yet.";
}

std::string CircuitPrimitivesDatabase::Describe() const {
  std::stringstream ss;
  for (const auto &entry : circuits_by_name_) {
    ss << entry.first << std::endl;
    for (const auto &param_entry : entry.second->parameters()) {
      ss << "\t" << param_entry.first << ": "
         << param_entry.second.description << std::endl;
    }
  }
  return ss.str();
}

}  // namespace bfg
