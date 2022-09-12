#include "sky130_mux.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../circuit/wire.h"
#include "../cell.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/polygon.h"
#include "../geometry/poly_line.h"
#include "../poly_line_inflator.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::PolyLine;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Layer;

//

bfg::Cell *Sky130Mux::Generate() {
  // A flip-flop is two back-to-back latches.

  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_mux"));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  return cell.release();
}

bfg::Circuit *Sky130Mux::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  // Import the Sky130 dfxtp_1 cell and discard the input buffers. You get:

  circuit::Wire D = circuit->AddSignal("D");
  circuit::Wire CLK = circuit->AddSignal("CLK");
  circuit::Wire CLKI = circuit->AddSignal("CLKI");
  circuit::Wire Q = circuit->AddSignal("Q");

  circuit::Wire VPWR = circuit->AddSignal("VPWR");
  circuit::Wire VGND = circuit->AddSignal("VGND");
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(D);
  circuit->AddPort(CLK);
  circuit->AddPort(CLKI);
  circuit->AddPort(Q);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  return circuit.release();
}

bfg::Layout *Sky130Mux::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux4_layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout());
  mux2_layout->ResetOrigin();
  mux4_layout->AddLayout(*mux2_layout, "structure0");

  Rectangle mux2_bounding_box = mux2_layout->GetBoundingBox();
  int64_t intra_spacing = db.Rules("li.drawing").min_separation;

  mux2_layout->FlipHorizontal();
  mux2_layout->ResetOrigin();
  mux2_layout->Translate(Point(
        mux2_bounding_box.Width() , 0));
  mux4_layout->AddLayout(*mux2_layout, "structure1");
  layout->AddLayout(*mux4_layout, "structure2");

  mux4_layout->MirrorX();
  mux4_layout->ResetOrigin();
  mux4_layout->Translate(Point(
      0, -(mux4_layout->GetBoundingBox().Height())));
  layout->AddLayout(*mux4_layout, "");

  layout->ResetOrigin();

  Rectangle bounding_box = layout->GetBoundingBox();
  const IntraLayerConstraints &met1_rules = db.Rules("met1.drawing");

  {
    // Add vertical selector connections.
    int64_t offset_x = 50;
    int64_t pitch = met1_rules.min_width + met1_rules.min_separation;
    int64_t width = static_cast<int64_t>(bounding_box.Width());
    int64_t height = static_cast<int64_t>(bounding_box.Height());
    layout->SetActiveLayerByName("met1.drawing");
    for (int64_t x = offset_x; x < width; x += pitch) {
      layout->AddRectangle({{x, 0}, {x + met1_rules.min_width, height}});
    }
  }

  return layout.release();
}

