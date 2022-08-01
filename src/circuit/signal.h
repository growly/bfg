#ifndef CIRCUIT_SIGNAL_H_
#define CIRCUIT_SIGNAL_H_

#include <string>

#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

class Signal {
 public:
  Signal() = default;
  Signal(const std::string &name, uint64_t width)
      : name_(name),
        width_(width) {}

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  uint64_t width() const { return width_; }
  void set_width(const uint64_t width) { width_ = width; }

  ::vlsir::circuit::Signal ToVLSIRSignal() const;
 private:
  std::string name_;
  uint64_t width_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_SIGNAL_H_
