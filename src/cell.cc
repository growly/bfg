#include "cell.h"

#include "circuit.h"
#include "layout.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

::vlsir::raw::Cell Cell::ToVLSIRCell() const {
  ::vlsir::raw::Cell cell_pb;
  cell_pb.set_name(name_);
  if (layout_)
    *cell_pb.mutable_layout() = layout_->ToVLSIRLayout();
  if (circuit_)
    *cell_pb.mutable_module() = circuit_->ToVLSIRCircuit();
  return cell_pb;
}

}  // namespace bfg
