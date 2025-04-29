#include "sky130_transmission_gate.h"

#include "../layout.h"
#include "../scoped_layer.h"
#include "../geometry/compass.h"
#include "../geometry/point.h"
#include "../geometry/vector.h"
#include "sky130_simple_transistor.h"

namespace bfg {
namespace atoms {

geometry::Rectangle *Sky130TransmissionGate::AddPolyTab(
    const Sky130SimpleTransistor &fet_generator,
    const geometry::Compass &position,
    Layout *layout) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  // The tab will be a horizontal rectangle, whose height and width must
  // accommodate a via on the DiffConnectionLayer().

  const geometry::Point &origin = fet_generator.origin();

  const auto &dcon_rules = db.Rules(fet_generator.DiffConnectionLayer());
  const auto &diff_dcon_rules = db.Rules(
      fet_generator.DiffLayer(), fet_generator.DiffConnectionLayer());
  int64_t via_width = dcon_rules.via_width;
  int64_t via_height = dcon_rules.via_height;

  int64_t tab_height = via_height + 2 * diff_dcon_rules.via_overhang_wide;
  int64_t tab_width = via_width + 2 * diff_dcon_rules.via_overhang;

  geometry::Point poly_ll = fet_generator.PolyLowerLeft();
  geometry::Point poly_ur = fet_generator.PolyUpperRight();

  geometry::Point tab_ll;
  geometry::Point tab_ur;
  geometry::Vector tab_diagonal(tab_width, tab_height);
  switch (position) {
    case geometry::Compass::UPPER_LEFT:
      tab_ur = poly_ur + geometry::Vector(0, tab_height);
      tab_ll = tab_ur - tab_diagonal;
      break;
    case geometry::Compass::UPPER_RIGHT:
      tab_ll = geometry::Point(poly_ll.x(), poly_ur.y());
      tab_ur = tab_ll + tab_diagonal;
      break;
    case geometry::Compass::LOWER_LEFT:
      tab_ur = geometry::Point(poly_ur.x(), poly_ll.y());
      tab_ll = tab_ur - tab_diagonal;
      break;
    case geometry::Compass::LOWER_RIGHT:
      tab_ll = poly_ll - geometry::Vector(0, tab_height); 
      tab_ur = tab_ll + tab_diagonal;
      break;
    default:
      LOG(FATAL) << "Unsupported poly tab position: " << position;
  }
  ScopedLayer layer(layout, fet_generator.PolyLayer());
  return layout->AddRectangle(geometry::Rectangle(tab_ll, tab_ur));
}

bfg::Cell *Sky130TransmissionGate::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_transmission_gate": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *Sky130TransmissionGate::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

bfg::Layout *Sky130TransmissionGate::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  // Assume the PMOS and NMOS poly layers are the same!
  const auto &poly_rules = db.Rules(nfet_generator_->PolyLayer());

  int64_t n_height = nfet_generator_->PolyHeight();
  int64_t p_height = pfet_generator_->PolyHeight();

  // FIXME(aryap): Cell height should be the *tiling* cell height, so there
  // should be sapce at the top and bottom so that the diff regions are not to
  // close to (what we can expect will be) external instances of diff regions...
  int64_t anchor_y = parameters_.cell_height_nm ?
      db.ToInternalUnits(*parameters_.cell_height_nm) - p_height :
      n_height + poly_rules.min_separation;

  nfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, 0});
  pfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, anchor_y});


  std::unique_ptr<bfg::Layout> nfet_layout(nfet_generator_->GenerateLayout());
  layout->AddLayout(*nfet_layout, "nmos");
  std::unique_ptr<bfg::Layout> pfet_layout(pfet_generator_->GenerateLayout());
  layout->AddLayout(*pfet_layout, "pmos");

  if (parameters_.draw_nwell) {
    ScopedLayer layer(layout.get(), "nwell.drawing");

    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    layout->AddRectangle(PMOSBounds().WithPadding(nwell_margin));
  }

  //std::vector<Sky130SimpleTransistor::ViaPosition> positions = {
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER,
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE,
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_UPPER,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_MIDDLE,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_LOWER,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER
  //};
  //for (size_t i = 0; i < positions.size(); ++i) {
  //  layout->MakeVia(
  //      pfet_generator_->DiffConnectionLayer(),
  //      pfet_generator_->ViaLocation(positions[i]));
  //  layout->MakeVia(
  //      nfet_generator_->DiffConnectionLayer(),
  //      nfet_generator_->ViaLocation(positions[i]));
  //}
  if (parameters_.n_tab_position) {
    geometry::Rectangle *ntab = AddPolyTab(
        *nfet_generator_, *parameters_.n_tab_position, layout.get());

    //layout->MakeVia(
    //    nfet_generator_->DiffConnectionLayer(), ntab->centre());
  }
  if (parameters_.p_tab_position) {
    geometry::Rectangle *ptab = AddPolyTab(
        *pfet_generator_, *parameters_.p_tab_position, layout.get());
  }

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
