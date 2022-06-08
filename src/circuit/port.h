#ifndef CIRCUIT_PORT_H_
#define CIRCUIT_PORT_H_

namespace bfg {
namespace circuit {

enum PortDirection {
  INPUT,
  OUTPUT,
  INOUT,
  NONE
};

class Port {
 public:
  Port() = default;

  Signal *signal() const { return signal_; }
  void set_signal(Signal *signal) { signal_ = signal; }

  const PortDirection &direction { return direction_; }
  void set_direction(const PortDirection &direction) { direction_ = direction; }

 private:
  Signal *signal_;
  PortDirection direction_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_PORT_H_
