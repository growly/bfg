#include "sky130_simple_transistor.h"

#include "../circuit.h"
#include "../layout.h"
#include "../scoped_layer.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/vector.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

namespace bfg {
namespace atoms {

const std::map<Sky130SimpleTransistor::ViaPosition, std::string>
Sky130SimpleTransistor::kSavedPointNameByViaPosition = {
  {LEFT_DIFF_UPPER, "via_left_diff_upper"},
  {LEFT_DIFF_MIDDLE, "via_left_diff_middle"},
  {LEFT_DIFF_LOWER, "via_left_diff_lower"},
  //{POLY_UPPER, "via_poly_upper"},
  //{POLY_MIDDLE, "via_poly_middle"},
  //{POLY_LOWER, "via_poly_lower"},
  {RIGHT_DIFF_UPPER, "via_right_diff_upper"},
  {RIGHT_DIFF_MIDDLE, "via_right_diff_middle"},
  {RIGHT_DIFF_LOWER, "via_right_diff_lower"}
};

bfg::Cell *Sky130SimpleTransistor::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_simple_transistor": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

void Sky130SimpleTransistor::AlignTransistorPartTo(
    const Landmark &landmark, const geometry::Point &point) {
  // Set the origin, which should be in the middle of the gate, according to the
  // which alignment point we're using and where we're aligning that point to:
  switch (landmark) {
    case POLY_TOP_CENTRE:
      origin_ = point + geometry::Vector(0, poly_y_min_);
      break;
    case POLY_BOTTOM_CENTRE:
      origin_ = point + geometry::Vector(0, poly_y_max_);
      break;
    default:
      LOG(FATAL) << "Unsupported alignment: " << landmark;
  }
  LOG(INFO) << "Origin set so that " << landmark << " is at " << point;
}

geometry::Point Sky130SimpleTransistor::LowerLeft() const {
  // The origin is the centre of the gate (overlap of the poly and diff).
  return geometry::Point(
      origin_.x() - (
          DiffWing(geometry::Compass::LEFT) + TransistorLength() / 2),
      origin_.y() + poly_y_min_);
}

geometry::Point Sky130SimpleTransistor::ViaLocation (
    const ViaPosition &via_position) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  int64_t poly_height = static_cast<int64_t>(PolyHeight());
  int64_t left_wing = DiffWing(geometry::Compass::LEFT);
  int64_t right_wing = DiffWing(geometry::Compass::RIGHT);
  int64_t poly_width = TransistorLength();
  int64_t diff_height = TransistorWidth();

  const auto &dcon_rules = db.Rules(DiffConnectionLayer());
  int64_t via_width = dcon_rules.via_width;
  int64_t via_height = dcon_rules.via_height;
  int64_t via_centre_to_diff_edge = db.Rules(
      DiffConnectionLayer(), DiffLayer()).min_enclosure +
      via_width / 2;
  int64_t diff_width = left_wing + right_wing + poly_width;

  geometry::Point lower_left = LowerLeft();
  int64_t x_left = lower_left.x() + (
      parameters_.stacks_left ?  0 : via_centre_to_diff_edge);
  int64_t x_right = lower_left.x() + diff_width - (
      parameters_.stacks_right ? 0 : via_centre_to_diff_edge);

  // y-coordinate of the lower-left point on the diff.
  int64_t diff_y_ll = origin_.y() + diff_y_min_;
  int64_t y_lower = diff_y_ll + via_centre_to_diff_edge;
  int64_t y_middle = diff_y_ll + diff_height / 2;
  int64_t y_upper = diff_y_ll + diff_height - via_centre_to_diff_edge;

  switch (via_position) {
    case ViaPosition::LEFT_DIFF_LOWER:
      return {x_left, y_lower};
    case ViaPosition::LEFT_DIFF_MIDDLE:
      return {x_left, y_middle};
    case ViaPosition::LEFT_DIFF_UPPER:
      return {x_left, y_upper};
    case ViaPosition::RIGHT_DIFF_LOWER:
      return {x_right, y_lower};
    case ViaPosition::RIGHT_DIFF_MIDDLE:
      return {x_right, y_middle};
    case ViaPosition::RIGHT_DIFF_UPPER:
      return {x_right, y_upper};
    case ViaPosition::POLY_UPPER:
    case ViaPosition::POLY_MIDDLE:
    case ViaPosition::POLY_LOWER:
    default:
      LOG(FATAL) << "Unsupported ViaPosition: " << via_position;
  }
  return origin_;
}

geometry::Point Sky130SimpleTransistor::PolyTopCentre() const {
  return {origin_.x(), origin_.y() + poly_y_max_};
}

geometry::Point Sky130SimpleTransistor::PolyBottomCentre() const {
  return {origin_.x(), origin_.y() + poly_y_min_};
}

geometry::Point Sky130SimpleTransistor::PolyLowerLeft() const {
  return {origin_.x() - TransistorLength() / 2, origin_.y() + poly_y_min_};

}
geometry::Point Sky130SimpleTransistor::PolyUpperRight() const {
  return {origin_.x() + TransistorLength() / 2, origin_.y() + poly_y_max_};
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

int64_t Sky130SimpleTransistor::DiffWing(
    const geometry::Compass &direction) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules(PolyLayer());

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
    int64_t wing = (poly_rules.min_pitch - TransistorLength()) / 2;
    if (parameters_.stacking_pitch_nm) {
      int64_t stacking_pitch =
          db.ToInternalUnits(*parameters_.stacking_pitch_nm);
      wing = std::max(wing, (stacking_pitch - TransistorLength()) / 2);
    }
    return wing;
  }

