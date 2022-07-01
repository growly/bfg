#include "circuit_primitives_database.h"

#include <string>

namespace bfg {

Circuit *CircuitPrimitivesDatabase::GetCircuit(const std::string &name) {
  return new Circuit();
}

}  // namespace bfg
