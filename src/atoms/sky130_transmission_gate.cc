#include "sky130_transmission_gate.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../utility.h"
#include "../modulo.h"
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

  if (nmos_ll_vertical_offset_nm) {
    pb->set_nmos_ll_vertical_offset_nm(*nmos_ll_vertical_offset_nm);
  }

  if (nmos_ll_vertical_pitch_nm) {
    pb->set_nmos_ll_vertical_pitch_nm(*nmos_ll_vertical_pitch_nm);
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

  if (min_poly_boundary_separation_nm) {
    pb->set_min_poly_boundary_separation_nm(*min_poly_boundary_separation_nm);
  } else {
    pb->clear_min_poly_boundary_separation_nm();
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
  }

  if (pb.has_vertical_tab_offset_nm()) {
    vertical_tab_offset_nm = pb.vertical_tab_offset_nm();
  }

  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  }

  if (pb.has_draw_nwell()) {
    draw_nwell = pb.draw_nwell();
  }

  if (pb.has_p_tab_position()) {
    p_tab_position = geometry::ProtoCompassDirectionToCompass(
        pb.p_tab_position());
  }

  if (pb.has_n_tab_position()) {
    n_tab_position = geometry::ProtoCompassDirectionToCompass(
        pb.n_tab_position());
  }

  if (pb.has_nmos_ll_vertical_offset_nm()) {
    nmos_ll_vertical_offset_nm = pb.nmos_ll_vertical_offset_nm();
  }

  if (pb.has_nmos_ll_vertical_pitch_nm()) {
    nmos_ll_vertical_pitch_nm = pb.nmos_ll_vertical_pitch_nm();
  }

  if (pb.has_min_p_tab_diff_separation_nm()) {
    min_p_tab_diff_separation_nm = pb.min_p_tab_diff_separation_nm();
  }

  if (pb.has_min_n_tab_diff_separation_nm()) {
    min_n_tab_diff_separation_nm = pb.min_n_tab_diff_separation_nm();
  }

  if (pb.has_min_poly_boundary_separation_nm()) {
    min_poly_boundary_separation_nm = pb.min_poly_boundary_separation_nm();
  }

  if (pb.has_tabs_should_avoid_nearest_vias()) {
    tabs_should_avoid_nearest_vias = pb.tabs_should_avoid_nearest_vias();
  }
}

// TODO(aryap): Figure out why using DiffLayer and DiffConnectionLayer by
// accident here didn't trigger DRC errors. Very odd. Can we just use the
// smaller size encaps?
int64_t Sky130TransmissionGate::RegularlyOrientedPolyTabHeight(
    const Sky130SimpleTransistor &fet_generator) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  const auto &polycon_rules = db.Rules(fet_generator.PolyConnectionLayer());
  const auto &poly_polycon_rules = db.Rules(
      fet_generator.PolyLayer(), fet_generator.PolyConnectionLayer());
  int64_t via_height = polycon_rules.via_height;
  return via_height + 2 * poly_polycon_rules.via_overhang_wide;
}

int64_t Sky130TransmissionGate::RegularlyOrientedPolyTabWidth(
    const Sky130SimpleTransistor &fet_generator) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &polycon_rules = db.Rules(fet_generator.PolyConnectionLayer());

  const auto &poly_polycon_rules = db.Rules(
      fet_generator.PolyLayer(), fet_generator.PolyConnectionLayer());
  int64_t via_width = polycon_rules.via_width;
  return via_width + 2 * poly_polycon_rules.via_overhang;
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

  std::vector<geometry::Point> vertices;
  geometry::Point final_tab_ll;
  geometry::Point final_tab_ur;
  switch (position) {
    case geometry::Compass::UPPER_LEFT:   // Fallthrough intended.
    case geometry::Compass::UPPER_RIGHT:  // Fallthrough intended.
    case geometry::Compass::UPPER: {
      vertices = {
        geometry::Point(poly_ll.x(), poly_ur.y()),
        geometry::Point(poly_ll.x(), poly_ur.y()) + translator,
        tab_ll + translator,
        geometry::Point(tab_ll.x(), tab_ur.y()) + translator,
        tab_ur + translator,
        geometry::Point(tab_ur.x(), tab_ll.y()) + translator,
        poly_ur + translator,
        poly_ur
      };
      final_tab_ll = tab_ll + translator;
      final_tab_ur = tab_ur + translator;
      break;
    }
    case geometry::Compass::LOWER_LEFT:   // Fallthrough intended.
    case geometry::Compass::LOWER_RIGHT:  // Fallthrough intended.
    case geometry::Compass::LOWER: {
       vertices = {
        poly_ll,
        poly_ll - translator,
        geometry::Point(tab_ll.x(), tab_ur.y()) - translator,
        tab_ll - translator,
        geometry::Point(tab_ur.x(), tab_ll.y()) - translator,
        tab_ur - translator,
        geometry::Point(poly_ur.x(), poly_ll.y()) - translator,
        geometry::Point(poly_ur.x(), poly_ll.y())
      };
      final_tab_ll = tab_ll - translator;
      final_tab_ur = tab_ur - translator;
      break;
    }
    default:
      LOG(FATAL) << "Unsupported poly tab position: " << position;
  }

  layout->SavePoint(
      absl::StrCat(fet_generator.name(), ".", "poly_tab_ll"), final_tab_ll);
  layout->SavePoint(
      absl::StrCat(fet_generator.name(), ".", "poly_tab_ur"), final_tab_ur);

  return layout->AddPolygon(geometry::Polygon(vertices));
}

