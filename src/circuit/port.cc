#include "port.h"
#include "signal.h"

#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

::vlsir::circuit::Port Port::ToVLSIRPort() const {
  ::vlsir::circuit::Port port_pb;
  port_pb.set_signal(signal_.name());
  port_pb.set_direction(VLSIRPortDirection());
  return port_pb;
}

::vlsir::circuit::Port::Direction Port::VLSIRPortDirection() const {
  switch (direction_) {
    case INPUT:
      return ::vlsir::circuit::Port::INPUT;
    default:
      return ::vlsir::circuit::Port::NONE;
  }
}

}  // namespace circuit
}  // namespace bfg
