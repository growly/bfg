#include "port.h"
#include "signal.h"

#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

Port::PortDirection Port::FromVLSIRPortDirection(
    const ::vlsir::circuit::Port::Direction &direction_pb) {
  switch (direction_pb) {
    case ::vlsir::circuit::Port::INPUT:
      return INPUT;
    case ::vlsir::circuit::Port::OUTPUT:
      return OUTPUT;
    case ::vlsir::circuit::Port::INOUT:
      return INOUT;
    default:
       return NONE;
  }
}

::vlsir::circuit::Port::Direction Port::ToVLSIRPortDirection(
    const PortDirection &direction) {
  switch (direction) {
    case INPUT:
      return ::vlsir::circuit::Port::INPUT;
    case OUTPUT:
      return ::vlsir::circuit::Port::OUTPUT;
    case INOUT:
      return ::vlsir::circuit::Port::INOUT;
    default:
      return ::vlsir::circuit::Port::NONE;
  }
}

::vlsir::circuit::Port Port::ToVLSIRPort() const {
  ::vlsir::circuit::Port port_pb;
  port_pb.set_signal(signal_.name());
  port_pb.set_direction(ToVLSIRPortDirection(direction_));
  return port_pb;
}

}  // namespace circuit
}  // namespace bfg
