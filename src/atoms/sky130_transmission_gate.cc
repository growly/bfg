#include "sky130_transmission_gate.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../layout.h"
#include "../scoped_layer.h"
#include "../geometry/compass.h"
#include "../geometry/point.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../geometry/vector.h"
#include "proto/parameters/sky130_transmission_gate.pb.h"
#include "sky130_simple_transistor.h"

namespace bfg {
namespace atoms {

void Sky130TransmissionGate::Parameters::ToProto(
    proto::parameters::Sky130TransmissionGate *pb) const {
  pb->set_p_width_nm(p_width_nm);
  pb->set_p_length_nm(p_length_nm);
  pb->set_n_width_nm(n_width_nm);
  pb->set_n_length_nm(n_length_nm);

  pb->set_stacks_left(stacks_left);
  pb->set_stacks_right(stacks_right);

  if (vertical_tab_pitch_nm) {
    pb->set_vertical_tab_pitch_nm(*vertical_tab_pitch_nm);
  } else {
    pb->clear_vertical_tab_pitch_nm();
  }

  if (vertical_tab_offset_nm) {
    pb->set_vertical_tab_offset_nm(*vertical_tab_offset_nm);
  } else {
    pb->clear_vertical_tab_offset_nm();
  }

  if (poly_pitch_nm) {
    pb->set_poly_pitch_nm(*poly_pitch_nm);
  } else {
    pb->clear_poly_pitch_nm();
  }

  pb->set_draw_nwell(draw_nwell);

  if (p_tab_position) {
    pb->set_p_tab_position(
        geometry::CompassToProtoCompassDirection(*p_tab_position));
  } else {
    pb->clear_p_tab_position();
  }

  if (n_tab_position) {
    pb->set_n_tab_position(
        geometry::CompassToProtoCompassDirection(*n_tab_position));
  } else {
    pb->clear_n_tab_position();
  }

  if (min_p_tab_diff_separation_nm) {
    pb->set_min_p_tab_diff_separation_nm(*min_p_tab_diff_separation_nm);
  } else {
    pb->clear_min_p_tab_diff_separation_nm();
  }

  if (min_n_tab_diff_separation_nm) {
    pb->set_min_n_tab_diff_separation_nm(*min_n_tab_diff_separation_nm);
  } else {
    pb->clear_min_n_tab_diff_separation_nm();
  }

  pb->set_tabs_should_avoid_nearest_vias(tabs_should_avoid_nearest_vias);
}

void Sky130TransmissionGate::Parameters::FromProto(
    const proto::parameters::Sky130TransmissionGate &pb) {
  if (pb.has_p_width_nm()) {
    p_width_nm = pb.p_width_nm();
  }

  if (pb.has_p_length_nm()) {
    p_length_nm = pb.p_length_nm();
  }

  if (pb.has_n_width_nm()) {
    n_width_nm = pb.n_width_nm();
  }

  if (pb.has_n_length_nm()) {
    n_length_nm = pb.n_length_nm();
  }

  if (pb.has_n_width_nm()) {
    n_width_nm = pb.n_width_nm();
  }

  if (pb.has_stacks_left()) {
    stacks_left = pb.stacks_left();
  }

  if (pb.has_stacks_right()) {
    stacks_right = pb.stacks_right();
  }

  if (pb.has_vertical_tab_pitch_nm()) {
    vertical_tab_pitch_nm = pb.vertical_tab_pitch_nm();
  } else {
    vertical_tab_pitch_nm.reset();
  }

  if (pb.has_vertical_tab_offset_nm()) {
    vertical_tab_offset_nm = pb.vertical_tab_offset_nm();
  } else {
    vertical_tab_offset_nm.reset();
  }

  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  } else {
    poly_pitch_nm.reset();
  }

  if (pb.has_draw_nwell()) {
    draw_nwell = pb.draw_nwell();
  }

  if (pb.has_p_tab_position()) {
    p_tab_position = geometry::ProtoCompassDirectionToCompass(
        pb.p_tab_position());
  } else {
    p_tab_position.reset();
  }

  if (pb.has_n_tab_position()) {
    n_tab_position = geometry::ProtoCompassDirectionToCompass(
        pb.n_tab_position());
  } else {
    n_tab_position.reset();
  }

  if (pb.has_min_p_tab_diff_separation_nm()) {
    min_p_tab_diff_separation_nm = pb.min_p_tab_diff_separation_nm();
  } else {
    min_p_tab_diff_separation_nm.reset();
  }

  if (pb.has_min_n_tab_diff_separation_nm()) {
    min_n_tab_diff_separation_nm = pb.min_n_tab_diff_separation_nm();
  } else {
    min_n_tab_diff_separation_nm.reset();
  }

