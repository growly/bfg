#include "layout_demo.h"

#include <memory>

#include "atom.h"
#include "../cell.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {
namespace atoms {

bfg::Cell *LayoutDemo::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "layout_demo" : name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *LayoutDemo::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  return circuit.release();
}

bfg::Layout *LayoutDemo::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));
  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