// Stack up for our purposes:
//  top    - met1
//         - mcon
//         - li
//         - licon
//  bottom - poly | diff
bfg::Layout *Sky130Mux::GenerateMux2Layout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  // To avoid C++'s automatic arithmetic type conversions, make these all the
  // same int64_t type:
  int64_t pfet_0_width = 640;
  int64_t pfet_1_width = 640;
  int64_t pfet_2_width = 640;
  int64_t pfet_3_width = 640;
  int64_t pfet_4_width = 640;
  int64_t pfet_5_width = 640;

  const IntraLayerConstraints &diff_rules = db.Rules("diff.drawing");
  int64_t diffusion_min_distance = diff_rules.min_separation;
  const IntraLayerConstraints &licon_rules = db.Rules("licon.drawing");
  int64_t via_side = licon_rules.via_width;

  const IntraLayerConstraints &li_rules = db.Rules("li.drawing");
  const IntraLayerConstraints &poly_rules = db.Rules("poly.drawing");

  const InterLayerConstraints &li_licon_rules = db.Rules(
      "li.drawing", "licon.drawing");
  const InterLayerConstraints &poly_licon_rules = db.Rules(
      "poly.drawing", "licon.drawing");
  const InterLayerConstraints &diff_licon_rules = db.Rules(
      "diff.drawing", "licon.drawing");

  int64_t poly_pitch = poly_rules.min_pitch;
  int64_t start_x = poly_pitch / 2;
  int64_t height = std::max(pfet_0_width, pfet_2_width) +
     diffusion_min_distance + std::max(pfet_1_width, pfet_3_width) +
     li_rules.min_separation;
  int64_t poly_width = poly_rules.min_width;
  int64_t poly_gap = poly_pitch - poly_width;
  int64_t diff_wing = via_side + poly_licon_rules.min_separation +
      diff_licon_rules.min_separation;

  // TODO: This is the distance diffusion extends from poly. Is it a DRC rule?
  int64_t poly_overhang = li_rules.min_width + li_rules.min_separation;

  layout->SetActiveLayerByName("poly.drawing");
  int64_t column_0_x = start_x;
  Rectangle *column_0 = layout->AddRectangle(Rectangle(
      Point(column_0_x, -poly_overhang),
      Point(column_0_x + poly_width, height + poly_overhang)));

  int64_t column_1_x = column_0_x + poly_pitch;
  Rectangle *column_1 = layout->AddRectangle(Rectangle(
      Point(column_1_x, -poly_overhang),
      Point(column_1_x + poly_width, height + poly_overhang)));

  int64_t column_2_x = column_1_x + 2 * poly_pitch;
  Rectangle *column_2 = layout->AddRectangle(Rectangle(
      Point(column_2_x, height - pfet_4_width - poly_overhang),
      Point(column_2_x + poly_width, height + poly_overhang)));

  int64_t column_3_x = column_2_x + poly_pitch;
  Rectangle *column_3 = layout->AddRectangle(Rectangle(
      Point(column_3_x, height - pfet_5_width - poly_overhang),
      Point(column_3_x + poly_width, height + poly_overhang)));

  layout->SetActiveLayerByName("diff.drawing");
  // pfet 0
  int64_t column_0_1_mid_x = column_0->upper_right().x() + poly_gap / 2;
  layout->AddRectangle(Rectangle(
      Point(0, 0),
      Point(column_0_1_mid_x, pfet_0_width)));

  // pfet 1
  layout->AddRectangle(Rectangle(
      Point(0, height - pfet_1_width),
      Point(column_0_1_mid_x, height)));

  // pfet 2
  Rectangle * pfet_3_diff = layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, 0),
      Point(column_1->upper_right().x() + diff_wing, pfet_2_width)));

  // pfet 3
  layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, height - pfet_3_width),
      Point(column_1->upper_right().x() + diff_wing, height)));

  // pfet 4
  int64_t column_2_3_mid_x = column_2->upper_right().x() + poly_gap / 2;
  Rectangle *pfet_4_diff = layout->AddRectangle(Rectangle(
      Point(column_2->lower_left().x() - diff_wing, height - pfet_4_width),
      Point(column_2_3_mid_x, height)));

  // pfet 5
  Rectangle *pfet_5_diff = layout->AddRectangle(Rectangle(
      Point(column_2_3_mid_x, height - pfet_5_width),
      Point(column_3->upper_right().x() + diff_wing, height)));

  // diff/met0 vias
  layout->SetActiveLayerByName("licon.drawing");
  int64_t via_column_0_x = via_side / 2 + diff_licon_rules.min_separation;
  int64_t via_row_0_y = pfet_0_width / 2;
  Rectangle *via_0_0 = layout->AddSquare(
      Point(via_column_0_x, via_row_0_y), via_side);

  int64_t via_row_1_y = height - pfet_0_width / 2;
  Rectangle *via_0_1 = layout->AddSquare(
      Point(via_column_0_x, via_row_1_y), via_side);

  int64_t via_column_1_x = column_0_1_mid_x;
  Rectangle *via_1_0 = layout->AddSquare(
      Point(via_column_1_x, via_row_0_y), via_side);
  Rectangle *via_1_1 = layout->AddSquare(
      Point(via_column_1_x, via_row_1_y), via_side);

  int64_t via_column_2_x = pfet_3_diff->upper_right().x() - via_side / 2;
  Rectangle *via_2_0 = layout->AddSquare(
      Point(via_column_2_x, via_row_0_y), via_side);
  Rectangle *via_2_1 = layout->AddSquare(
      Point(via_column_2_x, via_row_1_y), via_side);

  int64_t via_column_3_x = pfet_4_diff->lower_left().x() + via_side / 2;
  Rectangle *via_3_1 = layout->AddSquare(
      Point(via_column_3_x, via_row_1_y), via_side);

  int64_t via_column_4_x = pfet_5_diff->upper_right().x() - via_side / 2;
  Rectangle *via_4_1 = layout->AddSquare(
      Point(via_column_4_x, via_row_1_y), via_side);

  PolyLineInflator inflator(design_db_->physical_db());

  Polygon *input_2_met_0;
  {
    // Input 2 metal.
    layout->SetActiveLayerByName("li.drawing"); 
    int64_t metal_width = li_rules.min_width;

    Point p_0 = via_0_0->centre();
    Point p_1 = p_0 + Point(0, 4 * li_licon_rules.via_overhang);
    Point p_2 = Point(0, p_1.y());
    PolyLine input_2_line = PolyLine({p_0, p_1, p_2});
    input_2_line.SetWidth(li_rules.min_width);
    input_2_line.set_overhang_start(via_side / 2 + li_licon_rules.via_overhang);
    input_2_line.set_overhang_end(0);
    Polygon input_2_template;
    inflator.InflatePolyLine(input_2_line, &input_2_template);
    // This is the installed object... it's different.
    input_2_met_0 = layout->AddPolygon(input_2_template);

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(p_1, via_side);
  }

  Polygon *input_3_met_0;
  {
    // Input 3 metal.
    layout->SetActiveLayerByName("li.drawing");
    int64_t metal_width = li_rules.min_width;
    int64_t lower_left_y = input_2_met_0->GetBoundingBox().lower_left().y() -
                           (metal_width / 2) - li_rules.min_separation;

    Point p_0 = Point(0, lower_left_y);
    Point p_2 = via_2_0->centre();
    Point p_1 = Point(p_2.x(), p_0.y());
    PolyLine input_3_line = PolyLine({p_0, p_1, p_2});
    input_3_line.SetWidth(metal_width);
    input_3_line.set_overhang_start(0);
    input_3_line.set_overhang_end(via_side / 2 + li_licon_rules.via_overhang);

    Polygon input_3_template;
    inflator.InflatePolyLine(input_3_line, &input_3_template);
    input_3_met_0 = layout->AddPolygon(input_3_template);

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(p_0 + Point(via_side / 2, 0), via_side);
  }

  {
    // Input 0 metal.
    layout->SetActiveLayerByName("li.drawing");
    Polygon *input_0_met_0 = layout->AddPolygon(*input_2_met_0);
    input_0_met_0->MirrorX();
    Point via_relative_to_corner =
        input_2_met_0->GetBoundingBox().UpperLeft() - via_0_0->centre();
    via_relative_to_corner.MirrorX();
    input_0_met_0->MoveLowerLeftTo(via_0_1->centre() + via_relative_to_corner);

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(
        input_0_met_0->GetBoundingBox().LowerRight() - Point(
            via_side / 2, -via_side / 2), via_side);
  }

  {
    // Input 1 metal.
    layout->SetActiveLayerByName("li.drawing");
    Polygon *input_1_met_0 = layout->AddPolygon(*input_3_met_0);
    input_1_met_0->MirrorX();
    Point via_relative_to_corner =
        input_3_met_0->GetBoundingBox().UpperLeft() - via_1_0->centre();
    via_relative_to_corner.MirrorX();
    input_1_met_0->MoveLowerLeftTo(via_1_1->centre() + via_relative_to_corner);

    // Since we constructed input_3_met_0 in this order, and then flipped the
    // points about the x-axis, we know which points should be at the "end" of
    // the wire:
    Point end = Point(via_side / 2, input_1_met_0->vertices().back().y() - via_side / 2);

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(end, via_side);
  }

  {
    layout->SetActiveLayerByName("li.drawing");
    Point p_0 = via_1_1->centre();
    int64_t line_y =  via_2_1->centre().y() - (
        (via_side / 2) + li_licon_rules.via_overhang + li_rules.min_separation);
    Point p_1 = Point(p_0.x(), line_y);
    Point p_2 = Point(via_3_1->centre().x(), p_1.y());
    Point p_3 = via_3_1->centre();
    PolyLine input_0_1_line = PolyLine({p_0, p_1, p_2, p_3});
    input_0_1_line.SetWidth(li_rules.min_width);
    input_0_1_line.set_overhang_start(via_side / 2 + li_licon_rules.via_overhang);
    input_0_1_line.set_overhang_end(via_side / 2 + li_licon_rules.via_overhang);
    PolyLineInflator inflator(design_db_->physical_db());
    Polygon input_0_1;
    inflator.InflatePolyLine(input_0_1_line, &input_0_1);
    layout->AddPolygon(input_0_1);
  }

  {
    layout->SetActiveLayerByName("li.drawing");
    Point p_0 = via_1_0->centre();
    int64_t line_y =  via_2_0->centre().y() + (
        (via_side / 2) + li_licon_rules.via_overhang + li_rules.min_separation);
    Point p_1 = Point(p_0.x(), line_y);
    Point p_2 = Point(via_4_1->centre().x(), p_1.y());
    Point p_3 = via_4_1->centre();
    PolyLine input_2_3_line = PolyLine({p_0, p_1, p_2, p_3});
    input_2_3_line.SetWidth(li_rules.min_width);
    input_2_3_line.set_overhang_start(via_side / 2 + li_licon_rules.via_overhang);
    input_2_3_line.set_overhang_end(via_side / 2 + li_licon_rules.via_overhang);
    Polygon input_2_3;
    inflator.InflatePolyLine(input_2_3_line, &input_2_3);
    layout->AddPolygon(input_2_3);
  }

  return layout.release();
}

bfg::Circuit *Sky130Mux::GenerateMux2Circuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());


  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