bfg::Cell *Sky130TransmissionGate::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_transmission_gate": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

// In higher-level tiles, the bfg::Cell for child elements is generated and then
// the layout() and circuit() separately used in instantiating the circuit.
// Here, the child components are mostly pre-configured, so we can independently
// generate a circuit and layout that are concordant with one another.
bfg::Circuit *Sky130TransmissionGate::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  // The transmission gate has 6 ports:
  //
  //           S_B
  //            |
  //           _o_
  //           ---
  //          |   |
  //     IN --+   +-- OUT
  //          |   |
  //   source  ---  drain
  //           --- gate
  //            |
  //            S
  //
  // There is 1 substrate connection per fet (not shown).

 
  circuit::Wire s = circuit->AddSignal("S");
  circuit::Wire s_b = circuit->AddSignal("S_B");
  circuit::Wire in = circuit->AddSignal("IN");
  circuit::Wire out = circuit->AddSignal("OUT");
  // p-substrate.
  circuit::Wire vpb = circuit->AddSignal("VPB");
  // n-substrate.
  circuit::Wire vnb = circuit->AddSignal("VNB");

  circuit->AddPort(s);
  circuit->AddPort(s_b);
  circuit->AddPort(in);
  circuit->AddPort(out);
  circuit->AddPort(vpb);
  circuit->AddPort(vnb);

  std::unique_ptr<bfg::Circuit> pfet_circuit(
      pfet_generator_->GenerateCircuit());
  std::unique_ptr<bfg::Circuit> nfet_circuit(
      nfet_generator_->GenerateCircuit());

  circuit::Instance *pfet = circuit->AddInstance("pfet", pfet_circuit.get());

  pfet->Connect({
      {pfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::SOURCE), in},
      {pfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::DRAIN), out},
      {pfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::GATE), s_b},
      {pfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::SUBSTRATE), vpb}});

  circuit::Instance *nfet = circuit->AddInstance("nfet", nfet_circuit.get());
  nfet->Connect({
      {nfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::SOURCE), in},
      {nfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::DRAIN), out},
      {nfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::GATE), s},
      {nfet_generator_->TerminalPortName(
          Sky130SimpleTransistor::Terminal::SUBSTRATE), vnb}});
 
  // Flatten the circuit so we don't need to keep the pfet_circuit and
  // nfet_circuit objects around anymore.
  circuit->Flatten();

  return circuit.release();
}

int64_t Sky130TransmissionGate::MinPolyBoundarySeparation() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(pfet_generator_->PolyLayer());
  int64_t minimum = poly_rules.min_separation / 2;
  if (parameters_.min_poly_boundary_separation_nm) {
    minimum = std::max(
        minimum,
        db.ToInternalUnits(*parameters_.min_poly_boundary_separation_nm));
  }
  return minimum;
}

int64_t Sky130TransmissionGate::FigureTopPadding(int64_t pmos_poly_top_y)
    const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t minimum = MinPolyBoundarySeparation();

  if (!parameters_.vertical_tab_pitch_nm) {
    return minimum;
  }

  int64_t pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t max_y = Utility::NextMultiple(pmos_poly_top_y + minimum, pitch);
  return max_y - pmos_poly_top_y;
}

