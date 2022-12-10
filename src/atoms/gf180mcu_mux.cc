#include "gf180mcu_mux.h"

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
using ::bfg::geometry::LineSegment;

namespace {

}   // namespace

bfg::Cell *Gf180McuMux::Generate() {
  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("gf180mcu_mux"));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *Gf180McuMux::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  return circuit.release();
}

bfg::Layout *Gf180McuMux::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules("poly.drawing");
  const auto &li_rules = db.Rules("li.drawing");
  const auto &ncon_rules = db.Rules("ncon.drawing");
  const auto &pcon_rules = db.Rules("pcon.drawing");
  const auto &mcon_rules = db.Rules("mcon.drawing");
  const auto &polycon_rules = db.Rules("polycon.drawing");
  const auto &diff_nsdm_rules = db.Rules("diff.drawing", "nsdm.drawing");
  const auto &diff_psdm_rules = db.Rules("diff.drawing", "psdm.drawing");
  const auto &psdm_nwell_rules = db.Rules("psdm.drawing", "nwell.drawing");
  const auto &ndiff_nwell_rules = db.Rules("nwell.drawing", "ndiff.drawing");
  const auto &pdiff_nwell_rules = db.Rules("pdiff.drawing", "nwell.drawing");
  const auto &pdiff_poly_rules = db.Rules("pdiff.drawing", "poly.drawing");
  const auto &ndiff_poly_rules = db.Rules("ndiff.drawing", "poly.drawing");
  const auto &li_ncon_rules = db.Rules("li.drawing", "ncon.drawing");
  const auto &li_pcon_rules = db.Rules("li.drawing", "pcon.drawing");
  const auto &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");
  const auto &li_polycon_rules = db.Rules("li.drawing", "polycon.drawing");
  const auto &poly_polycon_rules = db.Rules("poly.drawing", "polycon.drawing");

  Polygon *left_input_0;
  Polygon *left_input_1;
  Polygon *left_input_2;
  Polygon *left_input_3;
  Polygon *right_input_0;
  Polygon *right_input_1;
  Polygon *right_input_2;
  Polygon *right_input_3;

  Mux2Parameters mux2_params_n = {
    .diff_layer_name = "ndiff.drawing",
    .diff_contact_layer_name = "ncon.drawing",
    .fet_0_width = 465,
    .fet_1_width = 465,
    .fet_2_width = 465,
    .fet_3_width = 465,
    .fet_4_width = 465,
    .fet_5_width = 465,
    .fet_0_length = 600,
    .fet_1_length = 600,
    .fet_2_length = 600,
    .fet_3_length = 600,
    .fet_4_length = 600,
    .fet_5_length = 600,
    .fet_4_5_offset_y = -200,
    .add_input_wires = true,
    .col_0_poly_overhang_top = 0,
    .col_0_poly_overhang_bottom = ndiff_poly_rules.min_enclosure,
    .col_1_poly_overhang_top = ndiff_poly_rules.min_enclosure,
    .col_1_poly_overhang_bottom = 0,
    .col_2_poly_overhang_top = 0,
    .col_2_poly_overhang_bottom = ndiff_poly_rules.min_enclosure,
    .col_3_poly_overhang_top = ndiff_poly_rules.min_enclosure,
    .col_3_poly_overhang_bottom = 0,
    .input_0 = &left_input_0,
    .input_1 = &left_input_1,
    .input_2 = &left_input_2,
    .input_3 = &left_input_3,
    .input_x_padding = -2000,
    .input_y_padding = -2000
  };
  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout(mux2_params_n));

  left_input_0->set_is_pin(true);
  left_input_0->set_net("i0_l");
  left_input_1->set_is_pin(true);
  left_input_1->set_net("i1_l");
  left_input_2->set_is_pin(true);
  left_input_2->set_net("i2_l");
  left_input_3->set_is_pin(true);
  left_input_3->set_net("i3_l");

  Rectangle mux2_bounding_box = mux2_layout->GetBoundingBox();

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  mux2_layout->ResetOrigin();
  layout->AddLayout(*mux2_layout, "left");

  Mux2Parameters mux2_params_p = {
    .diff_layer_name = "pdiff.drawing",
    .diff_contact_layer_name = "pcon.drawing",
    .fet_0_width = 865,
    .fet_1_width = 865,
    .fet_2_width = 865,
    .fet_3_width = 865,
    .fet_4_width = 865,
    .fet_5_width = 865,
    .fet_0_length = 500,
    .fet_1_length = 500,
    .fet_2_length = 500,
    .fet_3_length = 500,
    .fet_4_length = 500,
    .fet_5_length = 500,
    .fet_4_5_offset_y = -200,
    .add_input_wires = true,
    .col_0_poly_overhang_top = pdiff_poly_rules.min_enclosure,
    .col_0_poly_overhang_bottom = 0,
    .col_1_poly_overhang_top = 0,
    .col_1_poly_overhang_bottom = pdiff_poly_rules.min_enclosure,
    .col_2_poly_overhang_top = pdiff_poly_rules.min_enclosure +
        polycon_rules.via_width + li_polycon_rules.via_overhang_wide,
    .col_2_poly_overhang_bottom = 0,
    .col_3_poly_overhang_top = 0,
    .col_3_poly_overhang_bottom = pdiff_poly_rules.min_enclosure,
    .input_0 = &right_input_0,
    .input_1 = &right_input_1,
    .input_2 = &right_input_2,
    .input_3 = &right_input_3,
    .input_x_padding = -2000,
    .input_y_padding = -2000
  };
  mux2_layout.reset(GenerateMux2Layout(mux2_params_p));

  right_input_0->set_is_pin(true);
  right_input_0->set_net("i0_r");
  right_input_1->set_is_pin(true);
  right_input_1->set_net("i1_r");
  right_input_2->set_is_pin(true);
  right_input_2->set_net("i2_r");
  right_input_3->set_is_pin(true);
  right_input_3->set_net("i3_r");

  int64_t nsdm_padding = diff_nsdm_rules.min_enclosure;
  int64_t psdm_padding = diff_psdm_rules.min_enclosure;
  int64_t nwell_padding = std::max(
      psdm_nwell_rules.min_enclosure + diff_psdm_rules.min_enclosure,
      pdiff_nwell_rules.min_enclosure);

  int64_t intra_spacing = 
      nwell_padding +   // Includes psdm_padding.
      std::max(nsdm_padding, ndiff_nwell_rules.min_separation);

  mux2_layout->FlipHorizontal();

  mux2_layout->MoveLowerLeftTo(Point(mux2_bounding_box.Width() + intra_spacing, 0));
  layout->AddLayout(*mux2_layout, "right");

  int64_t li_polycon_via_bulge_width =
      polycon_rules.via_width + 2 * li_polycon_rules.via_overhang_wide;
  int64_t li_polycon_via_bulge_length =
      polycon_rules.via_width + 2 * li_polycon_rules.via_overhang;
  int64_t poly_polycon_via_bulge_width = 
      polycon_rules.via_width + 2 * poly_polycon_rules.via_overhang_wide;
  int64_t poly_polycon_via_bulge_length =
      polycon_rules.via_width + 2 * poly_polycon_rules.via_overhang;

  // Connect the output.
  int64_t metal_width = li_rules.min_width;

  // Connect select-line polys.
  int64_t poly_width = poly_rules.min_width;
  int64_t bar_y_high = 0;
  int64_t bar_y_low = 0;
  int64_t hack_row_plus_1 = 0;
  int64_t hack_row_plus_2 = 0;
  int64_t hack_row_minus_1 = 0;
  {
    // Left column 2 poly to right column 3 poly.
    Point p_0 = layout->GetPoint("left.column_2_centre_bottom");
    Point p_3 = layout->GetPoint("right.column_3_centre_bottom");
    bar_y_high = layout->GetPoint("left.column_3_centre_top").y() +
        poly_width + poly_rules.min_separation;
    Point p_1 = Point(p_0.x(), bar_y_high);
    Point p_2 = Point(p_3.x(), bar_y_high);
    PolyLine line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(mux2_params_n.fet_4_length)},
        LineSegment {p_2, static_cast<uint64_t>(poly_width)},
        LineSegment {p_3, static_cast<uint64_t>(mux2_params_p.fet_4_length)}
    });

    layout->SetActiveLayerByName("poly.drawing");
    layout->AddPolyLine(line);

    layout->MakeVia("polycon.drawing", 
                    layout->GetPoint("left.column_3_centre_bottom_via"));

    Point actual_via = p_1 + Point(
        (-mux2_params_n.fet_4_length + polycon_rules.via_width) / 2 +
            poly_polycon_rules.min_enclosure,
        poly_rules.min_width / 2 - poly_polycon_rules.min_enclosure -
            polycon_rules.via_width / 2
    );

    layout->MakeVia("polycon.drawing", actual_via);
    p_0 = actual_via;
    p_1 = p_0 - Point(4 * polycon_rules.via_width / 2, 0);
    
    // HACK HACK HACK
    hack_row_plus_1 = layout->GetBoundingBox().upper_right().y() + 2000;
    p_2 = Point(p_1.x(), hack_row_plus_1);
    p_3 = Point(layout->GetBoundingBox().lower_left().x(), p_2.y());

    layout->SetActiveLayerByName("li.drawing");
    line = PolyLine({p_0, p_1, p_2, p_3});
    line.SetWidth(li_polycon_via_bulge_width);
    line.InsertBulge(p_0, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    line.InsertBulge(p_1, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    Polygon *polygon = layout->AddPolyLine(line);
    polygon->set_net("s0b");
    polygon->set_is_pin(true);
    //layout->MakePort("s0b", p_1, "li.drawing");
  }

  {
    // Left column 3 to right column 2.
    Point p_0 = layout->GetPoint("left.column_3_centre_bottom");
    Point p_3 = layout->GetPoint("right.column_2_centre_bottom");
    bar_y_low = std::min({
        p_0.y(),
        p_3.y(),
        layout->GetPoint("right.column_3_centre_bottom").y()}) -
            poly_width / 2 - poly_rules.min_separation;
    Point p_1 = Point(p_0.x(), bar_y_low);
    Point p_2 = Point(p_3.x(), bar_y_low);
    PolyLine line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(mux2_params_n.fet_5_length)},
        LineSegment {p_2, static_cast<uint64_t>(poly_width)},
        LineSegment {p_3, static_cast<uint64_t>(mux2_params_p.fet_5_length)}
    });

    layout->SetActiveLayerByName("poly.drawing");
    layout->AddPolyLine(line);

    Point nominal_via = layout->GetPoint("right.column_2_centre_top_via");
    Point actual_via = nominal_via + Point(
        (mux2_params_p.fet_4_length - polycon_rules.via_width) / 2 -
        poly_polycon_rules.min_enclosure,
        0);

    layout->MakeVia("polycon.drawing", actual_via);
    p_0 = actual_via;
    //p_1 = Point(p_0.x(), p_0.y() + polycon_rules.via_width);
    //p_2 = Point(p_1.x() + polycon_rules.via_width, p_1.y());
    p_1 = p_0 + Point(4 * polycon_rules.via_width / 2, 0);
    
    // HACK HACK HACK
    p_2 = Point(p_1.x(), hack_row_plus_1);
    p_3 = Point(layout->GetBoundingBox().upper_right().x(), p_2.y());

    layout->SetActiveLayerByName("li.drawing");
    line = PolyLine({p_0, p_1, p_2, p_3});
    line.SetWidth(li_polycon_via_bulge_width);
    line.InsertBulge(p_0, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    line.InsertBulge(p_1, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    Polygon *polygon = layout->AddPolyLine(line);
    polygon->set_net("s0");
    polygon->set_is_pin(true);
    //layout->MakePort("s0", p_1, "li.drawing");
  }

  {
    // Left column 1 poly to right column 0.
    Point p_0 = layout->GetPoint("left.column_1_centre_bottom");
    Point p_3 = layout->GetPoint("right.column_0_centre_bottom");
    int64_t new_bar_y_low = std::min({
        p_0.y(),
        p_3.y(),
        layout->GetPoint("right.column_1_centre_bottom").y()}) -
        poly_width / 2 - poly_rules.min_separation;
    bar_y_low = std::min(
        new_bar_y_low,
        bar_y_low - poly_width - poly_rules.min_separation);
    Point p_1 = Point(p_0.x(), bar_y_low);
    Point p_2 = Point(p_3.x(), bar_y_low);
    PolyLine line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(mux2_params_n.fet_4_length)},
        LineSegment {p_2, static_cast<uint64_t>(poly_width)},
        LineSegment {p_3, static_cast<uint64_t>(mux2_params_p.fet_4_length)}
    });

    Point actual_via = Point((p_1.x() + p_2.x()) / 2, bar_y_low);
    line.InsertBulge(
        actual_via,
        poly_polycon_via_bulge_width,
        poly_polycon_via_bulge_length);

    layout->SetActiveLayerByName("poly.drawing");
    layout->AddPolyLine(line);

    // The port needs to be on li.drawing.
    layout->MakeVia("polycon.drawing", actual_via);


    // HACK HACK HACK
    hack_row_minus_1 = layout->GetBoundingBox().lower_left().y() - 2000;
    p_0 = actual_via;
    p_1 = Point(p_0.x(), hack_row_minus_1);
    p_2 = Point(layout->GetBoundingBox().lower_left().x(), p_1.y());

    // HACK HACK HACK
    //layout->SetActiveLayerByName("li.drawing");
    //int64_t pour_side = std::max(
    //    li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    //Rectangle *rectangle = layout->AddSquare(actual_via, pour_side);
    //rectangle->set_net("s1");
    //rectangle->set_is_pin(true);
    layout->SetActiveLayerByName("li.drawing");
    line = PolyLine({p_0, p_1, p_2});
    line.SetWidth(li_polycon_via_bulge_width);
    line.InsertBulge(p_0, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    Polygon *polygon = layout->AddPolyLine(line);
    polygon->set_net("s1");
    polygon->set_is_pin(true);

    // FIXME(aryap): What port is this though?
    //layout->MakePort("s1", actual_via, "li.drawing");
  }

  {
    // Left column 0 poly to right column 1.
    Point p_0 = layout->GetPoint("left.column_0_centre_top");
    Point p_3 = layout->GetPoint("right.column_1_centre_top");
    int64_t new_bar_y_high = std::max(
        p_0.y() + poly_width / 2 + ndiff_poly_rules.min_enclosure,
        p_3.y() + poly_width / 2 + pdiff_poly_rules.min_enclosure);
    bar_y_high = std::max(
        new_bar_y_high,
        bar_y_high + poly_polycon_via_bulge_width / 2 +
            + poly_width / 2 + poly_rules.min_separation );
    Point p_1 = Point(p_0.x(), bar_y_high);
    Point p_2 = Point(p_3.x(), bar_y_high);
    PolyLine line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(mux2_params_n.fet_4_length)},
        LineSegment {p_2, static_cast<uint64_t>(poly_width)},
        LineSegment {p_3, static_cast<uint64_t>(mux2_params_p.fet_4_length)}
    });

    Point actual_via = Point((p_1.x() + p_2.x()) / 2, bar_y_high);
    line.InsertBulge(
        actual_via,
        poly_polycon_via_bulge_width,
        poly_polycon_via_bulge_length);

    layout->SetActiveLayerByName("poly.drawing");
    layout->AddPolyLine(line);

    // The port needs to be on li.drawing.
    layout->MakeVia("polycon.drawing", actual_via);

    // HACK HACK HACK
    hack_row_plus_2 = layout->GetBoundingBox().upper_right().y() + 2000;
    p_0 = actual_via;
    p_1 = Point(p_0.x(), hack_row_plus_2);
    p_2 = Point(layout->GetBoundingBox().lower_left().x(), p_1.y());

    //layout->SetActiveLayerByName("li.drawing");
    //int64_t pour_side = std::max(
    //    li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    //Rectangle *rectangle = layout->AddSquare(actual_via, pour_side);
    //rectangle->set_net("s1b");
    //rectangle->set_is_pin(true);
    layout->SetActiveLayerByName("li.drawing");
    line = PolyLine({p_0, p_1, p_2});
    line.SetWidth(li_polycon_via_bulge_width);
    line.InsertBulge(p_0, li_polycon_via_bulge_width, li_polycon_via_bulge_length);
    Polygon *polygon = layout->AddPolyLine(line);
    polygon->set_net("s1b");
    polygon->set_is_pin(true);

    // FIXME(aryap): This needs a port.
    // The port needs to be on li.drawing.
    // layout->MakePort("s0b",
    //                 layout->GetPoint("left.column_3_centre_bottom_via"),
    //                 "polycon.drawing");
    layout->MakeVia("polycon.drawing", 
                    layout->GetPoint("left.column_3_centre_bottom_via"));
  }

  {
    Point p_0 = layout->GetPoint("left.output");
    Point p_3 = layout->GetPoint("right.output");
    int64_t bar_y = std::max(
        layout->GetPoint("left.li_corner_ne_centre").y(),
        layout->GetPoint("right.li_corner_ne_centre").y()) +
        li_rules.min_separation + metal_width / 2;
    Point p_1 = Point(p_0.x(), bar_y);
    Point p_2 = Point(p_3.x(), bar_y);
    PolyLine line = PolyLine({p_0, p_1, p_2, p_3});
    line.SetWidth(metal_width);

    layout->MakeVia("ncon.drawing", p_0);
    layout->MakeVia("pcon.drawing", p_3);

    line.InsertBulge(
        p_0,
        ncon_rules.via_width + 2 * li_ncon_rules.via_overhang_wide,
        ncon_rules.via_width + 2 * li_ncon_rules.via_overhang);
    line.InsertBulge(
        p_3,
        pcon_rules.via_width + 2 * li_pcon_rules.via_overhang_wide,
        pcon_rules.via_width + 2 * li_pcon_rules.via_overhang);

    Point output_via_centre = Point((p_0.x() + p_3.x()) / 2, bar_y);
    line.InsertBulge(
        output_via_centre,
        mcon_rules.via_width + li_mcon_rules.via_overhang,
        mcon_rules.via_width + li_mcon_rules.via_overhang_wide);

    layout->SetActiveLayerByName("li.drawing");
    Polygon *polygon = layout->AddPolyLine(line);
    polygon->set_is_pin(true);
    polygon->set_net("z");

    // HACK HACK HACK
    p_0 = Point((output_via_centre.x() + p_3.x()) / 2, output_via_centre.y());
    p_1 = Point(p_0.x(), hack_row_plus_2);
    p_2 = Point(layout->GetBoundingBox().upper_right().x(), p_1.y());
    line = PolyLine({p_0, p_1, p_2});
    line.SetWidth(li_polycon_via_bulge_width);
    polygon = layout->AddPolyLine(line);

    polygon->set_is_pin(true);
    polygon->set_net("z");

    //layout->MakePort("z", output_via_centre, "mcon.drawing");
  }
  
  // Add diffusion qualifying layers, wells, etc.
  //
  // Left side is N.
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle({
      layout->GetPoint("left.diff_ll") - Point(
          nsdm_padding, nsdm_padding),
      layout->GetPoint("left.diff_ur") + Point(
          nsdm_padding, nsdm_padding)
  });

  // Right side is P, but note that that layout has been horizontally flipped
  // so the coordinates we use are also flipped.
  layout->SetActiveLayerByName("psdm.drawing");
  Rectangle *psdm = layout->AddRectangle({
      layout->GetPoint("right.diff_lr") - Point(
          psdm_padding, psdm_padding),
      layout->GetPoint("right.diff_ul") + Point(
          psdm_padding, psdm_padding)
  });

  // Add N-Well.
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle({
      layout->GetPoint("right.diff_lr") - Point(
          nwell_padding, nwell_padding),
      layout->GetPoint("right.diff_ul") + Point(
          nwell_padding, nwell_padding)
  });

  // Add PR boundary.
  Rectangle bounding_box = layout->GetBoundingBox();
  // 7t gf180mcu standard cell: 3920;
  int64_t height = bounding_box.Height();
  int64_t padding_left = nwell_padding;
  int64_t padding_right = nwell_padding;
  layout->SetActiveLayerByName("areaid.standardrc");
  layout->AddRectangle(
      Rectangle(
          {bounding_box.lower_left().x() - padding_left,
           bounding_box.centre().y() - height / 2},
          {bounding_box.upper_right().x() + padding_right,
           bounding_box.centre().y() + height / 2}));

  return layout.release();
}

}   // namespace bfg
}   // namespace atoms
