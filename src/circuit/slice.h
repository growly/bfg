#ifndef CIRCUIT_SLICE_H_
#define CIRCUIT_SLICE_H_

#include <string>

#include "signal.h"
#include "vlsir/circuit.pb.h"

namespace bfg {

class Circuit;

namespace circuit {

class Slice {
 public:
  static Slice FromVLSIRSlice(
      const Circuit &circuit,
      const vlsir::circuit::Slice &slice_pb);
  Slice(const Signal &signal, uint64_t low_index, uint64_t high_index)
      : signal_(signal),
        low_index_(low_index),
        high_index_(high_index) {}

  Slice(const Slice &other)
      : signal_(other.signal_),
        low_index_(other.low_index_),
        high_index_(other.high_index_) {}

  // Can't copy-assign a const Signal&, so force the compiler's hand. This feels
  // so icky.
  // TODO(aryap): Why does this even work? Why doesn't operator= get called in
  // the *this = ... call?
  //Slice &operator=(const Slice &other) {
  //  *this = Slice(other);
  //  return *this;
  //}

  ::vlsir::circuit::Slice ToVLSIRSlice() const {
    ::vlsir::circuit::Slice slice_pb;
    slice_pb.set_signal(signal_.name());
    slice_pb.set_bot(low_index_);
    slice_pb.set_top(high_index_);
    return slice_pb;
  }

  uint64_t Width() const { return high_index_ - low_index_ + 1; }

  const Signal &signal() const { return signal_; }
  uint64_t low_index() const { return low_index_; }
  uint64_t high_index() const { return high_index_; }

 private:
  const Signal &signal_;
  uint64_t low_index_;
  uint64_t high_index_;
};

}  // namespace circuit
 
std::ostream &operator<<(std::ostream &os, const circuit::Slice &slice);

}  // namespace bfg

#endif  // CIRCUIT_SLICE_H_