int64_t Sky130TransmissionGate::FigureBottomPadding() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(nfet_generator_->PolyLayer());
  int64_t minimum = poly_rules.min_separation / 2;

  if (parameters_.min_poly_boundary_separation_nm) {
    minimum = std::max( 
        minimum,
        db.ToInternalUnits(*parameters_.min_poly_boundary_separation_nm));
  }
  if (!NMOSHasLowerTab()) {
    // There is no lower tab, so we adjust the spacing to place the NMOS
    // lower-left point on the grid, if required:
    int64_t poly_overhang = NMOSPolyOverhangBottom();
    // If not rqeuired, this will just return the function argument:
    int64_t desired_ll_y = NextYOnNMOSLowerLeftGrid(minimum + poly_overhang);
    minimum = desired_ll_y - poly_overhang;
  }

  if (!parameters_.vertical_tab_pitch_nm || !NMOSHasLowerTab()) {
    return minimum;
  }
  int64_t tab_pitch = db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
  int64_t tab_offset = db.ToInternalUnits(
      parameters_.vertical_tab_offset_nm.value_or(0));

  int64_t tab_height = NMOSPolyTabHeight();

  int64_t padding = tab_offset - tab_height / 2;
  if (padding < minimum) {
    padding += tab_pitch;
  }
  return padding;
}

std::optional<int64_t> Sky130TransmissionGate::TabPitch() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  if (!parameters_.vertical_tab_pitch_nm) {
    return std::nullopt;
  }
  return db.ToInternalUnits(*parameters_.vertical_tab_pitch_nm);
}

int64_t Sky130TransmissionGate::NextYOnTabGrid(int64_t current_y) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  auto tab_pitch = TabPitch();
  if (!tab_pitch) {
    return current_y;
  }
  int64_t offset = db.ToInternalUnits(
      parameters_.vertical_tab_offset_nm.value_or(0));

  return Utility::NextMultiple(current_y - offset, *tab_pitch) + offset;
}

int64_t Sky130TransmissionGate::NextYOnNMOSLowerLeftGrid(int64_t current_y)
    const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  if (!parameters_.nmos_ll_vertical_pitch_nm ||
      *parameters_.nmos_ll_vertical_pitch_nm == 0) {
    return current_y;
  }
  int64_t tab_pitch = db.ToInternalUnits(
      *parameters_.nmos_ll_vertical_pitch_nm);
  int64_t offset = db.ToInternalUnits(
      parameters_.nmos_ll_vertical_offset_nm.value_or(0));

  return Utility::NextMultiple(current_y - offset, tab_pitch) + offset;
}

