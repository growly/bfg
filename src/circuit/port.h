#ifndef CIRCUIT_PORT_H_
#define CIRCUIT_PORT_H_

#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

class Signal;

class Port {
 public:
  enum PortDirection {
    INPUT,
    OUTPUT,
    INOUT,
    NONE
  };

  Port() = default;

  Signal *signal() const { return signal_; }
  void set_signal(Signal *signal) { signal_ = signal; }

  const PortDirection &direction() const { return direction_; }
  void set_direction(const PortDirection &direction) { direction_ = direction; }

  ::vlsir::circuit::Port ToVLSIRPort() const;
  ::vlsir::circuit::Port::Direction VLSIRPortDirection() const;

 private:
  Signal *signal_;
  PortDirection direction_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_PORT_H_
