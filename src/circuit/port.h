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

  Port(const Signal &signal, const PortDirection &direction)
      : signal_(signal), direction_(direction) {}

  const Signal &signal() const { return signal_; }
  const PortDirection &direction() const { return direction_; }

  ::vlsir::circuit::Port ToVLSIRPort() const;
  ::vlsir::circuit::Port::Direction VLSIRPortDirection() const;

 private:
  const Signal &signal_;
  PortDirection direction_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_PORT_H_