  std::string diff_layer = DiffLayer();
  std::string dcon_layer = DiffConnectionLayer();
  const auto &dcon_rules = db.Rules(dcon_layer);
  const auto &diff_dcon_rules = db.Rules(diff_layer, dcon_layer);
  const auto &poly_dcon_rules = db.Rules(PolyLayer(), dcon_layer);
  const auto &poly_diff_rules = db.Rules(PolyLayer(), diff_layer);
  int64_t via_side = dcon_rules.via_width;

  //      poly    poly
  //      |   |   |
  // +----|   |---|
  // |    |   |   |
  // |    |   |   |
  // +----|   |---|
  //  <---|   |
  //   diff_wing
  int64_t diff_wing = std::max(
      via_side + poly_dcon_rules.min_separation + diff_dcon_rules.min_enclosure,
      poly_diff_rules.min_extension);

  return diff_wing;
}

void Sky130SimpleTransistor::ComputeGeometries() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_diff_rules = db.Rules("poly.drawing", DiffLayer());

  // The "width" of the transistor poly is the "length" of the transistor, and
  // vice versa.
  int64_t diff_height = TransistorWidth();

  // These assume an origin at (0, 0).
  poly_y_min_ = -static_cast<int64_t>(PolyHeight()) / 2;
  poly_y_max_ = poly_y_min_ + static_cast<int64_t>(PolyHeight());

  diff_y_min_ = poly_y_min_ + poly_diff_rules.min_enclosure;
  diff_y_max_ = diff_y_min_ + TransistorWidth();
}

const geometry::Rectangle Sky130SimpleTransistor::DiffBounds() const {
  // The "width" of the transistor poly is the "length" of the transistor, and
  // vice versa.
  int64_t poly_width = TransistorLength();
  return geometry::Rectangle(
      {
        origin_.x() - poly_width / 2 - DiffWing(geometry::Compass::LEFT),
        origin_.y() + diff_y_min_
      },
      {
        origin_.x() + poly_width / 2 + DiffWing(geometry::Compass::RIGHT),
        origin_.y() + diff_y_max_
      }
  );
}

uint64_t Sky130SimpleTransistor::PolyHeight() const {
  const auto &poly_diff_rules =
      design_db_->physical_db().Rules("poly.drawing", DiffLayer());
  return TransistorWidth() + 2 * poly_diff_rules.min_enclosure;
}

// The origin will the the centre of the poly.
bfg::Layout *Sky130SimpleTransistor::GenerateLayout(
    geometry::Polygon **poly, geometry::Rectangle **diff) {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  layout->SetActiveLayerByName("poly.drawing");
  geometry::PolyLine line = geometry::PolyLine({
      origin_ + geometry::Point(0, poly_y_min_),
      origin_ + geometry::Point(0, poly_y_max_)
  });
  line.SetWidth(TransistorLength());
  geometry::Polygon *poly_polygon = layout->AddPolyLine(line);

  layout->SetActiveLayerByName(DiffLayer());
  geometry::Rectangle *diff_rectangle = layout->AddRectangle(DiffBounds());

  if (diff) {
    *diff = diff_rectangle;
  }
  if (poly) {
    *poly = poly_polygon;
  }

  // Save points of interest into the Layout.
  layout->SavePoint("poly_top_centre", line.End());
  layout->SavePoint("poly_bottom_centre", line.start());
  for (const auto &entry : kSavedPointNameByViaPosition) {
    layout->SavePoint(entry.second, ViaLocation(entry.first));
  }
  layout->SavePoint("diff_lower_left", diff_rectangle->lower_left());
  layout->SavePoint("diff_upper_right", diff_rectangle->upper_right());

  return layout.release();
}

std::string Sky130SimpleTransistor::CircuitCellName() const {
  switch (parameters_.fet_type) {
    case Parameters::FetType::PMOS:
      return "sky130_fd_pr__pfet_01v8";
    case Parameters::FetType::PMOS_HVT:
      return "sky130_fd_pr__pfet_01v8_hvt";
    case Parameters::FetType::PMOS_LVT:
      return "sky130_fd_pr__pfet_01v8_lvt";
    case Parameters::FetType::NMOS:
      return "sky130_fd_pr__nfet_01v8";
    case Parameters::FetType::NMOS_HVT:
      return "sky130_fd_pr__nfet_01v8_hvt";
    case Parameters::FetType::NMOS_LVT:
    default:
      return "sky130_fd_pr__nfet_01v8_lvt";
  }
}

bfg::Circuit *Sky130SimpleTransistor::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new Circuit());
  Circuit *parent_cell =
      design_db_->FindCellOrDie("sky130", CircuitCellName())->circuit();
  circuit::Instance *nfet_0 = circuit->AddInstance("fet", parent_cell);
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
