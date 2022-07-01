#ifndef CIRCUIT_PRIMITIVES_DATABASE_H_
#define CIRCUIT_PRIMITIVES_DATABASE_H_

#include <string>

#include "circuit.h"

namespace bfg {

// Stores template Circuits for primitive devices relied upon but other circuits
// throughout this program. This includes basic primitives like "inductor",
// "resistor" and "capacitor", but also PDK-dependent devices like "nmos_rvt",
// "pmos_hvt", etc.
class CircuitPrimitivesDatabase {
 public:
  CircuitPrimitivesDatabase() = default;

  Circuit *GetCircuit(const std::string &name);

 private:

};

}  // namespace bfg

#endif  // CIRCUIT_PRIMITIVES_DATABASE_H_