// Building the cell up from y = 0 and assuming the NMOS transistor
// construction (including the poly) gets up to current_y, find the the
// necessary cmos_gap so that when the PMOS construction is added (including
// any tab placement, metal channels, minimum cell height) all constraints are
// honoured.
//
// TODO(aryap): This assumes that the PMOS to diff separation rule is the same
// as the PMOS to diff minimum enclosure rule. That's probably not true in all
// PDKs.
int64_t Sky130TransmissionGate::FigureCMOSGap(
    int64_t nmos_centre_y, int64_t current_y) const {
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
  //           |  +----+     v
  //     min   |          ^
  //     diff. | CMOS gap v
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

  // The user can also specify a minimum separation distance between the two
  // furthest vias on N- and PMOS diffs.
  //
  //              |    |
  //         +----|    |----+
  //         | ^  |    |  diff
  //         | |  |    |    |
  //         +-|--|    |----+
  //     min   |  |    |     ^ poly overhang
  // furthest  |  +----+     v
  //     via   |
  //     sep.  |  +----+     ^ poly overhang
  //           |  |    |     v
  //         +-|--|    |----+
  //         | |  |    |    |
  //         | v  |    |  diff
  //         +----|    |----+
  //
  // We calculate it as follows. Given the current estimate of the gap, figure
  // where the two vias would end up. The Sky130SimpleTransistor will report
  // their positions as configured, relative to the origin_, which the centre
  // of the transistor. The difference between their separation
  // and the desired separation is added to the minimum gap requirement.
  //
  // (Unfortunately we also have to contend with the fact that the PMOS might
  // need to be offset by a tab below it. We have to do a similar thing for the
  // NMOS side.)
  if (parameters_.min_furthest_via_distance_nm) {
    int64_t max_via_y = min_y + PMOSPolyHeight() / 2 +
        pfet_generator_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).y() +
        (PMOSHasLowerTab() ?
            FigurePMOSLowerTabConnectorHeight() + PMOSPolyTabHeight() : 0);
    // Any tab below the NMOS generator origin will offset the calculated Via
    // position:
    int64_t min_via_y = nmos_centre_y +
        nfet_generator_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER).y();

    int64_t required = db.ToInternalUnits(
        *parameters_.min_furthest_via_distance_nm);
    int64_t diff = required - (max_via_y - min_via_y);
    if (diff > 0) {
      min_y += diff;
    }
  }

  // This is honky. Half-baked. Smooth-brained. No bueno. But it works, just.
  //
  // If the cell has a minimum height, the minimum y position must be adjusted
  // so that, after adding the PMOS transistor and tab (if any), the cell at
  // least meets that height. We determine the actual min_y when the tab needs
  // to align to the grid below, but we do not need to consider it here since
  // all we need is a minimum y value to meet the constraint. (Any adjustment
  // to align the tab to the grid will have to increase the minimum y.) Note
  // that this means we will almost certainly be making the cell too tall in
  // that case.
  //
  // Ok but also, to figure the CMOS gap when there is a poly pitch to which
  // the tabs must accord, first assume that the poly connector is maximally
  // sized to accommodate the pitch (pitch - 1), then figure the minimum y
  // value needed to meet the needed cell height. Then use that minimum y to
  // figure the upper tab connect height, and use that value for the top
  // section height, determining the actual minimum y value.
  //
  // TODO(aryap): One path through this works, but I suspect I have added a lot
  // of bugs because I have not thought this through.
  if (parameters_.min_cell_height_nm) {
    int64_t min_cell_height = db.ToInternalUnits(*parameters_.min_cell_height_nm);
    int64_t top_section_height = PMOSPolyHeight() + MinPolyBoundarySeparation();
    int64_t channel_spacing = parameters_.allow_metal_channel_top ?
        pfet_generator_->RequiredMetalSpacingForChannel() : 0;
    if (PMOSHasLowerTab()) {
      top_section_height += PMOSPolyTabHeight() +
          FigurePMOSLowerTabConnectorHeight() + channel_spacing;
    } else if (PMOSHasUpperTab()) {
      top_section_height += PMOSPolyTabHeight();
      int64_t max_top_section_height = top_section_height + (
          TabPitch().value_or(1) - 1) + channel_spacing;
      int64_t min_min_y = min_cell_height - max_top_section_height;

      // Re-figures the channel spacing, if any.
      top_section_height += FigurePMOSUpperTabConnectorHeight(
          min_min_y + PMOSPolyHeight());
    }
    min_y = std::max(min_y, min_cell_height - top_section_height);
  }

  // If the PMOS transistor has a lower-side tab, we might need to add a gap
  // here to get it onto the grid:
  if (PMOSHasLowerTab()) {
    int64_t tab_height = PMOSPolyTabHeight();
    int64_t next_y = NextYOnTabGrid(min_y + tab_height / 2);
    return next_y - tab_height / 2 - current_y;
  }

  return min_y - current_y;
}

int64_t Sky130TransmissionGate::FigureNMOSLowerTabConnectorHeight(
    int64_t nmos_bottom_tab_top_y) const {
  int64_t minimum = 0;
  if (parameters_.tabs_should_avoid_nearest_vias ||
      parameters_.allow_metal_channel_bottom) {
    int64_t extra_necessary =
        nfet_generator_->FigurePolyDiffExtension(
            NMOSPolyTabHeight() / 2, parameters_.allow_metal_channel_bottom);
    minimum = std::max(extra_necessary - NMOSPolyOverhangBottom(), 0L);
  }

  // The poly tab must be on the bottom, so the space to the lower-left diff
  // point is set by the tab connector height:
  if (parameters_.nmos_ll_vertical_pitch_nm) {
    int64_t poly_overhang = NMOSPolyOverhangBottom();
    int64_t desired_ll_y = NextYOnNMOSLowerLeftGrid(
        nmos_bottom_tab_top_y + poly_overhang);
    int64_t required = desired_ll_y - (nmos_bottom_tab_top_y + poly_overhang);
    minimum = std::max(minimum, required);
  }

  return minimum;
}

// Only called if the NMOS has an upper tab, which means we need to find the
// next on-grid position above nmos_poly_top_y where the tab can fit:
int64_t Sky130TransmissionGate::FigureNMOSUpperTabConnectorHeight(
    int64_t nmos_poly_top_y) const {
  int64_t tab_height = NMOSPolyTabHeight();
  int64_t tab_centre = nmos_poly_top_y + tab_height / 2;

  int64_t extra_extension = 0;
  if (parameters_.tabs_should_avoid_nearest_vias) {
    extra_extension =
        nfet_generator_->FigurePolyDiffExtension(tab_height / 2) -
        NMOSPolyOverhangTop();
  }

  int64_t next_on_grid = NextYOnTabGrid(tab_centre + extra_extension);
  return next_on_grid - tab_centre;
}

