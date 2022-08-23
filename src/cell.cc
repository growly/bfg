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

std::set<Cell*> Cell::DirectAncestors() const {
  std::set<Cell*> ancestors;
  if (circuit_) {
    for (const auto &instance : circuit_->instances()) {
      if (!instance->module()) {
        LOG(WARNING) << "Circuit instance " << instance->name()
                     << " has no parent circuit module";
        continue;
      }
      if (!instance->module()->parent_cell()) {
        LOG(WARNING) << "Instance " << instance->name()
                     << " is of circuit with no parent cell";
      }
      ancestors.insert(instance->module()->parent_cell());
    }
  }
  if (layout_) {
    for (const auto &instance : layout_->instances()) {
      if (!instance.template_layout()) {
        LOG(WARNING) << "Circuit instance " << instance.name()
                     << " has no parent layout template";
        continue;
      }
      if (!instance.template_layout()->parent_cell()) {
        LOG(WARNING) << "Instance " << instance.name()
                     << " is of layout with no parent cell";
      }
      ancestors.insert(instance.template_layout()->parent_cell());
    }
  }
  return ancestors;
}

}  // namespace bfg