  if (pb.has_tabs_should_avoid_nearest_vias()) {
    tabs_should_avoid_nearest_vias = pb.tabs_should_avoid_nearest_vias();
  }
}


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

geometry::Polygon *Sky130TransmissionGate::AddPolyTab(
    const Sky130SimpleTransistor &fet_generator,
    const geometry::Compass &position,
    int64_t connector_height,
    Layout *layout) {
  int64_t tab_height = PolyTabHeight(fet_generator);
  int64_t tab_width = PolyTabWidth(fet_generator);

  geometry::Point poly_ll = fet_generator.PolyLowerLeft();
  geometry::Point poly_ur = fet_generator.PolyUpperRight();

  layout->SavePoint(absl::StrCat(fet_generator.name(), ".", "poly_centre"),
                    (poly_ll + poly_ur) / 2);

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

  geometry::Rectangle via = fet_generator.PolyContactingVia(
      geometry::Point::MidpointOf(tab_ll, tab_ur));

  ScopedLayer layer(layout, fet_generator.PolyLayer());
  if (connector_height <= 0) {
    layout->SavePoint(absl::StrCat(fet_generator.name(), ".", "poly_tab_ll"),
                      via.lower_left());
    layout->SavePoint(absl::StrCat(fet_generator.name(), ".", "poly_tab_ur"),
                      via.upper_right());

    std::vector<geometry::Point> vertices = {
      tab_ll,
      geometry::Point(tab_ll.x(), tab_ur.y()),
      tab_ur,
      geometry::Point(tab_ur.x(), tab_ll.y())
    };
    return layout->AddPolygon(geometry::Polygon(vertices));
  }

  // There is some connector height, so we have to add the connector bits to the
  // polygon:
  geometry::Vector translator = {0, connector_height};

  layout->SavePoint(
      absl::StrCat(fet_generator.name(), ".", "poly_tab_ll"),
      via.lower_left() + translator);
  layout->SavePoint(
      absl::StrCat(fet_generator.name(), ".", "poly_tab_ur"),
      via.upper_right() + translator);

  switch (position) {
    case geometry::Compass::UPPER_LEFT:
    case geometry::Compass::UPPER_RIGHT:
    case geometry::Compass::UPPER: {
      std::vector<geometry::Point> vertices = {
        geometry::Point(poly_ll.x(), poly_ur.y()),
        geometry::Point(poly_ll.x(), poly_ur.y()) + translator,
        tab_ll + translator,
        geometry::Point(tab_ll.x(), tab_ur.y()) + translator,
        tab_ur + translator,
        geometry::Point(tab_ur.x(), tab_ll.y()) + translator,
        poly_ur + translator,
        poly_ur
      };
      return layout->AddPolygon(geometry::Polygon(vertices));
    }
    case geometry::Compass::LOWER_LEFT:
    case geometry::Compass::LOWER_RIGHT:
    case geometry::Compass::LOWER: {
      std::vector<geometry::Point> vertices = {
        poly_ll,
        poly_ll - translator,
        geometry::Point(tab_ll.x(), tab_ur.y()) - translator,
        tab_ll - translator,
        geometry::Point(tab_ur.x(), tab_ll.y()) - translator,
        tab_ur - translator,
        geometry::Point(poly_ur.x(), poly_ll.y()) - translator,
        geometry::Point(poly_ur.x(), poly_ll.y())
      };
      return layout->AddPolygon(geometry::Polygon(vertices));
    }
    default:
      LOG(FATAL) << "Unsupported poly tab position: " << position;
  }
  return nullptr;
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

int64_t Sky130TransmissionGate::FigureTopPadding(
    int64_t pmos_poly_top_y) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(pfet_generator_->PolyLayer());
  int64_t minimum = poly_rules.min_separation / 2;

  if (!parameters_.vertical_tab_pitch_nm) {
    return minimum;
  }

  int64_t pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t max_y = (((pmos_poly_top_y + minimum) / pitch) + 1) * pitch;
  return max_y - pmos_poly_top_y;
}

int64_t Sky130TransmissionGate::FigureBottomPadding() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(nfet_generator_->PolyLayer());
  int64_t minimum = poly_rules.min_separation / 2;
  if (!parameters_.vertical_tab_pitch_nm || !NMOSHasLowerTab()) {
    return minimum;
  }
  int64_t pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t offset = db.ToInternalUnits(
      parameters_.vertical_tab_offset_nm.value_or(0)) % pitch;

  int64_t tab_height = NMOSPolyTabHeight();

  int64_t padding = offset - tab_height / 2;
  if (padding < minimum) {
    padding += pitch;
  }
  return padding;
}

