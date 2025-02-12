#include "sky130_switch_complex.h"

#include <numeric>

#include "../circuit.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

#include "sky130_simple_transistor.h"

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
  struct TransistorSpecs {
    int64_t x = 0;
    Sky130SimpleTransistor::Parameters fet_params;
    Sky130SimpleTransistor *fet_generator;
  };

  std::map<std::string, TransistorSpecs> transistor_specs = {
    {"NE", TransistorSpecs{
      .x = x_start,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ne_nfet_width_nm,
        .length_nm = parameters_.ne_nfet_length_nm,
        .stacks_right = true
      }}},
    {"EW", TransistorSpecs{
      .x = x_start + poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ew_nfet_width_nm,
        .length_nm = parameters_.ew_nfet_length_nm,
        .stacks_left = true
      }}},
    {"NS", TransistorSpecs{
      .x = x_start + 3 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ns_nfet_width_nm,
        .length_nm = parameters_.ns_nfet_length_nm,
        .stacks_right = true
      }}},
    {"ES", TransistorSpecs{
      .x = x_start + 4 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.es_nfet_width_nm,
        .length_nm = parameters_.es_nfet_length_nm,
        .stacks_left = true
      }}},
    {"NW", TransistorSpecs{
      .x = x_start + 6 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.nw_nfet_width_nm,
        .length_nm = parameters_.nw_nfet_length_nm,
        .stacks_right = true
      }}},
    {"SW", TransistorSpecs{
      .x = x_start + 7 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.sw_nfet_width_nm,
        .length_nm = parameters_.sw_nfet_length_nm,
        .stacks_left = true
      }}},
    {"NE_B", TransistorSpecs{
      .x = x_start,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ne_pfet_width_nm,
        .length_nm = parameters_.ne_pfet_length_nm,
        .stacks_right = true
      }}},
    {"EW_B", TransistorSpecs{
      .x = x_start + poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ew_pfet_width_nm,
        .length_nm = parameters_.ew_pfet_length_nm,
        .stacks_left = true
      }}},
    {"NS_B", TransistorSpecs{
      .x = x_start + 3 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ns_pfet_width_nm,
        .length_nm = parameters_.ns_pfet_length_nm,
        .stacks_right = true
      }}},
    {"ES_B", TransistorSpecs{
      .x = x_start + 4 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.es_pfet_width_nm,
        .length_nm = parameters_.es_pfet_length_nm,
        .stacks_left = true
      }}},
    {"NW_B", TransistorSpecs{
      .x = x_start + 6 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.nw_pfet_width_nm,
        .length_nm = parameters_.nw_pfet_length_nm,
        .stacks_right = true
      }}},
    {"SW_B", TransistorSpecs{
      .x = x_start + 7 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.sw_pfet_width_nm,
        .length_nm = parameters_.sw_pfet_length_nm,
        .stacks_left = true
      }}},
  };

  static const std::vector<std::string> kNfetKeys = {
      "NE", "EW", "NS", "ES", "NW", "SW"};
  static const std::vector<std::string> kPfetKeys = {
      "NE_B", "EW_B", "NS_B", "ES_B", "NW_B", "SW_B"};

  for (auto &entry : transistor_specs) {
    TransistorSpecs &specs = entry.second;
    Sky130SimpleTransistor *fet_generator = new Sky130SimpleTransistor(
        specs.fet_params, design_db_);
    specs.fet_generator = fet_generator;
  }

  // Get the max length of all the bottom- and top-row TransistorSpecs.
  //
  // NOTE: This is more lines of code than a simple for loop to do the same
  // thing.
  uint64_t bottom_row_length_max = std::accumulate(
      kNfetKeys.begin(), kNfetKeys.end(),
      0, [&](uint64_t existing, const std::string &key) {
        return std::max(
            existing, transistor_specs[key].fet_generator->PolyHeight());
      });

  uint64_t top_row_length_max = std::accumulate(
      kPfetKeys.begin(), kPfetKeys.end(),
      0, [&](uint64_t existing, const std::string &key) {
        return std::max(
            existing, transistor_specs[key].fet_generator->PolyHeight());
      });

  for (const std::string &key : kNfetKeys) {
    const TransistorSpecs &specs = transistor_specs[key];
    Sky130SimpleTransistor *fet_generator = specs.fet_generator;
    fet_generator->AlignTransistorTo(
        Sky130SimpleTransistor::Alignment::POLY_BOTTOM_CENTRE,
        {specs.x, y_min});
  }

  y_min = y_min + bottom_row_length_max + poly_rules.min_separation +
          top_row_length_max;

  for (const std::string &key : kPfetKeys) {
    const TransistorSpecs &specs = transistor_specs[key];
    Sky130SimpleTransistor *fet_generator = specs.fet_generator;
    fet_generator->AlignTransistorTo(
        Sky130SimpleTransistor::Alignment::POLY_TOP_CENTRE,
        {specs.x, y_min});
  }

  for (auto &entry : transistor_specs) {
    TransistorSpecs &specs = entry.second;
    std::unique_ptr<bfg::Layout> transistor_layout(
        specs.fet_generator->GenerateLayout());
    layout->AddLayout(*transistor_layout);
    delete specs.fet_generator;
    specs.fet_generator = nullptr;
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
