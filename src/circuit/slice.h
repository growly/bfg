#ifndef CIRCUIT_SLICE_H_
#define CIRCUIT_SLICE_H_

#include <string>

#include "signal.h"

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

 private:
  const Signal &signal_;
  uint64_t low_index_;
  uint64_t high_index_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_SLICE_H_
