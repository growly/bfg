#include "sky130_transmission_gate.h"

#include <algorithm>
#include <cmath>

#include "../layout.h"
#include "../scoped_layer.h"
#include "../geometry/compass.h"
#include "../geometry/point.h"
#include "../geometry/vector.h"
#include "sky130_simple_transistor.h"

namespace bfg {
namespace atoms {

int64_t Sky130TransmissionGate::PolyTabHeight(
    const Sky130SimpleTransistor &fet_generator) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  const auto &dcon_rules = db.Rules(fet_generator.DiffConnectionLayer());
  const auto &diff_dcon_rules = db.Rules(
      fet_generator.DiffLayer(), fet_generator.DiffConnectionLayer());
  int64_t via_height = dcon_rules.via_height;
  return via_height + 2 * diff_dcon_rules.via_overhang_wide;
}

int64_t Sky130TransmissionGate::PolyTabWidth(
    const Sky130SimpleTransistor &fet_generator) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &dcon_rules = db.Rules(fet_generator.DiffConnectionLayer());

  const auto &diff_dcon_rules = db.Rules(
      fet_generator.DiffLayer(), fet_generator.DiffConnectionLayer());
  int64_t via_width = dcon_rules.via_width;
  return via_width + 2 * diff_dcon_rules.via_overhang;
}

geometry::Rectangle *Sky130TransmissionGate::AddPolyTab(
    const Sky130SimpleTransistor &fet_generator,
    const geometry::Compass &position,
    Layout *layout) {
  int64_t tab_height = PolyTabHeight(fet_generator);
  int64_t tab_width = PolyTabWidth(fet_generator);

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
    case geometry::Compass::UPPER:
      tab_ll = geometry::Point((poly_ll.x() + poly_ur.x() - tab_width) / 2,
                               poly_ur.y());
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
    case geometry::Compass::LOWER:
      tab_ur = geometry::Point((poly_ll.x() + poly_ur.x() + tab_width) / 2,
                               poly_ll.y());
      tab_ll = tab_ur - tab_diagonal;
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

int64_t Sky130TransmissionGate::PMOSPolyHeight() const {
  return pfet_generator_->PolyHeight() + (
      parameters_.p_tab_position ?  PMOSPolyTabHeight() : 0);
}

int64_t Sky130TransmissionGate::NMOSPolyHeight() const {
  return nfet_generator_->PolyHeight() + (
      parameters_.n_tab_position ?  NMOSPolyTabHeight() : 0);
}

int64_t Sky130TransmissionGate::FigureVerticalPadding(
    const Sky130SimpleTransistor &fet_generator, bool abuts_tab) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(fet_generator.PolyLayer());
  int64_t minimum = poly_rules.min_separation / 2;
  if (!parameters_.vertical_tab_pitch_nm || !abuts_tab) {
    return minimum;
  }
  int64_t pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t padding = (pitch - PolyTabHeight(fet_generator)) / 2;

  if (padding < minimum) {
    padding += pitch / 2;
  }

  return padding;
}

int64_t Sky130TransmissionGate::FigureTopPadding() const {
  return FigureVerticalPadding(*pfet_generator_, PMOSHasUpperTab());
}

int64_t Sky130TransmissionGate::FigureBottomPadding() const {
  return FigureVerticalPadding(*nfet_generator_, NMOSHasLowerTab());
}

int64_t Sky130TransmissionGate::FigureCellHeight() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // FIXME(aryap): This should also account for the minimum nwell/nsdm/psdm
  // spacing rules!
  const auto &poly_rules = db.Rules(nfet_generator_->PolyLayer());
  int64_t minimum = FigureBottomPadding() + NMOSPolyHeight() +
      poly_rules.min_separation + PMOSPolyHeight() + FigureTopPadding();

  if (parameters_.cell_height_nm && parameters_.vertical_tab_pitch_nm) {
    minimum = std::max(
        minimum, db.ToInternalUnits(*parameters_.cell_height_nm));
    double divisor = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
    return std::ceil(static_cast<double>(minimum) / divisor) * divisor;
  }

  // FIXME(aryap): Cell height should be the *tiling* cell height, so there
  // should be sapce at the top and bottom so that the diff regions are not to
  // close to (what we can expect will be) external instances of diff regions...
  if (parameters_.cell_height_nm) {
    return std::max(minimum, db.ToInternalUnits(*parameters_.cell_height_nm));
  } else if (parameters_.vertical_tab_pitch_nm) {
    double divisor = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
    return std::ceil(static_cast<double>(minimum) / divisor) * divisor;
  }
  return minimum;
}