int64_t Sky130TransmissionGate::NextYOnGrid(int64_t current_y) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  if (!parameters_.vertical_tab_pitch_nm) {
    return current_y;
  }
  int64_t pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t offset = db.ToInternalUnits(
      parameters_.vertical_tab_offset_nm.value_or(0)) % pitch;

  // We want floor() behaviour:
  int64_t quotient = (current_y - offset) / pitch;
  return (quotient + 1) * pitch + offset;
}

// Building the cell up from y = 0 and assuming the NMOS transistor
// construction (including the poly) gets up to current_y, find the the
// necessary cmos_gap so that when the PMOS construction is added (including
// any tab placement, minimum cell height) all constraints are honoured.
//
// TODO(aryap): This assumes that the PMOS to diff separation rule is the same
// as the PMOS to diff minimum enclosure rule. That's probably not true in all
// PDKs.
int64_t Sky130TransmissionGate::FigureCMOSGap(int64_t current_y) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  int64_t nwell_ndiff_separation = db.Rules(
      "nwell.drawing", nfet_generator_->DiffLayer()).min_separation;
  int64_t nwell_margin = db.Rules(
      "nwell.drawing", pfet_generator_->DiffLayer()).min_enclosure;

  // This is the 'required' y.
  //
  //              |    |  diff
  //         -----|    |-----
  //           ^  |    |     ^ poly overhang
  //     min   |  +----+     V
  //     diff. |
  //     sep.  |  +----+     ^ poly overhang
  //           v  |    |     v
  //         -----|    |-----
  //              |    |  diff
  int64_t min_diff_separation = nwell_ndiff_separation + nwell_margin;
  int64_t min_y = 
      std::max(
          current_y + db.Rules(nfet_generator_->PolyLayer()).min_separation,
          current_y - static_cast<int64_t>(nfet_generator_->PolyOverhangTop()) +
              min_diff_separation -
              static_cast<int64_t>(pfet_generator_->PolyOverhangBottom()));

  // If the cell has a minimum height, the minimum y position must be adjusted
  // so that, after adding the PMOS transistor and tab (if any), the cell at
  // least meets that height. We determine the actual min_y when the tab needs
  // to align to the grid below, but we do not need to consider it here since
  // all we need is a minimum y value to meet the constraint. (Any adjustment
  // to align the tab to the grid will have to increase the minimum y.)
  if (parameters_.min_cell_height_nm) {
    min_y = std::max(
        min_y,
        db.ToInternalUnits(*parameters_.min_cell_height_nm) - (
            PMOSPolyHeight() + (PMOSHasAnyTab() ? PMOSPolyTabHeight() : 0)));
  }

  // If the PMOS transistor has a lower-side tab, we might need to add a gap
  // here to get it onto the grid:
  if (PMOSHasLowerTab()) {
    int64_t tab_height = PMOSPolyTabHeight();
    int64_t next_y = NextYOnGrid(min_y + tab_height / 2);
    return next_y - tab_height / 2 - current_y;
  }

  return min_y - current_y;
}

int64_t Sky130TransmissionGate::FigureNMOSLowerTabConnectorHeight() const {
  if (parameters_.tabs_should_avoid_nearest_vias) {
    int64_t extra_necessary =
        nfet_generator_->FigurePolyDiffExtension(NMOSPolyTabHeight() / 2) -
        NMOSPolyOverhangBottom();
    return std::max(extra_necessary - NMOSPolyOverhangBottom(), 0L);
  }
  return 0;
}

// Only called if the NMOS has an upper tab, which means we need to find the
// next on-grid position above nmos_poly_top_y where the tab can fit:
int64_t Sky130TransmissionGate::FigureNMOSUpperTabConnectorHeight(
    int64_t nmos_poly_top_y) const {
  int64_t tab_height = NMOSPolyTabHeight();
  int64_t tab_centre = nmos_poly_top_y + tab_height / 2;

  if (parameters_.tabs_should_avoid_nearest_vias) {
    int64_t extra_necessary =
        nfet_generator_->FigurePolyDiffExtension(tab_height / 2) -
        NMOSPolyOverhangTop();
    if (extra_necessary > 0) {
      tab_centre += extra_necessary;
    }
  }

  int64_t next_on_grid = NextYOnGrid(tab_centre);
  return next_on_grid - tab_centre;
}

int64_t Sky130TransmissionGate::FigurePMOSLowerTabConnectorHeight() const {
  if (parameters_.tabs_should_avoid_nearest_vias) {
    int64_t extra_necessary =
        pfet_generator_->FigurePolyDiffExtension(PMOSPolyTabHeight() / 2) -
        PMOSPolyOverhangBottom();
    return std::max(extra_necessary - PMOSPolyOverhangBottom(), 0L);
  }
  return 0;
}

