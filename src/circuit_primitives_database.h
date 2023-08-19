#ifndef CIRCUIT_PRIMITIVES_DATABASE_H_
#define CIRCUIT_PRIMITIVES_DATABASE_H_

#include <string>

#include "circuit.h"

#include "vlsir/circuit.pb.h"
#include "vlsir/tech.pb.h"

namespace bfg {

// Ok the point of this class should be to provide a way to look up circuits
// based on their properties. That is, it should be responsile for mapping
// generic abstract things a designer might need (e.g. "resistor", "capacitor",
// "NMOS FET with low VT") to the primitives that come in the PDK.
//
// That might ultimately not be useful at all.
class CircuitPrimitivesDatabase {
 public:
  CircuitPrimitivesDatabase() = default;

  // TODO(aryap): This is needed for a conistent interface with
  // PhysicalPropertiesDatabase, but needs the package to be part of the
  // Technology proto first.
  void LoadTechnology(const vlsir::tech::Technology &pdk);

  // TODO(aryap): Loading packages that have technology primitives should happen
  // at the same time that the Technology is loaded. Additional data about the
  // devices should be included (V_th levels, etc).
  void LoadPackage(const vlsir::circuit::Package &package);

  Circuit *GetCircuit(const std::string &name) const;

  std::string Describe() const;

 private:
  std::vector<std::unique_ptr<Circuit>> circuits_;
  std::unordered_map<std::string, Circuit*> circuits_by_name_;
};

}  // namespace bfg

#endif  // CIRCUIT_PRIMITIVES_DATABASE_H_
