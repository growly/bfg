#include "connection.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

::vlsir::circuit::ConnectionTarget Connection::ToVLSIRConnection() const {
  ::vlsir::circuit::ConnectionTarget target_pb;
  switch(connection_type_) {
    case SIGNAL:
      target_pb.set_sig(signal_->name());
      break;
    case SLICE:
      *target_pb.mutable_slice() = slice_->ToVLSIRSlice();
      break;
    case CONCATENATION:
      // Fallthrough intended.
    default:
      break;
  }
  return target_pb;
}

}  // namespace circuit
}  // namespace bfg