int64_t Sky130TransmissionGate::FigurePMOSUpperTabConnectorHeight(
    int64_t pmos_poly_top_y) const {
  int64_t tab_height = PMOSPolyTabHeight();
  int64_t tab_centre = pmos_poly_top_y + tab_height / 2;

  if (parameters_.tabs_should_avoid_nearest_vias) {
    int64_t extra_necessary =
        pfet_generator_->FigurePolyDiffExtension(tab_height / 2) -
        PMOSPolyOverhangTop();
    if (extra_necessary > 0) {
      tab_centre += extra_necessary;
    }
  }

  int64_t next_on_grid = NextYOnGrid(tab_centre);
  return next_on_grid - tab_centre;
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

const Sky130TransmissionGate::VerticalSpacings
Sky130TransmissionGate::FigureSpacings() const {
  VerticalSpacings spacings;

  // This just tracks where we expect our y value to end up as we construct
  // upwards. It starts with whatever gap is necessary to put the bottom tab on
  // the grid, if a grid is defined, or at minimum spacing to the cell edge.
  spacings.bottom_padding = FigureBottomPadding();
  int64_t y = spacings.bottom_padding;

  int64_t nmos_align_y = y;
  int64_t nmos_tab_connector_height = 0;
  if (NMOSHasLowerTab()) {
    nmos_tab_connector_height = FigureNMOSLowerTabConnectorHeight();
    y += NMOSPolyTabHeight() + nmos_tab_connector_height;
    nmos_align_y = y;
  } else if (NMOSHasUpperTab()) {
    nmos_tab_connector_height =
        FigureNMOSUpperTabConnectorHeight(y + NMOSPolyHeight());
    y += NMOSPolyTabHeight() + nmos_tab_connector_height;
  }
  spacings.nmos_poly_bottom_y = nmos_align_y;
  spacings.nmos_tab_extension = nmos_tab_connector_height;

  y += NMOSPolyHeight();

  // TODO(aryap): This should also account for the minimum nwell/nsdm/psdm
  // spacing rules!
  int64_t cmos_gap = FigureCMOSGap(y);
  y += cmos_gap;

  int64_t pmos_align_y = y;
  int64_t pmos_tab_connector_height = 0;
  if (PMOSHasLowerTab()) {
    pmos_align_y += PMOSPolyTabHeight();
    y += PMOSPolyTabHeight();

  } else if (PMOSHasUpperTab()) {
    pmos_tab_connector_height =
        FigurePMOSUpperTabConnectorHeight(pmos_align_y + PMOSPolyHeight());
    y += PMOSPolyTabHeight() + pmos_tab_connector_height;
  }
  spacings.pmos_tab_extension = pmos_tab_connector_height;
  spacings.pmos_poly_bottom_y = pmos_align_y;
  y += PMOSPolyHeight();

  int64_t top_padding = FigureTopPadding(y);
  y += top_padding;

  spacings.cell_height = y;
  return spacings;
}

bfg::Layout *Sky130TransmissionGate::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  const auto spacings = FigureSpacings();

  nfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, spacings.nmos_poly_bottom_y});
  pfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, spacings.pmos_poly_bottom_y});

  std::unique_ptr<bfg::Layout> nfet_layout(nfet_generator_->GenerateLayout());
  layout->AddLayout(*nfet_layout, nfet_generator_->name());
  std::unique_ptr<bfg::Layout> pfet_layout(pfet_generator_->GenerateLayout());
  layout->AddLayout(*pfet_layout, pfet_generator_->name());

  if (parameters_.n_tab_position) {
    geometry::Polygon *ntab = AddPolyTab(
        *nfet_generator_, *parameters_.n_tab_position,
        spacings.nmos_tab_extension, layout.get());

    //layout->MakeVia(
    //    nfet_generator_->DiffConnectionLayer(), ntab->centre());
  }
  if (parameters_.p_tab_position) {
    geometry::Polygon *ptab = AddPolyTab(
        *pfet_generator_, *parameters_.p_tab_position,
        spacings.pmos_tab_extension, layout.get());
  }

  geometry::Rectangle pre_well_bounds = layout->GetBoundingBox();

  // TODO(aryap): nwell.drawing has a minimum width that must be considered
  // here. Does it make sense to make the nwell boundary generation a part of
  // the Sky130SimpleTransistor? Otherwise we have to check for min. dimensions
  // everywhere.
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
        {pre_well_bounds.upper_right().x(), spacings.cell_height});
    ScopedLayer layer(layout.get(), "areaid.standardc");
    layout->AddRectangle(tiling_bounds);
    layout->SetTilingBounds(tiling_bounds);
  }

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
