#include "sky130_simple_transistor.h"

#include "../circuit.h"
#include "../layout.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

namespace bfg {
namespace atoms {

bfg::Cell *Sky130SimpleTransistor::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_simple_transistor": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

std::string Sky130SimpleTransistor::DiffLayer() const {
  switch (parameters_.fet_type) {
    case Parameters::FetType::PMOS:
    case Parameters::FetType::PMOS_HVT:
    case Parameters::FetType::PMOS_LVT:
      return "pdiff.drawing";
    case Parameters::FetType::NMOS:
    case Parameters::FetType::NMOS_HVT:
    case Parameters::FetType::NMOS_LVT:
    default:
      return "ndiff.drawing";
  }
}

std::string Sky130SimpleTransistor::DiffConnectionLayer() const {
  switch (parameters_.fet_type) {
    case Parameters::FetType::PMOS:
    case Parameters::FetType::PMOS_HVT:
    case Parameters::FetType::PMOS_LVT:
      return "pcon.drawing";
    case Parameters::FetType::NMOS:
    case Parameters::FetType::NMOS_HVT:
    case Parameters::FetType::NMOS_LVT:
    default:
      return "ncon.drawing";
  }
}

int64_t Sky130SimpleTransistor::GetDiffWing(
    const geometry::Compass &direction) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules("poly.drawing");

  bool stacks = false;
  switch (direction) {
    case geometry::Compass::LEFT:
      stacks = parameters_.stacks_left;
      break;
    case geometry::Compass::RIGHT:
      stacks = parameters_.stacks_right;
      break;
    default:
      LOG(FATAL) << "Unusable compass direction: " << direction;
      return 0;
  }
  if (stacks) {
    return (poly_rules.min_pitch - TransistorWidth()) / 2;
  }

  std::string diff_layer = DiffLayer();
  std::string dcon_layer = DiffConnectionLayer();
  const auto &dcon_rules = db.Rules(dcon_layer);
  const auto &diff_dcon_rules = db.Rules(diff_layer, dcon_layer);
  const auto &poly_dcon_rules = db.Rules("poly.drawing", dcon_layer);
  int64_t via_side = dcon_rules.via_width;

  //      poly    poly
  //      |   |   |
  // +----|   |---|
  // |    |   |   |
  // |    |   |   |
  // +----|   |---|
  //  <---|   |
  //   diff_wing
  int64_t diff_wing = via_side + poly_dcon_rules.min_separation +
      diff_dcon_rules.min_enclosure;
  return diff_wing;
}

// The origin will the the centre of the poly.
bfg::Layout *Sky130SimpleTransistor::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  int64_t x_pos = 0;
  int64_t length = db.ToInternalUnits(parameters_.length_nm);
  int64_t width = db.ToInternalUnits(parameters_.width_nm);

  int64_t y_min = -length / 2;
  int64_t y_max = length / 2;

  layout->SetActiveLayerByName("poly.drawing");
  geometry::PolyLine line = geometry::PolyLine(
      {{x_pos, y_min}, {x_pos, y_max}});
  line.SetWidth(width);
  layout->AddPolyLine(line);

  layout->SetActiveLayerByName(DiffLayer());
  layout->AddRectangle(geometry::Rectangle(
      {x_pos - width / 2 - GetDiffWing(geometry::Compass::LEFT),
       y_min + 100},  // FIXME: this is an encap rule?
      {x_pos + width / 2 + GetDiffWing(geometry::Compass::RIGHT),
       y_max - 100}));  // FIXME: This is that same encap rule
       

  return layout.release();
}

bfg::Circuit *Sky130SimpleTransistor::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