bool Sky130TransmissionGate::PMOSHasUpperTab() const {
  if (!parameters_.p_tab_position)
    return false;
  switch (*parameters_.p_tab_position) {
    case geometry::Compass::UPPER_LEFT:
      // Fallthrough intended.
    case geometry::Compass::UPPER:
      // Fallthrough intended.
    case geometry::Compass::UPPER_RIGHT:
      return true;
    default:
      return false;
  }
}

bool Sky130TransmissionGate::PMOSHasLowerTab() const {
  if (!parameters_.p_tab_position)
    return false;
  switch (*parameters_.p_tab_position) {
    case geometry::Compass::LOWER_LEFT:
      // Fallthrough intended.
    case geometry::Compass::LOWER:
      // Fallthrough intended.
    case geometry::Compass::LOWER_RIGHT:
      return true;
    default:
      return false;
  }
}

bool Sky130TransmissionGate::NMOSHasUpperTab() const {
  if (!parameters_.n_tab_position)
    return false;
  switch (*parameters_.n_tab_position) {
    case geometry::Compass::UPPER_LEFT:
      // Fallthrough intended.
    case geometry::Compass::UPPER:
      // Fallthrough intended.
    case geometry::Compass::UPPER_RIGHT:
      return true;
    default:
      return false;
  }
}

bool Sky130TransmissionGate::NMOSHasLowerTab() const {
  if (!parameters_.n_tab_position)
    return false;
  switch (*parameters_.n_tab_position) {
    case geometry::Compass::LOWER_LEFT:
      // Fallthrough intended.
    case geometry::Compass::LOWER:
      // Fallthrough intended.
    case geometry::Compass::LOWER_RIGHT:
      return true;
    default:
      return false;
  }
}

bfg::Layout *Sky130TransmissionGate::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  const auto &poly_rules = db.Rules(nfet_generator_->PolyLayer());

  int64_t cell_height = FigureCellHeight();

  int64_t anchor_y_high = cell_height - (PMOSPolyHeight() + FigureTopPadding());
  if (PMOSHasLowerTab()) {
    anchor_y_high += PMOSPolyTabHeight();
  }

  int64_t anchor_y_low = FigureBottomPadding();
  if (NMOSHasLowerTab()) {
    anchor_y_low += NMOSPolyTabHeight();
  }

  nfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, anchor_y_low});
  pfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, anchor_y_high});

  std::unique_ptr<bfg::Layout> nfet_layout(nfet_generator_->GenerateLayout());
  layout->AddLayout(*nfet_layout, "nmos");
  std::unique_ptr<bfg::Layout> pfet_layout(pfet_generator_->GenerateLayout());
  layout->AddLayout(*pfet_layout, "pmos");

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

  geometry::Rectangle pre_well_bounds = layout->GetBoundingBox();

  if (parameters_.draw_nwell) {
    ScopedLayer layer(layout.get(), "nwell.drawing");

    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    layout->AddRectangle(PMOSBounds().WithPadding(nwell_margin));
  }

  // Set tiling bounds.
  {
    int64_t min_y = pre_well_bounds.lower_left().y();
    geometry::Rectangle tiling_bounds = geometry::Rectangle(
        {pre_well_bounds.lower_left().x(), 0},
        {pre_well_bounds.upper_right().x(), cell_height});
    ScopedLayer layer(layout.get(), "areaid.standardc");
    layout->AddRectangle(tiling_bounds);
    layout->SetTilingBounds(tiling_bounds);
  }

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
