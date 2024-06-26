#include "slice.h"
#include "../circuit.h"

namespace bfg {
namespace circuit {

Slice Slice::FromVLSIRSlice(const Circuit &circuit,
                            const vlsir::circuit::Slice &slice_pb) {
  const Signal *signal;
  if (!signal) {
    LOG(FATAL) << "Signal " << slice_pb.signal() << " not found";
  };
  return Slice(*signal, slice_pb.bot(), slice_pb.top());
}

}  // namespace circuit
}  // namespace bfg
