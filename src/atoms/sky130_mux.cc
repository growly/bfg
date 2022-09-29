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

namespace {

void ConnectPolyOrDiffToMet1(
    const PhysicalPropertiesDatabase &db,
    const Point &poly_point,
    const Point &met1_point,
    bfg::Layout *layout) {
  PolyLineInflator inflator(db);

  const IntraLayerConstraints &li_rules = db.Rules("li.drawing");
  const IntraLayerConstraints &licon_rules = db.Rules("licon.drawing");
  const IntraLayerConstraints &mcon_rules = db.Rules("mcon.drawing");
  const InterLayerConstraints &li_licon_rules = db.Rules("li.drawing", "licon.drawing");
  const InterLayerConstraints &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");

  layout->MakeVia("licon.drawing", poly_point);

  layout->SetActiveLayerByName("li.drawing");
  PolyLine li_pour_line = PolyLine({poly_point, met1_point});
  li_pour_line.SetWidth(li_rules.min_width);
  li_pour_line.set_overhang_start(
      db.Rules("licon.drawing").via_width / 2 + li_licon_rules.via_overhang);
  li_pour_line.set_overhang_end(
      db.Rules("mcon.drawing").via_width/ 2 + li_mcon_rules.via_overhang);
  Polygon li_pour_template;
  inflator.InflatePolyLine(li_pour_line, &li_pour_template);
  layout->AddPolygon(li_pour_template);

  layout->MakeVia("mcon.drawing", met1_point);
}

Polygon *StraightLineBetweenLayers(
    const PhysicalPropertiesDatabase &db,
    const Point &start,
    const Point &end,
    const std::string &start_layer,
    const std::string &path_layer,
    const std::string &end_layer,
    bfg::Layout *layout) {
  int64_t width = db.Rules(path_layer).min_width;
  int64_t start_overhang = db.Rules(start_layer).via_width / 2
        + db.Rules(start_layer, path_layer).via_overhang;
  int64_t end_overhang = db.Rules(end_layer).via_width / 2
        + db.Rules(end_layer, path_layer).via_overhang;
  PolyLineInflator inflator(db);
  PolyLine line = PolyLine({start, end});
  line.SetWidth(width);
  line.set_overhang_start(start_overhang);
  line.set_overhang_end(end_overhang);
  Polygon polygon;
  inflator.InflatePolyLine(line, &polygon);
  return layout->AddPolygon(polygon);
}

void ConnectNamedPointsToColumn(
    const PhysicalPropertiesDatabase &db,
    const std::vector<std::string> named_points,
    const Rectangle &rectangle,
    bfg::Layout *layout) {
  for (const std::string &name : named_points) {
    Point source = layout->GetPoint(name);
    ConnectPolyOrDiffToMet1(db, source, Point(rectangle.centre().x(), source.y()), layout);
  }
}

Polygon *AddElbowPath(
    const PhysicalPropertiesDatabase &db,
    const Point &start,
    const Point &end,
    const int64_t width,
    const int64_t start_overhang,
    const int64_t end_overhang,
    bfg::Layout *layout) {
  PolyLineInflator inflator(db);
  PolyLine line = PolyLine({start, Point(start.x(), end.y()), end});
  line.SetWidth(width);
  line.set_overhang_start(start_overhang);
  line.set_overhang_end(end_overhang);
  Polygon polygon;
  inflator.InflatePolyLine(line, &polygon);
  return layout->AddPolygon(polygon);
}

Polygon *AddElbowPathBetweenLayers(
    const PhysicalPropertiesDatabase &db,
    const Point &start,
    const Point &end,
    const std::string &start_layer,
    const std::string &path_layer,
    const std::string &end_layer,
    bfg::Layout *layout) {
  return AddElbowPath(
      db,
      start,
      end,
      db.Rules(path_layer).min_width,
      db.Rules(start_layer).via_width / 2
          + db.Rules(start_layer, path_layer).via_overhang,
      db.Rules(end_layer).via_width / 2
          + db.Rules(end_layer, path_layer).via_overhang,
      layout);
}

}

