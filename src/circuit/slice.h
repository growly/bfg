#ifndef CIRCUIT_SLICE_H_
#define CIRCUIT_SLICE_H_

#include <string>

#include "signal.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace circuit {

class Slice {
 public:
  Slice(const Signal &signal, uint64_t low_index, uint64_t high_index)
      : signal_(signal),
        low_index_(low_index),
        high_index_(high_index) {}

  Slice(const Slice &other)
      : signal_(other.signal_),
        low_index_(other.low_index_),
        high_index_(other.high_index_) {}

  // Slice &operator=(const Slice &other) {
  //   *this = Slice(other);
  //   return *this;
  // }

  ::vlsir::circuit::Slice ToVLSIRSlice() const {
    ::vlsir::circuit::Slice slice_pb;
    slice_pb.set_signal(signal_.name());
    slice_pb.set_bot(low_index_);
    slice_pb.set_top(high_index_);
    return slice_pb;
  }

  const Signal &signal() const { return signal_; }

 private:
  const Signal &signal_;
  uint64_t low_index_;
  uint64_t high_index_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_SLICE_H_
