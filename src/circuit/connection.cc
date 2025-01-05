#include "connection.h"
#include "../circuit.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

Connection Connection::FromVLSIRConnection(
    const Circuit &circuit,
    const vlsir::circuit::Connection &conn_pb) {
  const Signal *signal;
  Connection connection;
  switch (conn_pb.target().stype_case()) {
    case vlsir::circuit::ConnectionTarget::StypeCase::kSig:
      signal = circuit.GetSignal(conn_pb.target().sig());
      if (!signal) {
        LOG(WARNING) << "Signal " << conn_pb.target().sig() << " not found";
      } else {
        connection.set_signal(signal);
      }
      break;
    case vlsir::circuit::ConnectionTarget::StypeCase::kSlice: {
      const vlsir::circuit::Slice &slice_pb = conn_pb.target().slice();
      connection.set_slice(Slice::FromVLSIRSlice(circuit, slice_pb));
      break;
    }
    case vlsir::circuit::ConnectionTarget::StypeCase::kConcat:
      // TODO(aryap).
      break;
  }
  return connection;
}

vlsir::circuit::ConnectionTarget Connection::ToVLSIRConnection() const {
  vlsir::circuit::ConnectionTarget target_pb;
  switch(connection_type_) {
    case SIGNAL:
      target_pb.set_sig(signal_->name());
      break;
    case SLICE:
      *target_pb.mutable_slice() = slice_->ToVLSIRSlice();
      break;
    case CONCATENATION:
      // TODO(aryap).
      // Fallthrough intended.
    default:
      break;
  }
  return target_pb;
}

}  // namespace circuit

std::ostream &operator<<(
    std::ostream &os, const circuit::Connection &connection) {
  os << "[";

  switch (connection.connection_type()) {
    case circuit::Connection::ConnectionType::SIGNAL:
      os << "Signal: " << connection.signal()->name();
      break;
    case circuit::Connection::ConnectionType::SLICE:
      os << "Slice: " << connection.slice()->signal().name();
      break;
    case circuit::Connection::ConnectionType::CONCATENATION:
      os << "Concatenation";
      break;
  }
  os << "]";
  return os;
}

}  // namespace bfg
