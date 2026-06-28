#include "signal.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

bool Signal::CompareByName(const Signal &lhs, const Signal &rhs) {
  return lhs.name() < rhs.name();
}
bool Signal::ComparePtrByName(Signal *lhs, Signal *rhs) {
  return CompareByName(*lhs, *rhs);
}

::vlsir::circuit::Signal Signal::ToVLSIRSignal() const {
  ::vlsir::circuit::Signal signal_pb;
  signal_pb.set_name(name_);
  signal_pb.set_width(width_);
  return signal_pb;
}

}  // namespace circuit

bool operator==(const circuit::Signal &lhs, const circuit::Signal &rhs) {
  if (lhs.width() != rhs.width()) {
    return false;
  }
  return lhs.name() == rhs.name();
}

std::ostream &operator<<(std::ostream &os, const circuit::Signal &signal) {
  os << signal.name();
  return os;
}

}  // namespace bfg
