#include "sky130_switch_complex.h"

#include <numeric>

#include "../circuit.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

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

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules("poly.drawing");
  const auto &poly_pdiff_rules = db.Rules("poly.drawing", "pdiff.drawing");
  const auto &poly_ndiff_rules = db.Rules("poly.drawing", "ndiff.drawing");
  int64_t poly_pdiff_overhang = poly_pdiff_rules.min_enclosure;
  int64_t poly_ndiff_overhang = poly_ndiff_rules.min_enclosure;
  int64_t poly_pitch = poly_rules.min_pitch;

  // "dcon" is shorthand for "pcon" or "ncon"
  const auto &pcon_rules = db.Rules("pcon.drawing");
  int64_t pcon_via_side = pcon_rules.via_width;
  const auto &ncon_rules = db.Rules("ncon.drawing");
  int64_t ncon_via_side = ncon_rules.via_width;

  //      poly    poly
  //      |   |   |
  // +----|   |---|
  // |    |   |   |
  // |    |   |   |
  // +----|   |---|
  //  <---|   |
  //   diff_wing
  //int64_t diff_wing = via_side + poly_dcon_rules.min_separation +
  //    diff_dcon_rules.min_enclosure;

  int64_t x_start = 0;
  int64_t y_min = 0;
  struct PolyDimensions {
    int64_t x = 0;
    int64_t width = 0;
    int64_t length = 0;
  };

  std::map<std::string, PolyDimensions> poly_specs = {
    {"NE", PolyDimensions{
      .x = x_start,
      .width = db.ToInternalUnits(parameters_.ne_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.ne_nfet_width_nm)
           }},
    {"EW", PolyDimensions{
      .x = x_start + poly_pitch,
      .width = db.ToInternalUnits(parameters_.ew_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.ew_nfet_width_nm)
           }},
    {"NS", PolyDimensions{
      .x = x_start + 3 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.ns_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.ns_nfet_width_nm)
           }},
    {"ES", PolyDimensions{
      .x = x_start + 4 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.es_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.es_nfet_width_nm)
           }},
    {"NW", PolyDimensions{
      .x = x_start + 6 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.nw_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.nw_nfet_width_nm)
           }},
    {"SW", PolyDimensions{
      .x = x_start + 7 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.sw_nfet_length_nm),
      .length = 2 * poly_ndiff_overhang + db.ToInternalUnits(
          parameters_.sw_nfet_width_nm)
           }},
    {"NE_B", PolyDimensions{
      .x = x_start,
      .width = db.ToInternalUnits(parameters_.ne_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.ne_pfet_width_nm)
           }},
    {"EW_B", PolyDimensions{
      .x = x_start + poly_pitch,
      .width = db.ToInternalUnits(parameters_.ew_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.ew_pfet_width_nm)
           }},
    {"NS_B", PolyDimensions{
      .x = x_start + 3 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.ns_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.ns_pfet_width_nm)
           }},
    {"ES_B", PolyDimensions{
      .x = x_start + 4 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.es_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.es_pfet_width_nm)
           }},
    {"NW_B", PolyDimensions{
      .x = x_start + 6 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.nw_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.nw_pfet_width_nm)
           }},
    {"SW_B", PolyDimensions{
      .x = x_start + 7 * poly_pitch,
      .width = db.ToInternalUnits(parameters_.sw_pfet_length_nm),
      .length = 2 * poly_pdiff_overhang + db.ToInternalUnits(
          parameters_.sw_pfet_width_nm)
           }},
  };

  static const std::vector<std::string> kNfetKeys = {
      "NE", "EW", "NS", "ES", "NW", "SW"};
  static const std::vector<std::string> kPfetKeys = {
      "NE_B", "EW_B", "NS_B", "ES_B", "NW_B", "SW_B"};
  // Get the max length of all the bottom row PolyDimensions.
  //
  // NOTE: This is more lines than a simple for loop to do the same thing.
  int64_t bottom_row_length_max = std::accumulate(
      kNfetKeys.begin(), kNfetKeys.end(),
      0, [&](int64_t existing, const std::string &key) {
        return std::max(existing, poly_specs[key].length);
      });

  for (const std::string &key : kNfetKeys) {
    const PolyDimensions &dimensions = poly_specs[key];
    geometry::PolyLine line = geometry::PolyLine(
        {{dimensions.x, y_min}, {dimensions.x, y_min + dimensions.length}});
    line.SetWidth(dimensions.width);
    layout->AddPolyLine(line);

    //int64_t diff_ll_x = dimensions.x - dimensions.width / 2 -
  }

  // Shift y_min above the bottom row to add the PMOS FETs.
  y_min = y_min + bottom_row_length_max + poly_rules.min_separation;
  for (const std::string &key : kPfetKeys) {
    const PolyDimensions &dimensions = poly_specs[key];
    geometry::PolyLine line = geometry::PolyLine(
        {{dimensions.x, y_min}, {dimensions.x, y_min + dimensions.length}});
    line.SetWidth(dimensions.width);
    layout->AddPolyLine(line);
  }

  return layout.release();
}

bfg::Circuit *Sky130SwitchComplex::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
