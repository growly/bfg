#include "signal.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

::vlsir::circuit::Signal Signal::ToVLSIRSignal() const {
  ::vlsir::circuit::Signal signal_pb;
  signal_pb.set_name(name_);
  signal_pb.set_width(width_);
  return signal_pb;
}

}  // namespace circuit

std::ostream &operator<<(std::ostream &os, const circuit::Signal &signal) {
  os << signal.name();
  return os;
}

}  // namespace bfg
