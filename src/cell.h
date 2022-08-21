#ifndef CELL_H_
#define CELL_H_

#include <string>
#include <vector>

#include "circuit.h"
#include "layout.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

class Cell {
 public:
  Cell() = default;
  Cell(const std::string &name) : name_(name) {}

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  void SetLayout(Layout *layout) {
    layout_.reset(layout);
    layout->set_parent_cell(this);
  }
  Layout *layout() { return layout_.get(); }
  Layout *const layout() const { return layout_.get(); }

  void SetCircuit(Circuit *circuit) {
    circuit_.reset(circuit);
    circuit->set_parent_cell(this);
  }
  Circuit *circuit() { return circuit_.get(); }
  Circuit *const circuit() const { return circuit_.get(); }

  ::vlsir::raw::Cell ToVLSIRCell() const;

 private:
  std::string name_;

  std::unique_ptr<Layout> layout_;
  std::unique_ptr<Circuit> circuit_;
};

}  // namespace bfg

#endif  // CELL_H_
