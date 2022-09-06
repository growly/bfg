#include "sky130_mux.h"

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
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout());
  layout->AddLayout(*mux2_layout);

  return layout.release();
}

bfg::Layout *Sky130Mux::GenerateMux2Layout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  constexpr int64_t poly_pitch = 500;

  // To avoid C++'s automatic arithmetic type conversions, make these all the
  // same int64_t type:
  int64_t pfet_0_width = 640;
  int64_t pfet_1_width = 640;
  int64_t pfet_2_width = 640;
  int64_t pfet_3_width = 640;
  int64_t pfet_4_width = 640;
  int64_t pfet_5_width = 640;

  int64_t diffusion_min_distance = 270;

  int64_t via_side = 170;

  // This is li1 in Sky130.
  int64_t met_0_min_separation = 170;
  int64_t met_0_via_overhang = 80;
  int64_t met_0_via_poly_separation = 50;
  int64_t met_0_via_diff_padding = 40;
  int64_t met_0_min_width = 170;

  int64_t start_x = poly_pitch / 2;
  int64_t height = std::max(pfet_0_width, pfet_2_width) +
     diffusion_min_distance + std::max(pfet_1_width, pfet_3_width) + 100;
  int64_t poly_width = 170;
  int64_t poly_gap = poly_pitch - poly_width;
  int64_t poly_overhang = 130;
  int64_t diff_wing = via_side + met_0_via_poly_separation +
      met_0_via_diff_padding;

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
  int64_t via_column_0_x = via_side / 2 + met_0_via_diff_padding;
  int64_t via_row_0_y = pfet_0_width / 2;
  Rectangle *via_0_0 = layout->AddSquare(
      Point(via_column_0_x, via_row_0_y), via_side);

  int64_t via_row_1_y = height - pfet_0_width / 2;
  Rectangle *via_0_1 = layout->AddSquare(
      Point(via_column_0_x, via_row_1_y), via_side);

  int64_t via_column_1_x = column_0_1_mid_x;
  Rectangle *via_1_0 = layout->AddSquare(
      Point(via_column_1_x, via_row_0_y), via_side);
  layout->AddSquare(Point(via_column_1_x, via_row_1_y), via_side);

  int64_t via_column_2_x = pfet_3_diff->upper_right().x() - via_side / 2;
  layout->AddSquare(Point(via_column_2_x, via_row_0_y), via_side);
  layout->AddSquare(Point(via_column_2_x, via_row_1_y), via_side);

  int64_t via_column_3_x = pfet_4_diff->lower_left().x() + via_side / 2;
  layout->AddSquare(Point(via_column_3_x, via_row_1_y), via_side);

  int64_t via_column_4_x = pfet_5_diff->upper_right().x() - via_side / 2;
  layout->AddSquare(Point(via_column_4_x, via_row_1_y), via_side);

  Polygon *input_2_met_0;
  {
    layout->SetActiveLayerByName("li.drawing"); 
    int64_t metal_width = met_0_min_width;
    Point p_0 = via_0_0->lower_left() - Point(0, met_0_via_overhang);
    Point p_1 = Point(via_0_0->upper_right().x(), p_0.y());
    Point p_2 = via_0_0->upper_right() + Point(0, 4 * met_0_via_overhang);
    Point p_3 = Point(0, p_2.y());
    Point p_4 = Point(0, p_2.y() - metal_width);
    Point p_5 = Point(p_0.x(), p_4.y());
    input_2_met_0  = layout->AddPolygon(Polygon(
          {p_0, p_1, p_2, p_3, p_4, p_5}));

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(p_5 + Point(via_side / 2, via_side / 2), via_side);
  }

  {
    layout->SetActiveLayerByName("li.drawing");
    int64_t metal_width = met_0_min_width;
    int64_t lower_left_y =
        input_2_met_0->GetBoundingBox().lower_left().y() -
        metal_width - met_0_min_separation;
    Point p_0 = Point(0, lower_left_y);
    Point p_1 = Point(via_1_0->upper_right().x(), p_0.y());
    Point p_2 = via_1_0->upper_right() + Point(0, met_0_via_overhang);
    Point p_3 = Point(via_1_0->lower_left().x(), p_2.y());
    Point p_5 = p_0 + Point(0, metal_width);
    Point p_4 = Point(p_3.x(), p_5.y());
    layout->AddPolygon(Polygon({p_0, p_1, p_2, p_3, p_4, p_5}));

    layout->SetActiveLayerByName("li.pin");
    layout->AddSquare(p_0 + Point(via_side / 2, via_side / 2), via_side);
  }

  layout->SetActiveLayerByName("li.drawing");
  Polygon *input_0_met_0 = layout->AddPolygon(*input_2_met_0);
  input_0_met_0->FlipVertical();
  input_0_met_0->MoveLowerLeftTo(
      via_0_1->centre() + (input_2_met_0->GetBoundingBox().UpperLeft() - via_0_0->centre()));

  return layout.release();
}

bfg::Circuit *Sky130Mux::GenerateMux2Circuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());


  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
