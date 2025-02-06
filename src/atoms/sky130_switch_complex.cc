#include "sky130_switch_complex.h"

#include "../circuit.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"

namespace bfg {
namespace atoms {

bfg::Cell *Sky130SwitchComplex::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_switch_complex": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Layout *Sky130SwitchComplex::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  layout->SetActiveLayerByName("poly.drawing");

  //    NE_B EW_B      NS_B ES_B      NW_B SW_B
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //
  //
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //    NE   EW        NS   ES        NW   SW

  int64_t poly_x = 0;
  geometry::PolyLine line_ne_b = geometry::PolyLine(
      {{poly_x, 0}, {poly_x, 300}});
  line_ne_b.SetWidth(150);

  // TODO(aryap): Add "Layout::AddInflatedPolyLineOrDie":
  std::optional<Polygon> polygon = InflatePolyLine(db, line);

  return layout.release();
}

bfg::Circuit *Sky130SwitchComplex::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