bfg::Layout *Sky130Mux::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const IntraLayerConstraints &poly_rules = db.Rules("poly.drawing");

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux4_layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout());

  Rectangle mux2_bounding_box = mux2_layout->GetBoundingBox();
  int64_t mux2_width =
      (1 + mux2_bounding_box.Width() / poly_rules.min_pitch) * poly_rules.min_pitch;
  int64_t mux2_height = mux2_bounding_box.Height();
  int64_t intra_spacing = 0; //db.Rules("li.drawing").min_separation;
  int64_t vert_spacing = 0;

  mux2_layout->ResetOrigin();
  layout->AddLayout(*mux2_layout, "upper_left");

  mux2_layout->FlipHorizontal();
  mux2_layout->MoveLowerLeftTo(Point(mux2_width + intra_spacing, 0));
  layout->AddLayout(*mux2_layout, "upper_right");

  mux2_layout->FlipVertical();
  mux2_layout->MoveLowerLeftTo(
      Point(mux2_width + intra_spacing, -(mux2_height + vert_spacing)));
  layout->AddLayout(*mux2_layout, "lower_right");

  mux2_layout->FlipHorizontal();
  mux2_layout->MoveLowerLeftTo(Point(0, -(mux2_height + vert_spacing)));
  layout->AddLayout(*mux2_layout, "lower_left");

  layout->ResetOrigin();

  Rectangle bounding_box = layout->GetBoundingBox();
  const IntraLayerConstraints &li_rules = db.Rules("li.drawing");
  const IntraLayerConstraints &licon_rules = db.Rules("licon.drawing");
  const IntraLayerConstraints &mcon_rules = db.Rules("mcon.drawing");
  const IntraLayerConstraints &met1_rules = db.Rules("met1.drawing");
  const InterLayerConstraints &li_licon_rules = db.Rules("li.drawing", "licon.drawing");
  const InterLayerConstraints &diff_licon_rules = db.Rules("diff.drawing", "licon.drawing");
  const InterLayerConstraints &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");
  const InterLayerConstraints &poly_licon_rules = db.Rules(
      "poly.drawing", "licon.drawing");
  const InterLayerConstraints &met1_licon_rules = db.Rules(
      "met1.drawing", "licon.drawing");
  const InterLayerConstraints &met1_mcon_rules = db.Rules(
      "met1.drawing", "mcon.drawing");

  std::set<int64_t> enabled_columns = {0, 2, 4, 7, 9, 12, 14, 16};
  std::map<int64_t, int64_t> met1_column_x_midpoints;
  std::map<int64_t, Rectangle*> enabled_column_geometries;
  {
    // Add vertical selector connections.
    int64_t offset_x = 50;
    int64_t pitch = met1_rules.min_width + met1_rules.min_separation;
    int64_t width = static_cast<int64_t>(bounding_box.Width());
    int64_t height = static_cast<int64_t>(bounding_box.Height());
    int64_t extension = 150;
    layout->SetActiveLayerByName("met1.drawing");

    for (int64_t x = offset_x, k = 0; x < width; x += pitch, ++k) {
      Point ll = {x, -extension};
      Point ur = {x + met1_rules.min_width, height + extension};
      met1_column_x_midpoints[k] = (ll.x() + ur.x()) / 2;
      if (enabled_columns.find(k) != enabled_columns.end()) {
        enabled_column_geometries[k] = layout->AddRectangle({ll, ur});
      }
    }
  }

  ConnectNamedPointsToColumn(
      db,
      {"lower_left.column_0_centre_bottom", "lower_left.column_0_centre_top",
       "upper_left.column_0_centre_top"},
      *enabled_column_geometries[0],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"lower_left.column_1_centre_bottom", "lower_left.column_1_centre_top",
       "upper_left.column_1_centre_top"},
      *enabled_column_geometries[2],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"upper_left.column_2_centre_top", "lower_left.column_2_centre_top"},
      *enabled_column_geometries[4],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"upper_left.column_3_centre_top", "lower_left.column_3_centre_top"},
      *enabled_column_geometries[7],
      layout.get());

  ConnectNamedPointsToColumn(
      db,
      {"upper_right.column_0_centre_bottom", "upper_right.column_0_centre_top",
       "lower_right.column_0_centre_top"},
      *enabled_column_geometries[16],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"upper_right.column_1_centre_bottom", "upper_right.column_1_centre_top",
       "lower_right.column_1_centre_top"},
      *enabled_column_geometries[14],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"upper_right.column_2_centre_top", "lower_right.column_2_centre_top"},
      *enabled_column_geometries[12],
      layout.get());
  ConnectNamedPointsToColumn(
      db,
      {"upper_right.column_3_centre_top", "upper_right.column_3_centre_top"},
      *enabled_column_geometries[9],
      layout.get());

           
  //LOG(INFO) << layout->Describe();

  int64_t stage_2_mux_fet_0_width = 640;
  int64_t stage_2_mux_fet_1_width = 640;

  {
    int64_t diff_wing = licon_rules.via_width +
        poly_licon_rules.min_separation +
        diff_licon_rules.min_separation;
    int64_t poly_overhang = li_rules.min_width + li_rules.min_separation;
    int64_t poly_pitch = poly_rules.min_pitch;
    int64_t poly_gap = poly_pitch - poly_rules.min_width;
    Point gap_top = layout->GetPoint("upper_left.column_2_centre_bottom");
    Point gap_bottom = layout->GetPoint("lower_left.column_2_centre_bottom");
    // 2 * poly_overhang + stage_2_mux_fet_0_width;
    int64_t height = (gap_top.y() - gap_bottom.y()) - 2 * poly_rules.min_separation;
    Point poly_ur = Point(gap_top.x() + poly_rules.min_width / 2,
                          ((gap_top + gap_bottom).y() + height) / 2);

    Point poly_ll = poly_ur - Point(poly_rules.min_width, height);
    Point diff_ur = poly_ur + Point(poly_gap / 2, -poly_overhang);
    Point diff_ll = Point(
        poly_ll.x() - diff_wing, diff_ur.y() - stage_2_mux_fet_0_width);
    layout->SetActiveLayerByName("poly.drawing");
    layout->AddRectangle({poly_ll, poly_ur});
    layout->SetActiveLayerByName("diff.drawing");
    Rectangle *fet_0_diff = layout->AddRectangle({diff_ll, diff_ur});
    Point input_0 = Point(
        fet_0_diff->lower_left().x() + diff_wing / 2,
        fet_0_diff->upper_right().y()
            - diff_licon_rules.min_separation
            - licon_rules.via_width / 2);
    layout->SavePoint("output_mux_s0", input_0);

    // Opposite side:
    poly_ur.Translate({poly_pitch, 0});
    poly_ll.Translate({poly_pitch, 0});
    diff_ll.set_x(diff_ur.x());
    diff_ur = poly_ur + Point({diff_wing, -poly_overhang});
    diff_ll.set_y(diff_ur.y() - stage_2_mux_fet_1_width);
    layout->SetActiveLayerByName("poly.drawing");
    layout->AddRectangle({poly_ll, poly_ur});
    layout->SetActiveLayerByName("diff.drawing");
    Rectangle *fet_1_diff = layout->AddRectangle({diff_ll, diff_ur});
    Point input_1 = Point(
        fet_1_diff->upper_right().x() - diff_wing / 2,
        fet_1_diff->lower_left().y()
            + diff_licon_rules.min_separation
            + licon_rules.via_width / 2);
    layout->SavePoint("output_mux_s1", input_1);

    Point source = layout->GetPoint("upper_left.output");
    layout->MakeVia("licon.drawing", source);
    Point destination = layout->GetPoint("output_mux_s0");
    layout->MakeVia("licon.drawing", destination);
    int64_t via_side = li_rules.via_width;
    Point met1_connect_destination = Point(
        met1_column_x_midpoints[5], poly_ur.y() - met1_rules.min_width / 2);
    Point met1_connect_source = Point(
        met1_column_x_midpoints[5], source.y());

    layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        destination, met1_connect_destination,
        "licon.drawing", "li.drawing", "mcon.drawing",
        layout.get());

    layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_connect_source, met1_connect_destination,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        layout.get());
    layout->MakeVia("mcon.drawing", met1_connect_destination);

    layout->SetActiveLayerByName("li.drawing");
    ConnectPolyOrDiffToMet1(db, source, met1_connect_source, layout.get());

    source = layout->GetPoint("lower_left.output");
    layout->MakeVia("licon.drawing", source);
    destination = layout->GetPoint("output_mux_s1");
    layout->MakeVia("licon.drawing", destination);
    // This could also be something like
    //  layout->GetPort("output").centre();
    // if it had been added as a port.
    met1_connect_destination = Point(
        met1_column_x_midpoints[6], poly_ll.y() + met1_rules.min_width / 2);
    met1_connect_source = Point(
        met1_column_x_midpoints[6], source.y());
    layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        destination, met1_connect_destination,
        "licon.drawing", "li.drawing", "mcon.drawing",
        layout.get());
    layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_connect_source, met1_connect_destination,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        layout.get());
    layout->MakeVia("mcon.drawing", met1_connect_destination);

    layout->SetActiveLayerByName("li.drawing");
    ConnectPolyOrDiffToMet1(db, source, met1_connect_source, layout.get());
  }

  return layout.release();
}

