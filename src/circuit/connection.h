#ifndef CIRCUIT_CONNECTION_H_
#define CIRCUIT_CONNECTION_H_

#include "slice.h"
#include "vlsir/circuit.pb.h"

namespace bfg {

class Circuit;

namespace circuit {

class Signal;

class Connection {
 public:
  enum ConnectionType {
    SIGNAL,
    SLICE,
    CONCATENATION
  };

  static Connection FromVLSIRConnection(
      const Circuit &circuit,
      const vlsir::circuit::Connection &conn_pb);

  Connection() = default;

  Connection(const Connection &other)
      : connection_type_(other.connection_type_),
        signal_(other.signal_),
        slice_(other.slice_ ? new Slice(*other.slice_) : nullptr) {}

  Connection &operator=(const Connection &other) {
    *this = Connection(other);
    return *this;
  }

  ::vlsir::circuit::ConnectionTarget ToVLSIRConnection() const;

  void set_signal(const Signal *signal) {
    connection_type_ = SIGNAL;
    signal_ = signal;
  }

  void set_slice(const Slice &slice) {
    connection_type_ = SLICE;
    slice_.reset(new Slice(slice));
  }

  const ConnectionType &connection_type() const { return connection_type_; }
  const Signal *signal() const { return signal_; }
  Slice *slice() const { return slice_.get(); }

 private:
  ConnectionType connection_type_;

  // Signals are owned by the Circuit.
  const Signal *signal_;

  // Slices, Connections, Wires ephemeral so we keep a copy.
  std::unique_ptr<Slice> slice_;
  // Concatenation concatenation_;
};

}  // namespace circuit

std::ostream &operator<<(
    std::ostream &os,
    const circuit::Connection &connection);

}  // namespace bfg

#endif  // CIRCUIT_CONNECTION_H_
