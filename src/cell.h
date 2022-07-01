#ifndef CELL_H_
#define CELL_H_

#include <string>
#include <vector>

#include "circuit.h"
#include "layout.h"

namespace bfg {

class Cell {
 public:
  Cell() = default;
  Cell(const std::string &name) : name_(name) {}

  void set_layout(Layout *layout) { layout_.reset(layout); }
  Layout *layout() { return layout_.get(); }
  void set_circuit(Circuit *circuit) { circuit_.reset(circuit); }
  Circuit *circuit() { return circuit_.get(); }

 private:
  std::string name_;

  std::unique_ptr<Layout> layout_;
  std::unique_ptr<Circuit> circuit_;
};

}  // namespace bfg

#endif  // CELL_H_
