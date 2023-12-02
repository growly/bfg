#include "sky130_tap.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../circuit/wire.h"
#include "../cell.h"
#include "../layout.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Layer;

bfg::Cell *Sky130Tap::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_tap": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  // TODO(growly): std::move?
  return cell.release();
}

bfg::Circuit *Sky130Tap::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  return circuit.release();
}

namespace {

// We have a span of length L which must accommodate the end enclosure spacings
// A, via widths B, and inter-via spacings C:
//
//                 L
// |------+====+--------+====+------|
//     A     B      C      B     A
//
// s.t. (n - 1)*C + n*B + 2*A <= L
//      (B + C)*n - C <= L - 2*A
//      (B + C)*n <= L - 2*A + C
//      n = floor((L - 2*A +C) / (B + C))
//
// since n must be an integer.
//
// JOKES. Our problem is harder so we simplify this function by setting A=0:
int64_t HowManyViasFitInLiSpan(
    const PhysicalPropertiesDatabase db,
    int64_t span_start,
    int64_t span_end) {
  int64_t span = span_end - span_start;
  int64_t via_width = db.Rules("licon.drawing").via_width;
  int64_t via_spacing = db.Rules("licon.drawing").min_separation;

  int64_t num_vias = (span + via_spacing) /
      (via_width + via_spacing);
  return num_vias;
}

}   // namespace

