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

void Sky130SimpleTransistor::AlignTransistorPartTo(
    const Alignment &alignment, const geometry::Point &point) {
  alignment_ = alignment;
  alignment_point_ = point;

  if (alignment_ && alignment_point_) {
    switch (*alignment_) {
      case POLY_TOP_CENTRE:
        origin_ = geometry::Point(
            alignment_point_->x(),
            alignment_point_->y() - static_cast<int64_t>(PolyHeight() / 2));
        break;
      case POLY_BOTTOM_CENTRE:
        origin_ = geometry::Point(
            alignment_point_->x(),
            alignment_point_->y() + static_cast<int64_t>(PolyHeight() / 2));
        break;
      default:
        LOG(FATAL) << "Unsupported alignment: " << *alignment_;
    }
  }
}

geometry::Point Sky130SimpleTransistor::LowerLeft() const {
  // The origin is the centre of the poly and diff.
  return geometry::Point(
      origin_.x() - (
          DiffWing(geometry::Compass::LEFT) + TransistorLength() / 2),
      origin_.y() - static_cast<int64_t>(PolyHeight()) / 2);
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
    return (poly_rules.min_pitch - TransistorLength()) / 2;
  }

  std::string diff_layer = DiffLayer();
  std::string dcon_layer = DiffConnectionLayer();
  const auto &dcon_rules = db.Rules(dcon_layer);
  const auto &diff_dcon_rules = db.Rules(diff_layer, dcon_layer);
  const auto &poly_dcon_rules = db.Rules("poly.drawing", dcon_layer);
  const auto &poly_diff_rules = db.Rules("poly.drawing", diff_layer);
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
  layout->SavePoint("poly_top_centre", line.End());
  layout->SavePoint("poly_bottom_centre", line.start());

  layout->SetActiveLayerByName(DiffLayer());
  geometry::Rectangle *diff_rectangle = layout->AddRectangle(DiffBounds());

  if (diff) {
    *diff = diff_rectangle;
  }
  if (poly) {
    *poly = poly_polygon;
  }

  // Align.
  if (alignment_ && alignment_point_) {
    geometry::Point reference;
    switch (*alignment_) {
      case Alignment::POLY_BOTTOM_CENTRE:
        reference = layout->GetPointOrDie("poly_bottom_centre");
        break;
      case Alignment::POLY_TOP_CENTRE:
        reference = layout->GetPointOrDie("poly_top_centre");
        break;
      default:
        LOG(FATAL) << "Unsupported alignment: " << *alignment_;
    }
    layout->AlignPointTo(reference, *alignment_point_);
  }
       
  return layout.release();
}

bfg::Circuit *Sky130SimpleTransistor::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