// Stack up for our purposes:
//  top    - met1
//         - mcon
//         - met0 (li)
//         - licon
//  bottom - poly | diff
bfg::Layout *Sky130Mux::GenerateMux2Layout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  // To avoid C++'s automatic arithmetic type conversions, make these all the
  // same int64_t type:
  int64_t fet_0_width = 640;
  int64_t fet_1_width = 640;
  int64_t fet_2_width = 640;
  int64_t fet_3_width = 640;
  int64_t fet_4_width = 640;
  int64_t fet_5_width = 640;

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
  int64_t height = std::max(fet_0_width, fet_2_width) +
     diffusion_min_distance + std::max(fet_1_width, fet_3_width) +
     li_rules.min_separation;
  int64_t poly_width = poly_rules.min_width;
  int64_t poly_gap = poly_pitch - poly_width;
  int64_t diff_wing = via_side + poly_licon_rules.min_separation +
      diff_licon_rules.min_separation;

  // TODO: This is the distance diffusion extends from poly. Is it a DRC rule?
  int64_t poly_overhang = li_rules.min_width + li_rules.min_separation
      + licon_rules.via_width + li_licon_rules.via_overhang;

  layout->SetActiveLayerByName("poly.drawing");
  int64_t column_0_x = start_x;
  Rectangle *column_0 = layout->AddRectangle(Rectangle(
      Point(column_0_x, -poly_overhang),
      Point(column_0_x + poly_width, height + poly_overhang)));

  layout->SavePoint("column_0_centre_bottom", Point(
      column_0->centre().x(), column_0->lower_left().y()));
  layout->SavePoint("column_0_centre_top", Point(
      column_0->centre().x(), column_0->upper_right().y()));

  int64_t column_1_x = column_0_x + poly_pitch;
  Rectangle *column_1 = layout->AddRectangle(Rectangle(
      Point(column_1_x, -poly_overhang),
      Point(column_1_x + poly_width, height + poly_overhang)));

  layout->SavePoint("column_1_centre_bottom", Point(
      column_1->centre().x(), column_1->lower_left().y()));
  layout->SavePoint("column_1_centre_top", Point(
      column_1->centre().x(), column_1->upper_right().y()));

  int64_t column_2_x = column_1_x + 2 * poly_pitch;
  Rectangle *column_2 = layout->AddRectangle(Rectangle(
      Point(column_2_x, height - fet_4_width - poly_overhang),
      Point(column_2_x + poly_width, height + poly_overhang)));

  layout->SavePoint("column_2_centre_bottom", Point(
      column_2->centre().x(), column_2->lower_left().y()));
  layout->SavePoint("column_2_centre_top", Point(
      column_2->centre().x(), column_2->upper_right().y()));

  int64_t column_3_x = column_2_x + poly_pitch;
  Rectangle *column_3 = layout->AddRectangle(Rectangle(
      Point(column_3_x, height - fet_5_width - poly_overhang),
      Point(column_3_x + poly_width, height + poly_overhang)));

  layout->SavePoint("column_3_centre_bottom", Point(
      column_3->centre().x(), column_3->lower_left().y()));
  layout->SavePoint("column_3_centre_top", Point(
      column_3->centre().x(), column_3->upper_right().y()));

  layout->SetActiveLayerByName("diff.drawing");
  // pfet 0
  int64_t column_0_1_mid_x = column_0->upper_right().x() + poly_gap / 2;
  layout->AddRectangle(Rectangle(
      Point(0, 0),
      Point(column_0_1_mid_x, fet_0_width)));

  // pfet 1
  layout->AddRectangle(Rectangle(
      Point(0, height - fet_1_width),
      Point(column_0_1_mid_x, height)));

  // pfet 2
  Rectangle *pfet_3_diff = layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, 0),
      Point(column_1->upper_right().x() + diff_wing, fet_2_width)));

  // pfet 3
  layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, height - fet_3_width),
      Point(column_1->upper_right().x() + diff_wing, height)));

  // pfet 4
  int64_t column_2_3_mid_x = column_2->upper_right().x() + poly_gap / 2;
  Rectangle *pfet_4_diff = layout->AddRectangle(Rectangle(
      Point(column_2->lower_left().x() - diff_wing, height - fet_4_width),
      Point(column_2_3_mid_x, height)));

  // pfet 5
  Rectangle *pfet_5_diff = layout->AddRectangle(Rectangle(
      Point(column_2_3_mid_x, height - fet_5_width),
      Point(column_3->upper_right().x() + diff_wing, height)));

  layout->SavePoint("output", Point(
        pfet_4_diff->upper_right().x(),
        (pfet_4_diff->centre() + pfet_5_diff->centre()).y() / 2));

  // diff/met0 vias
  layout->SetActiveLayerByName("licon.drawing");
  int64_t via_column_0_x = via_side / 2 + diff_licon_rules.min_separation;
  int64_t via_row_0_y = fet_0_width / 2;
  Rectangle *via_0_0 = layout->AddSquare(
      Point(via_column_0_x, via_row_0_y), via_side);

  int64_t via_row_1_y = height - fet_0_width / 2;
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