int64_t Sky130TransmissionGate::FigurePMOSLowerTabConnectorHeight() const {
  if (parameters_.tabs_should_avoid_nearest_vias) {
    int64_t extra_necessary =
        pfet_generator_->FigurePolyDiffExtension(PMOSPolyTabHeight() / 2);
    return std::max(extra_necessary - PMOSPolyOverhangBottom(), 0L);
  }
  return 0;
}

int64_t Sky130TransmissionGate::FigurePMOSUpperTabConnectorHeight(
    int64_t pmos_poly_top_y) const {
  int64_t tab_height = PMOSPolyTabHeight();
  int64_t tab_centre = pmos_poly_top_y + tab_height / 2;

  int64_t extra_extension = 0;
  if (parameters_.tabs_should_avoid_nearest_vias ||
      parameters_.allow_metal_channel_top) {
    extra_extension = std::max(
        pfet_generator_->FigurePolyDiffExtension(
            tab_height / 2, parameters_.allow_metal_channel_top) -
        PMOSPolyOverhangTop(),
        0L);
  }

  int64_t next_on_grid = NextYOnTabGrid(tab_centre + extra_extension);
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

// This is the world's shittiest constraint solver. We're trying to do
//    min A + B + C + D
//    s.t. A < something
//         B <= something
//         etc.
//
// TODO(aryap): Just use an ILP or some shit.
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
    y += NMOSPolyTabHeight();
    nmos_tab_connector_height = FigureNMOSLowerTabConnectorHeight(y);
    y += nmos_tab_connector_height;
    nmos_align_y = y;
  } else if (NMOSHasUpperTab()) {
    nmos_tab_connector_height =
        FigureNMOSUpperTabConnectorHeight(y + NMOSPolyHeight());
    y += NMOSPolyTabHeight() + nmos_tab_connector_height;
  }
  int64_t nmos_centre_y = nmos_align_y + NMOSPolyHeight() / 2;
  spacings.nmos_poly_bottom_y = nmos_align_y;
  spacings.nmos_tab_extension = nmos_tab_connector_height;

  y += NMOSPolyHeight();

  // TODO(aryap): This should also account for the minimum nwell/nsdm/psdm
  // spacing rules!
  int64_t cmos_gap = FigureCMOSGap(nmos_centre_y, y);
  y += cmos_gap;

  int64_t pmos_align_y = y;
  int64_t pmos_tab_connector_height = 0;
  if (PMOSHasLowerTab()) {
    pmos_tab_connector_height = FigurePMOSLowerTabConnectorHeight();
    pmos_align_y = y;
  } else if (PMOSHasUpperTab()) {
    pmos_tab_connector_height =
        FigurePMOSUpperTabConnectorHeight(pmos_align_y + PMOSPolyHeight());
  }
  y += PMOSPolyTabHeight() + pmos_tab_connector_height;
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

  int64_t pad_left = 0;
  int64_t pad_right = 0;
  if (parameters_.pitch_match_to_boundary) {
    int64_t poly_pitch = db.ToInternalUnits(*parameters_.poly_pitch_nm);
    if (!parameters_.stacks_left) {
      int64_t left_x = pre_well_bounds.lower_left().x();
      // left_x is negative:
      DCHECK(left_x <= 0);
      pad_left = modulo(left_x, poly_pitch);
    }
    if (!parameters_.stacks_right) {
      int64_t right_x = pre_well_bounds.upper_right().x();
      DCHECK(right_x >= 0);
      pad_right = poly_pitch - modulo(right_x, poly_pitch);
    }
  }

  // TODO(aryap): nwell.drawing has a minimum width that must be considered
  // here. Does it make sense to make the nwell boundary generation a part of
  // the Sky130SimpleTransistor? If not we have to check for min. dimensions
  // everywhere.
  if (parameters_.draw_nwell) {
    ScopedLayer layer(layout.get(), "nwell.drawing");
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    geometry::Rectangle nwell = PMOSBounds().WithPadding(
        pad_left + nwell_margin,   // Left.
        nwell_margin,              // Top.
        pad_right + nwell_margin,  // Right.
        nwell_margin);             // Bottom.
    layout->AddRectangle(nwell);
  }

  // Set tiling bounds.
  {
    int64_t min_y = pre_well_bounds.lower_left().y();
    geometry::Rectangle tiling_bounds = geometry::Rectangle(
        {pre_well_bounds.lower_left().x() - pad_left, 0},
        {pre_well_bounds.upper_right().x() + pad_right, spacings.cell_height});
    ScopedLayer layer(layout.get(), "areaid.standardc");
    layout->AddRectangle(tiling_bounds);
    layout->SetTilingBounds(tiling_bounds);
  }

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
