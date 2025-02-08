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
  };

  static const std::vector<std::string> kBottomRowKeys = {
    "NE", "EW", "NS", "ES", "NW", "SW"};
  int64_t bottom_row_y_max = std::transform(
      kBottomRowKeys.begin(), kBottomRowKeys.end(), [](int64_t existing, std::string key) {
      return std::max(existing, poly_specs[key].length);
      });
  // I want to do this functionally but std::transform is awkward.
  for (const auto &key : kBottomRowKeys) {
    bottom_row_y_max = std::max(bottom_row_y_max, poly_specs[key].length);
  }
  LOG(INFO) << "bottom_row_y_max = " << bottom_row_y_max;

  for (const auto &entry : poly_specs) {
    const PolyDimensions &dimensions = entry.second;
    geometry::PolyLine line = geometry::PolyLine(
        {{dimensions.x, y_min}, {dimensions.x, y_min + dimensions.length}});
    line.SetWidth(dimensions.width);
    layout->AddPolyLine(line);
  }

  return layout.release();
}

bfg::Circuit *Sky130SwitchComplex::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
