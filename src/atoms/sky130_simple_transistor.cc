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
}

geometry::Point Sky130SimpleTransistor::ViaLocation(
    const ViaPosition &via_position) {
  geometry::Point lower_left;
  if (alignment_ && alignment_point_) {
    switch (*alignment_) {
      case POLY_TOP_CENTRE:
        lower_left = geometry::Point(
            alignment_point_->x() - TransistorLength() / 2 -
                DiffWing(geometry::Compass::LEFT),
            alignment_point_->y() - PolyHeight());
        break;
      case POLY_BOTTOM_CENTRE:
        lower_left = geometry::Point(
            alignment_point_->x() - TransistorLength() / 2 -
                DiffWing(geometry::Compass::LEFT),
            alignment_point_->y());
        break;
      default:
        LOG(FATAL) << "Unsupported alignment: " << *alignment_;
    }
  }
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

uint64_t Sky130SimpleTransistor::PolyHeight() const {
  const auto &poly_diff_rules =
      design_db_->physical_db().Rules("poly.drawing", DiffLayer());
  return TransistorWidth() + 2 * poly_diff_rules.min_enclosure;
}

// The origin will the the centre of the poly.
bfg::Layout *Sky130SimpleTransistor::GenerateLayout(
    geometry::Polygon **poly, geometry::Rectangle **diff) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  const auto &poly_diff_rules = db.Rules("poly.drawing", DiffLayer());

  int64_t x_pos = 0;
  // The "width" of the transistor poly is the "length" of the transistor, and
  // vice versa.
  int64_t poly_width = TransistorLength();
  int64_t diff_height = TransistorWidth();

  int64_t poly_y_min = -PolyHeight() / 2;
  int64_t poly_y_max = poly_y_min + PolyHeight();

  int64_t diff_y_min = poly_y_min + poly_diff_rules.min_enclosure;
  int64_t diff_y_max = diff_y_min + diff_height;

  layout->SetActiveLayerByName("poly.drawing");
  geometry::PolyLine line = geometry::PolyLine(
      {{x_pos, poly_y_min}, {x_pos, poly_y_max}});
  line.SetWidth(poly_width);
  geometry::Polygon *poly_polygon = layout->AddPolyLine(line);
  layout->SavePoint("poly_top_centre", line.End());
  layout->SavePoint("poly_bottom_centre", line.start());

  layout->SetActiveLayerByName(DiffLayer());
  geometry::Rectangle *diff_rectangle =
      layout->AddRectangle(geometry::Rectangle(
          {
            x_pos - poly_width / 2 - DiffWing(geometry::Compass::LEFT),
            diff_y_min
          },
          {
            x_pos + poly_width / 2 + DiffWing(geometry::Compass::RIGHT),
            diff_y_max
          }
      )
  );

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