bfg::Layout *Sky130Tap::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  int64_t cell_width = db.ToInternalUnits(parameters_.width_nm);
  int64_t cell_height = db.ToInternalUnits(parameters_.height_nm);

  int64_t metal_width = db.ToInternalUnits(parameters_.metal_1_width_nm);
  int64_t li_width = db.ToInternalUnits(parameters_.li_width_nm);

  int64_t li_min_separation = db.Rules("li.drawing").min_separation;
  int64_t li_arm_width = cell_width - li_min_separation;

  // From measuring a few HD, LP, HS examples.
  int64_t li_to_npsdm = db.ToInternalUnits(105);

  // I read these off the HD tapvpwrvgnd_1 cell. TODO(aryap): What
  // reasoning/rule dictates these?
  int64_t li_ground_arm_length = db.ToInternalUnits(725);
  int64_t li_power_arm_length = db.ToInternalUnits(1165);
  int64_t continuity_sdm_height = db.ToInternalUnits(380);

  int64_t via_width = db.Rules("licon.drawing").via_width;
  int64_t via_spacing = db.Rules("licon.drawing").min_separation;
  int64_t via_enclosure = db.Rules("li.drawing", "licon.drawing").min_enclosure;

  const auto &tap_licon_rules = db.Rules("tap.drawing", "licon.drawing");
  const auto &tap_nsdm_rules = db.Rules("tap.drawing", "nsdm.drawing");
  const auto &tap_psdm_rules = db.Rules("tap.drawing", "psdm.drawing");
  const auto &nsdm_nwell_rules = db.Rules("nsdm.drawing", "nwell.drawing");
  const auto &tap_nwell_rules = db.Rules("tap.drawing", "nwell.drawing");
  const auto &nwell_rules = db.Rules("nwell.drawing");
  const auto &tap_rules = db.Rules("tap.drawing");

  // areaid.standardc 81/4
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  layout->SetActiveLayerByName("areaid.standardc");
  Rectangle *tiling_bounds = layout->AddRectangle(
      Rectangle(Point(0, 0), cell_width, cell_height));
  layout->SetTilingBounds(*tiling_bounds);

  // Ground rail:
  layout->SetActiveLayerByName("met1.drawing");
  Rectangle *metal_ground_pour = layout->AddRectangle(
      Rectangle({0, -metal_width / 2},
                cell_width,
                metal_width));
  layout->SetActiveLayerByName("li.drawing");
  Rectangle *li_ground_pour = layout->AddRectangle(
      Rectangle({0, -li_width / 2},
                cell_width,
                li_width));
  Rectangle *li_ground_arm = layout->AddRectangle(
      Rectangle({li_min_separation / 2, li_ground_pour->upper_right().y()},
                li_arm_width,
                li_ground_arm_length));
  layout->MakeVia("mcon.drawing", li_ground_pour->centre());
  layout->SetActiveLayerByName("nsdm.drawing");
  Rectangle *lower_nsdm = layout->AddRectangle(
      Rectangle({0, -continuity_sdm_height / 2},
                cell_width,
                continuity_sdm_height));

  int64_t min_length = std::ceil(
      static_cast<double>(tap_rules.min_area) / static_cast<double>(li_width));

  int64_t closest_via_ll_to_ground_y = std::max(
      li_ground_arm->lower_left().y() + via_enclosure,
      lower_nsdm->upper_right().y() + tap_nsdm_rules.min_separation
  );
  int64_t furthest_via_ur_from_ground_y = std::max(
      li_ground_arm->upper_right().y() - via_enclosure,
      closest_via_ll_to_ground_y + min_length
  );

  int64_t num_vias = HowManyViasFitInLiSpan(
      db,
      closest_via_ll_to_ground_y,
      furthest_via_ur_from_ground_y);
  int64_t via_span = std::max(
      num_vias * (via_width + via_spacing),
      min_length
  );

  int64_t y = closest_via_ll_to_ground_y;
  std::vector<Rectangle*> vias;
  for (int64_t i = 0; i < num_vias; ++i) {
    Rectangle *via = layout->MakeVia(
        "licon.drawing", {li_ground_arm->centre().x(), y + via_width /2});
    y += via_width + via_spacing;
    vias.push_back(via);
  }

  // Ground tap:
  layout->SetActiveLayerByName("tap.drawing");
  Rectangle *ground_tap = layout->AddRectangle(
      Rectangle({
              li_ground_arm->centre().x() - via_width / 2 -
                  tap_licon_rules.via_overhang_wide,
              closest_via_ll_to_ground_y - tap_licon_rules.via_overhang
          }, {
              li_ground_arm->centre().x() + via_width / 2 +
                  tap_licon_rules.via_overhang_wide,
              // FIXME(growly): This is fucked?
              std::max(
                  furthest_via_ur_from_ground_y + tap_licon_rules.via_overhang,
                  closest_via_ll_to_ground_y - tap_licon_rules.via_overhang + via_span
              )
          }
      )
  );

  layout->SetActiveLayerByName("psdm.drawing");
  Rectangle *lower_psdm = layout->AddRectangle(
      Rectangle(
          {0, lower_nsdm->upper_right().y()},
          {cell_width,
              ground_tap->upper_right().y() + tap_psdm_rules.min_enclosure}));
           
  // Power rail:
  layout->SetActiveLayerByName("met1.drawing");
  Rectangle *metal_power_pour = layout->AddRectangle(
      Rectangle({0, cell_height - metal_width / 2},
                cell_width,
                metal_width));
  layout->SetActiveLayerByName("li.drawing");
  Rectangle *li_power_pour = layout->AddRectangle(
      Rectangle({0, cell_height - li_width / 2},
                cell_width,
                li_width));
  Rectangle *li_power_arm = layout->AddRectangle(
      Rectangle({li_min_separation / 2,
                    li_power_pour->lower_left().y() - li_power_arm_length},
                li_arm_width,
                li_power_arm_length));
  layout->MakeVia("mcon.drawing", li_power_pour->centre());
  layout->SetActiveLayerByName("psdm.drawing");
  Rectangle *upper_psdm = layout->AddRectangle(
      Rectangle({0, cell_height - continuity_sdm_height / 2},
                cell_width,
                continuity_sdm_height));

  int64_t closest_via_ur_to_power_y = std::min(
      li_power_arm->upper_right().y() - via_enclosure,
      upper_psdm->lower_left().y() - tap_psdm_rules.min_separation
  );
  int64_t furthest_via_ll_from_power_y =
      li_power_arm->lower_left().y() + via_enclosure;

  num_vias = HowManyViasFitInLiSpan(
      db,
      furthest_via_ll_from_power_y,
      closest_via_ur_to_power_y);
  via_span = num_vias * (via_width + via_spacing);

  y = furthest_via_ll_from_power_y;
  vias.clear();
  for (int64_t i = 0; i < num_vias; ++i) {
    Rectangle *via = layout->MakeVia(
        "licon.drawing", {li_power_arm->centre().x(), y + via_width / 2});
    y += via_width + via_spacing;
    vias.push_back(via);
  }

  // Power tap;
  layout->SetActiveLayerByName("tap.drawing");
  Rectangle *power_tap = layout->AddRectangle(
      Rectangle({
          vias.front()->lower_left().x() - tap_licon_rules.via_overhang_wide,
          vias.front()->lower_left().y() - tap_licon_rules.via_overhang}, {
          vias.back()->upper_right().x() + tap_licon_rules.via_overhang_wide,
          vias.back()->upper_right().y() + tap_licon_rules.via_overhang}));

  layout->SetActiveLayerByName("nsdm.drawing");
  Rectangle *upper_nsdm = layout->AddRectangle(
      Rectangle(
          {0, power_tap->lower_left().y() - tap_nsdm_rules.min_enclosure},
          {cell_width, upper_psdm->lower_left().y()}));

  // The n-well.
  layout->SetActiveLayerByName("nwell.drawing");

  // There is actually a pretty annoying minimum-width requirement on the N-well
  // that we have to deal with as well:
  int64_t nwell_width = std::max(
      static_cast<int64_t>(power_tap->Width()) +
          2 * tap_nwell_rules.min_enclosure,
      nwell_rules.min_width
  );

  Point nwell_ll = {
      power_tap->centre().x() - nwell_width / 2,
      power_tap->lower_left().y() - tap_nwell_rules.min_enclosure
  };
  Point nwell_ur = {
      power_tap->centre().x() + nwell_width / 2,
      std::max(
          power_tap->upper_right().y() + tap_nwell_rules.min_enclosure,
          cell_height)
  };
  Rectangle *nwell = layout->AddRectangle(Rectangle(nwell_ll, nwell_ur));

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
