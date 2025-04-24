#include "sky130_transmission_gate.h"

#include "../layout.h"
#include "../scoped_layer.h"
#include "sky130_simple_transistor.h"

namespace bfg {
namespace atoms {

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

  const auto &poly_rules = db.Rules("poly.drawing");

  int64_t n_height = nfet_generator_->PolyHeight();
  int64_t p_height = pfet_generator_->PolyHeight();

  int64_t anchor_y = parameters_.cell_height_nm ?
      db.ToInternalUnits(*parameters_.cell_height_nm) - p_height :
      n_height + poly_rules.min_separation;

  nfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Alignment::POLY_BOTTOM_CENTRE,
      {0, 0});
  pfet_generator_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Alignment::POLY_BOTTOM_CENTRE,
      {0, anchor_y});

  std::unique_ptr<bfg::Layout> nfet_layout(nfet_generator_->GenerateLayout());
  layout->AddLayout(*nfet_layout);
  std::unique_ptr<bfg::Layout> pfet_layout(pfet_generator_->GenerateLayout());
  layout->AddLayout(*pfet_layout);

  if (parameters_.draw_nwell) {
    ScopedLayer layer(layout.get(), "nwell.drawing");

    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    layout->AddRectangle(PMOSBounds().WithPadding(nwell_margin));
  }

  std::vector<Sky130SimpleTransistor::ViaPosition> positions = {
    Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER,
    Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE,
    Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER,
    //Sky130SimpleTransistor::ViaPosition::POLY_UPPER,
    //Sky130SimpleTransistor::ViaPosition::POLY_MIDDLE,
    //Sky130SimpleTransistor::ViaPosition::POLY_LOWER,
    Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER,
    Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE,
    Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER
  };
  for (size_t i = 0; i < positions.size(); ++i) {
    layout->MakeVia(
        pfet_generator_->DiffConnectionLayer(),
        pfet_generator_->ViaLocation(positions[i]));
    layout->MakeVia(
        nfet_generator_->DiffConnectionLayer(),
        nfet_generator_->ViaLocation(positions[i]));
  }

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
