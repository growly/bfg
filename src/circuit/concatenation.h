#ifndef CIRCUIT_CONCATENATION_H_
#define CIRCUIT_CONCATENATION_H_

#include <string>
#include <variant>

#include "catenable.h"
#include "vlsir/circuit.pb.h"

namespace bfg {

class Circuit;

namespace circuit {

// FIXME(aryap): This is generally unimplemented.
class Concatenation {
 public:
  Concatenation() {}

  Concatenation(const std::vector<Catenable> &members)
      : members_(members) {}

  ::vlsir::circuit::Concat ToVLSIRConcatenation() const {
    ::vlsir::circuit::Concat concat_pb;
    return concat_pb;
  }

  // TODO(aryap): Needs to sum over the widths of all members_.
  uint64_t Width() const { return 0; }

 private:
  std::vector<Catenable> members_;
};

}  // namespace circuit
 
std::ostream &operator<<(
    std::ostream &os, const circuit::Concatenation &concat);

}  // namespace bfg

#endif  // CIRCUIT_CONCATENATION_H_
