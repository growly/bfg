#include "layout_demo.h"

#include <memory>

#include "atom.h"
#include "../cell.h"
#include "../circuit.h"
#include "../layout.h"
#include "../physical_properties_database.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/rectangle.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::PolyLine;
using ::bfg::geometry::Rectangle;

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
  // The PhysicalPropertiesDatabase holds the process technology: the layer
  // names, the internal-unit scale, and the design rules. A Layout is always
  // created against it.
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  // All geometry is in "internal units"; convert our nanometre parameters up
  // front. The FET width runs vertically (along the diffusion), the length is
  // the horizontal width of the poly gate.
  int64_t width = db.ToInternalUnits(parameters_.width_nm);
  int64_t length = db.ToInternalUnits(parameters_.length_nm);

  // Design rules relating poly and (p-type) diffusion tell us how far the poly
  // must overhang the diffusion (min_enclosure), and how far the diffusion must
  // extend beyond the poly on either side to make room for the source/drain
  // (min_extension).
  const auto &poly_diff_rules = db.Rules("poly.drawing", "pdiff.drawing");
  int64_t poly_overhang = poly_diff_rules.min_enclosure;
  int64_t diff_wing = poly_diff_rules.min_extension;

  // The transistor is centred on the origin (0, 0).

  // Diffusion: p-type, since this is a PMOS. The source/drain diffusion extends
  // a "wing" either side of the poly.
  layout->SetActiveLayerByName("pdiff.drawing");
  Rectangle *diff = layout->AddRectangle(Rectangle(
      Point(-length / 2 - diff_wing, -width / 2),
      Point(length / 2 + diff_wing, width / 2)));

  // A PMOS lives inside an n-well, and its diffusion must be marked p-type with
  // a p-select (psdm) implant. Both of these layers must enclose the diffusion
  // by at least the min_enclosure given by the design rules.
  int64_t nwell_enclosure =
      db.Rules("pdiff.drawing", "nwell.drawing").min_enclosure;
  int64_t psdm_enclosure =
      db.Rules("pdiff.drawing", "psdm.drawing").min_enclosure;
  Point diff_ll = diff->lower_left();
  Point diff_ur = diff->upper_right();

  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle(Rectangle(
      Point(diff_ll.x() - nwell_enclosure, diff_ll.y() - nwell_enclosure),
      Point(diff_ur.x() + nwell_enclosure, diff_ur.y() + nwell_enclosure)));

  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddRectangle(Rectangle(
      Point(diff_ll.x() - psdm_enclosure, diff_ll.y() - psdm_enclosure),
      Point(diff_ur.x() + psdm_enclosure, diff_ur.y() + psdm_enclosure)));

  // Poly gate: a vertical wire of the FET's length, crossing the diffusion and
  // overhanging it top and bottom. A PolyLine is a convenient way to draw a
  // wire of a fixed width along a centreline.
  layout->SetActiveLayerByName("poly.drawing");
  PolyLine gate({Point(0, -width / 2 - poly_overhang),
                 Point(0, width / 2 + poly_overhang)});
  gate.SetWidth(length);
  layout->AddPolyLine(gate);

  // Save a few named points so a parent generator could align to or connect
  // against this transistor.
  layout->SavePoint("diff_lower_left", diff->lower_left());
  layout->SavePoint("diff_upper_right", diff->upper_right());
  layout->SavePoint("gate_bottom_centre", Point(0, -width / 2 - poly_overhang));
  layout->SavePoint("gate_top_centre", Point(0, width / 2 + poly_overhang));

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
