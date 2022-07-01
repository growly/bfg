#ifndef CIRCUIT_WIRE_H_
#define CIRCUIT_WIRE_H_

#include <string>

#include "slice.h"

namespace bfg {
namespace circuit {

// A Wire is a single index into a Signal, i.e. it represents single wires, not
// buses. However, since it is just a glorified (Signal, index) pair, it
// requires that a Signal already exist.
//
// In practice a Wire is just a Slice with the low and high indices set to the
// same thing.
class Wire : public Slice {
 public:
  Wire(const Signal &signal, uint64_t index)
      : Slice(signal, index, index) {}
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_WIRE_H_
