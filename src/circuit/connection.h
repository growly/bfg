#ifndef CIRCUIT_CONNECTION_H_
#define CIRCUIT_CONNECTION_H_

#include "slice.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

class Signal;

class Connection {
 public:
  enum ConnectionType {
    SIGNAL,
    SLICE,
    CONCATENATION
  };

  Connection() = default;

  Connection(const Connection &other)
      : connection_type_(other.connection_type_),
        signal_(other.signal_),
        slice_(other.slice_ ? new Slice(*other.slice_) : nullptr) {}

  Connection &operator=(const Connection &other) {
    *this = Connection(other);
    return *this;
  }

  void set_signal(Signal *signal) {
    connection_type_ = SIGNAL;
    signal_ = signal;
  }

  void set_slice(const Slice &slice) {
    connection_type_ = SLICE;
    slice_.reset(new Slice(slice));
  }

  ::vlsir::circuit::ConnectionTarget ToVLSIRConnection() const;

 private:
  ConnectionType connection_type_;

  // Signals are owned by the Circuit.
  Signal *signal_;

  // Slices, Connections, Wires ephemeral so we keep a copy.
  std::unique_ptr<Slice> slice_;
  // Concatenation concatenation_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_CONNECTION_H_
