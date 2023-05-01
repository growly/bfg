#include "sky130_mux.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "atom.h"
#include "../circuit/wire.h"
#include "../cell.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/line_segment.h"
#include "../geometry/polygon.h"
#include "../geometry/poly_line.h"
#include "../poly_line_inflator.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::PolyLine;
using ::bfg::geometry::LineSegment;
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

  circuit::Wire S0 = circuit->AddSignal("S0");
  circuit::Wire S0B = circuit->AddSignal("S0B");
  circuit::Wire S1 = circuit->AddSignal("S1");
  circuit::Wire S1B = circuit->AddSignal("S1B");
  circuit::Wire S2 = circuit->AddSignal("S2");
  circuit::Wire S2B = circuit->AddSignal("S2B");

  circuit::Wire X0 = circuit->AddSignal("X0");
  circuit::Wire X1 = circuit->AddSignal("X1");
  circuit::Wire X2 = circuit->AddSignal("X2");
  circuit::Wire X3 = circuit->AddSignal("X3");
  circuit::Wire X4 = circuit->AddSignal("X4");
  circuit::Wire X5 = circuit->AddSignal("X5");
  circuit::Wire X6 = circuit->AddSignal("X6");
  circuit::Wire X7 = circuit->AddSignal("X7");

  circuit::Wire Z = circuit->AddSignal("Z");

  //circuit::Wire VPWR = circuit->AddSignal("VPWR");
  //circuit::Wire VGND = circuit->AddSignal("VGND");
  //circuit::Wire VPB = circuit->AddSignal("VPB");
  //circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(S0);
  circuit->AddPort(S0B);
  circuit->AddPort(S1);
  circuit->AddPort(S1B);
  circuit->AddPort(S2);
  circuit->AddPort(S2B);
  circuit->AddPort(X0);
  circuit->AddPort(X1);
  circuit->AddPort(X2);
  circuit->AddPort(X3);
  circuit->AddPort(X4);
  circuit->AddPort(X5);
  circuit->AddPort(X6);
  circuit->AddPort(X7);
  circuit->AddPort(Z);

  return circuit.release();
}

namespace {

std::optional<Polygon> InflatePolyLine(const PhysicalPropertiesDatabase &db,
                                       const PolyLine line) {
  PolyLineInflator inflator(db);
  return inflator.InflatePolyLine(line);
}

Polygon InflatePolyLineOrDie(const PhysicalPropertiesDatabase &db,
                             const PolyLine line) {
  std::optional<Polygon> polygon = InflatePolyLine(db, line);
  LOG_IF(FATAL, !polygon) << "Could not inflate polyline: " << line;
  return *polygon;
}

Polygon *AddElbowPath(
    const PhysicalPropertiesDatabase &db,
    const Point &start,
    const Point &end,
    const int64_t width,
    const int64_t start_encap_width,
    const int64_t start_encap_length,
    const int64_t end_encap_width,
    const int64_t end_encap_length,
    bfg::Layout *layout) {
  // How to avoid constantly copying this?
  Point elbow = {start.x(), end.y()};
  PolyLine line = PolyLine({start, elbow, end});
  line.SetWidth(width);
  line.InsertBulge(start, start_encap_width, start_encap_length);
  line.InsertBulge(end, end_encap_width, end_encap_length);
  return layout->AddPolygon(InflatePolyLineOrDie(db, line));
}

Polygon *AddElbowPathBetweenLayers(
    const PhysicalPropertiesDatabase &db,
    const Point &start,
    const Point &end,
    const std::string &start_layer,
    const std::string &path_layer,
    const std::string &end_layer,
    bfg::Layout *layout) {
  const int64_t start_encap_width = db.Rules(start_layer).via_width +
      2 * db.Rules(start_layer, path_layer).via_overhang_wide;
  const int64_t start_encap_length = db.Rules(start_layer).via_width +
      2 * db.Rules(start_layer, path_layer).via_overhang;
  const int64_t end_encap_width = db.Rules(end_layer).via_width +
      2 * db.Rules(end_layer, path_layer).via_overhang_wide;
  const int64_t end_encap_length = db.Rules(end_layer).via_width +
      2 * db.Rules(end_layer, path_layer).via_overhang;
  // TODO(aryap): This is a hack workaround for the problem that frequently
  // occurs when an elbows start and end encapsulation pours are too close
  // together, and a divet is created between them that violates the min.
  // separation distance for the layer. This is most elegantly solved as an
  // automatic feature of the PolyLine which should automatically widen widths
  // to avoid divets.
  const int64_t width = std::max({
      db.Rules(path_layer).min_width, end_encap_width});
  LOG(INFO) << "Adding elbow (" << start_layer << ") " << start << " -("
            << path_layer << ")-> " << end << " (" << end_layer << ") "
            << width << " "
            << start_encap_width << " "
            << start_encap_length << " "
            << end_encap_width << " "
            << end_encap_length;
  layout->SetActiveLayerByName(path_layer);
  Polygon *path = AddElbowPath(
      db,
      start,
      end,
      width,
      start_encap_width,
      start_encap_length,
      end_encap_width,
      end_encap_length,
      layout);
  layout->RestoreLastActiveLayer();
  return path;
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
  PolyLine line = PolyLine({start, end});
  line.set_layer(db.GetLayer(path_layer));
  line.set_min_separation(db.Rules(path_layer).min_separation); 
  line.SetWidth(width);

  int64_t start_via_side = db.Rules(start_layer).via_width;
  int64_t start_via_encap_width =
      start_via_side + 2 * db.Rules(path_layer, start_layer).via_overhang_wide;
  int64_t start_via_encap_length =
      start_via_side + 2 * db.Rules(path_layer, start_layer).via_overhang;

  int64_t end_via_side = db.Rules(end_layer).via_width;
  int64_t end_via_encap_width =
      end_via_side + 2 * db.Rules(path_layer, end_layer).via_overhang_wide;
  int64_t end_via_encap_length =
      end_via_side + 2 * db.Rules(path_layer, end_layer).via_overhang;

  // Simplify the shape if the start and end point are the same: make it the
  // biggest square that satisifies the minimum encap rules for below/above
  // layers.
  if (start == end) {
    int64_t bulge_side = std::max({
        start_via_encap_width,
        end_via_encap_width,
        start_via_encap_length,
        end_via_encap_length
    });
    line.InsertBulge(start, bulge_side, bulge_side);
  } else {
    line.InsertBulge(start, start_via_encap_width, start_via_encap_length);
    line.InsertBulge(end, end_via_encap_width, end_via_encap_length);
  }

  Polygon polygon = InflatePolyLineOrDie(db, line);
  layout->SetActiveLayerByName(path_layer);
  Polygon *added = layout->AddPolygon(polygon);
  layout->RestoreLastActiveLayer();
  return added;
}

Polygon *ConnectDiffToMet1(
  const PhysicalPropertiesDatabase &db,
  const Point &diff_point,
  const Point &met1_point,
  const std::string &diff_contact,
  bfg::Layout *layout) {
  layout->MakeVia(diff_contact, diff_point);
  Polygon *added = StraightLineBetweenLayers(db,
                                             diff_point,
                                             met1_point,
                                             diff_contact,
                                             "li.drawing",
                                             "mcon.drawing",
                                             layout);
  layout->MakeVia("mcon.drawing", met1_point);
  return added;
}

enum BulgeDirection {
  LEFT,
  CENTRE,
  RIGHT
};

Polygon *ConnectPolyToMet1(
    const PhysicalPropertiesDatabase &db,
    const Point &poly_point,
    const Point &met1_point,
    const std::string &poly_contact,
    bool rotate,
    const BulgeDirection &bulge_direction,
    bfg::Layout *layout) {
  int64_t via_side = db.Rules(poly_contact).via_width;
  int64_t via_overhang_wide =
      db.Rules("poly.drawing", poly_contact).via_overhang_wide;
  int64_t via_encap_width = via_side + 2 * via_overhang_wide;
  int64_t via_encap_length =
      via_side + 2 * db.Rules("poly.drawing", poly_contact).via_overhang;

  int64_t poly_width = db.Rules("poly.drawing").min_width;

  layout->SetActiveLayerByName("poly.drawing");

  Point poly_via_centre = poly_point;

  switch (bulge_direction) {
    case BulgeDirection::LEFT:
      LOG(INFO) << "translating left";
      poly_via_centre -= Point(via_overhang_wide, 0);
      break;
    case BulgeDirection::CENTRE:
      // Do nothing; pour starts centred.
      break;
    case BulgeDirection::RIGHT:
      LOG(INFO) << "translating right";
      poly_via_centre += Point(via_overhang_wide, 0);
      break;
  }
  layout->AddRectangle(
      {poly_via_centre - Point(via_encap_width / 2, via_encap_length / 2),
       poly_via_centre + Point(via_encap_width / 2, via_encap_length / 2)});

  layout->RestoreLastActiveLayer();

  layout->MakeVia(poly_contact, poly_via_centre);

  Polygon *added = AddElbowPathBetweenLayers(
      db,
      rotate ? poly_via_centre : met1_point,
      rotate ? met1_point : poly_via_centre,
      rotate ? poly_contact : "mcon.drawing",
      "li.drawing",
      rotate ? "mcon.drawing" : poly_contact,
      layout);

  layout->MakeVia("mcon.drawing", met1_point);

  return added;
}

Polygon *ConnectPolyToMet1(
    const PhysicalPropertiesDatabase &db,
    const Point &poly_point,
    const Point &met1_point,
    const std::string &poly_contact,
    bfg::Layout *layout) {
  return ConnectPolyToMet1(
      db,
      poly_point,
      met1_point,
      poly_contact,
      false,
      BulgeDirection::CENTRE,
      layout);
}

// The mapping is a triple of (named point, connecting column poly line, and
// whether or not the jog should be rotated).
void ConnectNamedPointsToColumns(
    const PhysicalPropertiesDatabase &db,
    std::vector<
        std::tuple<std::string, PolyLine*, bool>> mapping,
    const std::string &poly_contact,
    bool point_up,
    bfg::Layout *layout) {
  int64_t mcon_via_side = db.Rules("mcon.drawing").via_width;
  int64_t mcon_encap_width = mcon_via_side + 2 * db.Rules(
      "li.drawing", "mcon.drawing").via_overhang_wide;
  int64_t mcon_encap_length = mcon_via_side + 2 * db.Rules(
      "li.drawing", "mcon.drawing").via_overhang;

  BulgeDirection bulge_direction = BulgeDirection::LEFT;
  for (size_t i = 0; i < mapping.size(); ++i) {
    const auto &triple = mapping[i];
    std::string name;
    PolyLine *poly_line;
    bool rotate;
    std::tie(name, poly_line, rotate) = triple;
    // NOTE(aryap): Apparently in C++17 you can do:
    // const auto [name, poly_line, rotate] = triple;

    Point source = layout->GetPoint(name);
    Point connection = {poly_line->start().x(), source.y()};

    if (point_up) {
      connection.set_y(source.y() + mcon_via_side);
    } else {
      connection.set_y(source.y() - mcon_via_side);
    }
    ConnectPolyToMet1(
        db, source, connection, poly_contact, rotate, bulge_direction, layout);
    rotate = !rotate;
    bulge_direction = bulge_direction == BulgeDirection::LEFT ?
        BulgeDirection::RIGHT : bulge_direction == BulgeDirection::RIGHT ?
        BulgeDirection::LEFT : BulgeDirection::CENTRE;

    poly_line->InsertBulge(connection, mcon_encap_width, mcon_encap_length);
  }
}

void ConnectNamedPointsToColumn(
    const PhysicalPropertiesDatabase &db,
    const std::vector<std::string> named_points,
    PolyLine *poly_line,
    const std::string &poly_contact,
    bfg::Layout *layout) {
  int64_t mcon_via_side = db.Rules("mcon.drawing").via_width;
  int64_t mcon_encap_width = mcon_via_side + 2 * db.Rules(
      "li.drawing", "mcon.drawing").via_overhang_wide;
  int64_t mcon_encap_length = mcon_via_side + 2 * db.Rules(
      "li.drawing", "mcon.drawing").via_overhang;

  bool point_up = true;
  for (const std::string &name : named_points) {
    Point source = layout->GetPoint(name);
    Point connection = {poly_line->start().x(), source.y()};

    if (point_up) {
      connection.set_y(connection.y() + mcon_via_side);
    } else {
      connection.set_y(connection.y() - mcon_via_side);
    }
    ConnectPolyToMet1(
        db, source, connection, poly_contact, layout);

    poly_line->InsertBulge(connection, mcon_encap_width, mcon_encap_length);
    point_up = !point_up;
  }
}

void GenerateOutput2To1Mux(
    const PhysicalPropertiesDatabase &db,
    int64_t left_left_metal_column_x,
    int64_t left_right_metal_column_x,
    int64_t right_left_metal_column_x,
    int64_t right_right_metal_column_x,
    int64_t met1_top_y,
    int64_t met1_bottom_y,
    bfg::Layout *main_layout) {
  const auto &li_rules = db.Rules("li.drawing");
  const auto &ncon_rules = db.Rules("ncon.drawing");
  const auto &pcon_rules = db.Rules("pcon.drawing");
  const auto &polycon_rules = db.Rules("polycon.drawing");
  const auto &poly_rules = db.Rules("poly.drawing");
  const auto &mcon_rules = db.Rules("mcon.drawing");
  const auto &met1_rules = db.Rules("met1.drawing");
  const auto &li_ncon_rules = db.Rules("li.drawing", "ncon.drawing");
  const auto &li_pcon_rules = db.Rules("li.drawing", "pcon.drawing");
  const auto &li_polycon_rules = db.Rules("li.drawing", "polycon.drawing");
  const auto &ndiff_ncon_rules = db.Rules("ndiff.drawing", "ncon.drawing");
  const auto &ndiff_polycon_rules =
      db.Rules("ndiff.drawing", "polycon.drawing");
  const auto &pdiff_pcon_rules = db.Rules("pdiff.drawing", "pcon.drawing");
  const auto &pdiff_polycon_rules =
      db.Rules("pdiff.drawing", "polycon.drawing");
  const auto &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");
  const auto &poly_polycon_rules = db.Rules("poly.drawing", "polycon.drawing");
  const auto &poly_ncon_rules = db.Rules("poly.drawing", "ncon.drawing");
  const auto &poly_pcon_rules = db.Rules("poly.drawing", "pcon.drawing");
  const auto &met1_mcon_rules = db.Rules("met1.drawing", "mcon.drawing");

  int64_t stage_2_mux_fet_0_width = db.ToInternalUnits(640);
  int64_t stage_2_mux_fet_1_width = db.ToInternalUnits(640);

  //int64_t stage_2_mux_nfet_0

  int64_t poly_contact_to_ndiff =
      ndiff_polycon_rules.min_separation + polycon_rules.via_width / 2;
  int64_t poly_contact_to_pdiff =
      pdiff_polycon_rules.min_separation + polycon_rules.via_width / 2;

  int64_t via_centre_to_poly_edge =
      polycon_rules.via_width / 2 + poly_polycon_rules.min_separation;

  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  int64_t diff_wing = ncon_rules.via_width +
      poly_ncon_rules.min_separation +
      ndiff_ncon_rules.min_enclosure;
  int64_t poly_overhang = li_rules.min_width + li_rules.min_separation;
  int64_t poly_pitch = poly_rules.min_pitch;
  int64_t poly_gap = std::max(
      poly_pitch - poly_rules.min_width,
      std::max(ncon_rules.via_width, pcon_rules.via_width) +
          2 * std::max(poly_ncon_rules.min_separation,
                       poly_pcon_rules.min_separation));
  Point gap_top = main_layout->GetPoint("upper_left.column_2_centre_bottom");
  Point gap_bottom = main_layout->GetPoint("lower_left.column_2_centre_bottom");
  // 2 * poly_overhang + stage_2_mux_fet_0_width;
  int64_t height = (
      gap_top.y() - gap_bottom.y()) - 2 * poly_rules.min_separation;
  Point poly_ur = Point(gap_top.x() + poly_rules.min_width / 2,
                        ((gap_top + gap_bottom).y() + height) / 2);

  // The limiting constraint on the output mux is the number of horizontal li
  // tracks needed to connect to the poly and metal. If we start with one
  // bounding edge, the closest li1 pour in the lower left mux2, we can build y
  // upwards and place the li pours. Then we can place the diffusion pour so
  // that the poly encapsulation (larger than the gate length) doesn't overlap
  // the transistor diffusion.
 
  Point poly_ll = poly_ur - Point(poly_rules.min_width, height);
  layout->SetActiveLayerByName("poly.drawing");
  Rectangle *template_left_poly = layout->AddRectangle({poly_ll, poly_ur});
  Rectangle left_left_poly = *template_left_poly;

  poly_ur.Translate({poly_pitch, 0});
  poly_ll.Translate({poly_pitch, 0});
  layout->SetActiveLayerByName("poly.drawing");
  Rectangle *template_right_poly = layout->AddRectangle({poly_ll, poly_ur});
  Rectangle left_right_poly = *template_right_poly;

  // Compute the offset required to align the polys.
  int64_t target_x =
      main_layout->GetPoint("upper_right.column_3_centre_bottom_via").x();
  int64_t offset_x = template_right_poly->centre().x();
  layout->Translate(Point(target_x - offset_x, bb.lower_left().y()));

  // After transformation, the shapes in the original are now:
  Rectangle right_left_poly = *template_left_poly;
  Rectangle right_right_poly = *template_right_poly;

  int64_t via_side = li_rules.via_width;
  int64_t li_pitch_optimistic = li_rules.min_width / 2 +
      std::max(li_rules.min_width / 2, mcon_rules.via_width / 2) +
      li_rules.min_separation +
      li_mcon_rules.via_overhang_wide;
  int64_t pcon_via_encap_side = li_pcon_rules.via_overhang_wide;

  main_layout->AddLayout(*layout, "output_mux_right");

  Point output_mux_ul_elbow_connect =
      Point(left_left_metal_column_x, input_0.y());
  Polygon *left_input_li_pour = nullptr;
  // Connect the outputs of the outer muxes to the final stage mux.
  // Upper left:
  //    p_2       (from upper left mux output)
  //      +------+
  //      |      |
  //      |      + p_0
  //      |
  //      |
  //      |
  //      + p_3
  //      |
  //      |
  //    +-+ p_4
  //    |
  //    + p_6
  {
    const auto &li_dcon_rules = db.Rules("li.drawing", "ncon.drawing");
    const auto &dcon_rules = db.Rules("ncon.drawing");
    int64_t via_side = dcon_rules.via_width;
    int64_t li_mcon_via_encap_width =
        via_side + 2 * li_mcon_rules.via_overhang_wide;
    int64_t li_mcon_via_encap_length =
        via_side + 2 * li_mcon_rules.via_overhang;
    int64_t li_dcon_via_encap_width =
        via_side + 2 * li_dcon_rules.via_overhang_wide;
    int64_t li_dcon_via_encap_length =
        via_side + 2 * li_dcon_rules.via_overhang;

    Point source = main_layout->GetPoint("upper_left.output");
    main_layout->MakeVia("licon.drawing", source);
    Point destination = main_layout->GetPoint("output_mux_left.left_input");

    Point p_0 = source;
    Point p_2 = Point(
        left_left_metal_column_x,
        main_layout->GetPoint("upper_left.via_3_1_ur").y() + (
            li_rules.min_separation +
            li_mcon_rules.via_overhang_wide +
            mcon_rules.via_width / 2
        ));
    Point p_6 = destination;
    Point p_3 = Point(
        left_left_metal_column_x,
        main_layout->GetPoint("upper_left.li_corner_se_centre").y() - (
            li_rules.min_separation + li_mcon_rules.via_overhang +
            mcon_rules.via_width / 2 + li_rules.min_width / 2));
    Point p_4 = Point(p_3.x(), (p_6.y() + p_3.y()) / 2);
    Point p_5 = Point(p_6.x(), p_4.y());

    main_layout->SetActiveLayerByName("li.drawing");
    PolyLine left_input_jog = PolyLine(p_3, {
        LineSegment {p_4, static_cast<uint64_t>(li_mcon_via_encap_width)},
        LineSegment {p_5, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_6, static_cast<uint64_t>(li_rules.min_width)}
    });
    left_input_jog.InsertBulge(p_3,
                               li_mcon_via_encap_width,
                               li_mcon_via_encap_length);
    left_input_jog.InsertBulge(p_6,
                               li_dcon_via_encap_width,
                               li_dcon_via_encap_length);
    left_input_li_pour = main_layout->AddPolygon(
        InflatePolyLineOrDie(db, left_input_jog));

    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        p_2, p_3,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);
    main_layout->MakeVia("mcon.drawing", p_3);

    main_layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        p_0, p_2,
        "ncon.drawing", "li.drawing", "mcon.drawing",
        main_layout);
  }


  Point output_mux_lr_elbow_top;
  Point output_mux_lr_elbow_bottom;
  {
    //            destination
    //              + p_5
    //              |
    // p_3  +-------/
    //      |
    //      |  + p_0 (from lower left mux output)
    //      |  | source
    // p_2  +--/
    //

    Point p_0 = main_layout->GetPoint("lower_left.output");
    main_layout->MakeVia("licon.drawing", p_0);

    Point p_5 = main_layout->GetPoint("output_mux_left.right_input");

    // Jog up a bit and connect to the metal track.
    Point p_2 = Point(
        left_right_metal_column_x, p_0.y() - li_rules.min_width);
    Point p_3 = Point(
        left_right_metal_column_x,
        // This could also be something like
        //  main_layout->GetPort("output").centre();
        // if it had been added as a port.
        main_layout->GetPoint("lower_left.li_corner_se_centre").y()
            + li_pitch_optimistic);

    output_mux_lr_elbow_top = p_3;
          
    main_layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        p_5, p_3,
        "licon.drawing", "li.drawing", "mcon.drawing",
        main_layout);

    main_layout->MakeVia("mcon.drawing", p_3);
    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        p_3, p_2,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);
    main_layout->MakeVia("mcon.drawing", p_2);

    main_layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        p_0, p_2,
        "licon.drawing", "li.drawing", "mcon.drawing",
        main_layout);
  }

  Polygon *left_right_poly_li_pour = nullptr;
  Polygon *top_poly_connector = nullptr;
  {
    // Connect the signal that selects the output of the bottom-left mux
    // structure.
    //
    //     + p_0
    //     |
    //     |
    //     |
    //     |
    //     |
    //     +---+ p_2
    //     p_1
    //
    Point p_0 = Point(left_right_metal_column_x, met1_top_y);
    int64_t lower_left_poly_connect_y =
        main_layout->GetPoint("upper_left.li_corner_se_centre").y() - (
            li_rules.min_separation + li_mcon_rules.via_overhang +
            mcon_rules.via_width / 2 + li_rules.min_width / 2);
    Point lower_left_poly_connect = Point(
        left_right_poly.centre().x(), lower_left_poly_connect_y);
    Point p_1 = Point(
        p_0.x(), lower_left_poly_connect_y);
    main_layout->SetActiveLayerByName("met1.drawing");
    top_poly_connector = StraightLineBetweenLayers(
        db,
        p_0, p_1,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);

    main_layout->SetActiveLayerByName("li.drawing");
    left_right_poly_li_pour = ConnectPolyToMet1(
        db,
        Point(left_right_poly.centre().x(),
              p_1.y()),
        p_1,
        "polycon.drawing",
        main_layout);
  }

  {
    // Connect the signal that selects the output of the upper-left mux
    // structure to the appropriate gate (poly).
    Point met1_p1 = Point(left_left_metal_column_x, met1_bottom_y);
    int64_t upper_left_poly_connect_y = std::max({
        // Can't be further than poly_contact_to_ndiff from diffusion.
        //fet_0_diff->lower_left().y() - poly_contact_to_ndiff,
        // Can't be too close to the bottom end of the poly.
        left_left_poly.lower_left().y() + (
            via_side / 2
            + poly_polycon_rules.min_separation
            + li_rules.min_width / 2),
        output_mux_lr_elbow_top.y() + li_pitch_optimistic
    });
    Point upper_left_poly_connect = Point(
        left_left_poly.centre().x(), upper_left_poly_connect_y);
    Point met1_p0 = Point(
        met1_p1.x(), upper_left_poly_connect.y());
    main_layout->SetActiveLayerByName("met1.drawing");
    Polygon *bottom_poly_connector = StraightLineBetweenLayers(
        db,
        met1_p1, met1_p0,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);

    main_layout->SetActiveLayerByName("li.drawing");
    Polygon *left_left_poly_li_pour = ConnectPolyToMet1(
        db,
        Point(left_left_poly.centre().x(),
              met1_p0.y()),
        met1_p0,
        "polycon.drawing",
        main_layout);
  }

  {
    Point source = main_layout->GetPoint("upper_right.output");
    main_layout->MakeVia("licon.drawing", source);
    Point destination = main_layout->GetPoint("output_mux_right.left_input");
    Point met1_p0 = Point(
        right_left_metal_column_x,
        main_layout->GetPoint("upper_right.li_corner_se_centre").y()
            - li_pitch_optimistic - pcon_via_encap_side);
    Point met1_p1 = Point(right_left_metal_column_x, source.y());
    output_mux_ul_elbow_connect = met1_p0;

    main_layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        destination, met1_p0,
        "licon.drawing", "li.drawing", "mcon.drawing",
        main_layout);

    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_p1, met1_p0,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);
    main_layout->MakeVia("mcon.drawing", met1_p0);

    main_layout->SetActiveLayerByName("li.drawing");
    ConnectDiffToMet1(
        db, source, met1_p1, "ncon.drawing", main_layout);
  }

  //    +
  //    |
  //    \-------_
  //            |
  //            |
  //            |
  //        +--+
  //  (from lower right mux output)
  {
    Point source = main_layout->GetPoint("lower_right.output");
    main_layout->MakeVia("licon.drawing", source);
    Point destination = main_layout->GetPoint("output_mux_right.right_input");
    Point met1_p0 = Point(
        right_right_metal_column_x,
        main_layout->GetPoint("lower_right.li_corner_se_centre").y()
            + li_pitch_optimistic);
    output_mux_lr_elbow_top = met1_p0;
          
    Point met1_p1 = Point(right_right_metal_column_x, source.y());
    main_layout->SetActiveLayerByName("li.drawing");
    AddElbowPathBetweenLayers(
        db,
        destination, met1_p0,
        "licon.drawing", "li.drawing", "mcon.drawing",
        main_layout);
    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_p1, met1_p0,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);
    main_layout->MakeVia("mcon.drawing", met1_p0);

    main_layout->SetActiveLayerByName("li.drawing");
    ConnectDiffToMet1(
        db, source, met1_p1, "ncon.drawing", main_layout);
  }

  {
    // Connect the signal that selects the output of the bottom-left mux
    // structure to the right gate (poly).
    Point met1_p1 = Point(right_right_metal_column_x, met1_top_y);
    Point lower_left_poly_connect = Point(
        right_right_poly.centre().x(),
        std::min(
            right_right_poly.upper_right().y() - (via_side / 2
                + poly_polycon_rules.min_separation
                + li_rules.min_width / 2),
            output_mux_ul_elbow_connect.y() - li_pitch_optimistic
        ));
    Point met1_p0 = Point(
        met1_p1.x(), lower_left_poly_connect.y());
    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_p1, met1_p0,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);

    main_layout->SetActiveLayerByName("li.drawing");
    ConnectDiffToMet1(
        db,
        Point(right_right_poly.centre().x(),
              met1_p0.y()),
        met1_p0,
        "ncon.drawing",
        main_layout);
  }

  {
    // Connect the signal that selects the output of the upper-left mux
    // structure.
    Point met1_p1 = Point(right_left_metal_column_x, met1_bottom_y);
    upper_left_poly_connect = Point(
        right_left_poly.centre().x(),
        std::min(
            right_left_poly.upper_right().y() + (via_side / 2
                + poly_polycon_rules.min_separation
                + li_rules.min_width / 2),
            output_mux_lr_elbow_top.y() + li_pitch_optimistic
        ));
    Point met1_p0 = Point(
        met1_p1.x(), upper_left_poly_connect.y());
    main_layout->SetActiveLayerByName("met1.drawing");
    StraightLineBetweenLayers(
        db,
        met1_p1, met1_p0,
        "mcon.drawing", "met1.drawing", "mcon.drawing",
        main_layout);

    main_layout->SetActiveLayerByName("li.drawing");
    Polygon *right_right_poly_li_pour = ConnectPolyToMet1(
        db,
        Point(right_left_poly.centre().x(),
              met1_p0.y()),
        met1_p0,
        "polycon.drawing",
        main_layout);
  }

  // Draw poly and diffusion (i.e. the transistors) for four transistors.
  //
  // left left               right left
  //            left right             right right
  // +---------+---------+   +---------+---------+
  // | nfet 0  | nfet 1  |   | pfet 0  | pfet 1  |
  // +---------+---------+   +---------+---------+
  //        (N-fets)               (P-fets)
  //
  // fet 0:
  layout->SetActiveLayerByName("ndiff.drawing");
  Point diff_ur = Point(
      poly_ur.x() + poly_gap / 2,
      (poly_ur.y() + poly_ll.y() + stage_2_mux_fet_0_width) / 2);
  Point diff_ll = Point(
      poly_ll.x() - diff_wing, diff_ur.y() - stage_2_mux_fet_0_width);
  Rectangle *fet_0_diff = layout->AddRectangle({diff_ll, diff_ur});

  // fet 1:
  diff_ll.set_x(diff_ur.x());
  diff_ur = Point(
      poly_ur.x() + diff_wing,
      (poly_ur.y() + poly_ll.y() + stage_2_mux_fet_1_width) / 2);
  diff_ll.set_y(diff_ur.y() - stage_2_mux_fet_1_width);
  layout->SetActiveLayerByName("pdiff.drawing");
  Rectangle *fet_1_diff = layout->AddRectangle({diff_ll, diff_ur});

  // Name input and output via points.
  Point input_0 = Point(
      fet_0_diff->lower_left().x() + diff_wing / 2,
      fet_0_diff->upper_right().y() - via_centre_to_poly_edge);
  layout->SavePoint("left_input", input_0);
  layout->MakeVia("licon.drawing", input_0);

  Point input_1 = Point(
      fet_1_diff->upper_right().x() - diff_wing / 2,
      fet_1_diff->lower_left().y() + via_centre_to_poly_edge);
  layout->SavePoint("right_input", input_1);
  layout->MakeVia("licon.drawing", input_1);

  Point output = Point(
      fet_1_diff->lower_left().x(),
      (fet_1_diff->upper_right().y() + std::max(
           fet_0_diff->lower_left().y(), fet_1_diff->lower_left().y())) / 2);
  layout->SavePoint("output", output);
  layout->MakeVia("licon.drawing", output);

  // FIXME(aryap): Do not copy and flip this to align; the P-fets have to be
  // sized independently of the N-fets.

  // Add the first side of the mux back to the main layout.
  main_layout->AddLayout(*layout, "output_mux_left");
  Rectangle bb = layout->GetBoundingBox();

  // Flip and shift the layout to create the p-side.
  layout->MirrorY();
  layout->ResetOrigin();

  // Connect the P- and N-MOS pass gate outputs.
  Point left = main_layout->GetPoint("output_mux_left.output");
  Point right = main_layout->GetPoint("output_mux_right.output");

  // Find the bounding box for the li pour around the output via.
  // TODO(growly): It would be nice to ask the Polygon for its width at a given
  // y, or height at a given x.
  // TODO(growly): It would be nice to ask what the maximum available
  // size/position is for a rectangle within a given area on a given layer,
  // knowing the minimum padding required from other shapes on that layer!
  {
    int64_t li_centre_to_edge_x =
        li_rules.min_separation + li_rules.min_width / 2;
    int64_t min_x = 
        left_input_li_pour->GetBoundingBox().upper_right().x() +
        li_rules.min_separation;
    int64_t max_x = main_layout->GetPoint(
        "output_mux_left.right_input").x() - li_centre_to_edge_x;

    int64_t li_centre_to_edge_y =
        li_centre_to_edge_x + li_mcon_rules.via_overhang_wide;
    int64_t min_y = left_left_poly_li_pour->GetBoundingBox().upper_right().y() +
        li_rules.min_separation;
    int64_t max_y = left_right_poly_li_pour->GetBoundingBox().lower_left().y() -
        li_rules.min_separation;

    main_layout->SetActiveLayerByName("li.drawing");
    Rectangle *left_pour = main_layout->AddRectangle(
        {{min_x, min_y}, {max_x, max_y}});
    Rectangle *right_pour = main_layout->AddRectangle(*left_pour);
    int64_t right_shift =
        main_layout->GetPoint("output_mux_right.output").x()
        - main_layout->GetPoint("output_mux_left.output").x();
    right_pour->Translate(Point(right_shift, 0));

    // Add mcon vias:
    main_layout->MakeVia("mcon.drawing", left_pour->centre());
    main_layout->MakeVia("mcon.drawing", right_pour->centre());

    min_y = bottom_poly_connector->GetBoundingBox().upper_right().y()
        + met1_rules.min_separation;
    max_y = top_poly_connector->GetBoundingBox().lower_left().y()
        - met1_rules.min_separation;

    main_layout->SetActiveLayerByName("met1.drawing");
    left_pour = main_layout->AddRectangle({{min_x, min_y}, {max_x, max_y}});
    right_pour = main_layout->AddRectangle(*left_pour);
    right_pour->Translate(Point(right_shift, 0));

    // Add via to met2.
    main_layout->MakeVia("via1.drawing", left_pour->centre());
    main_layout->MakeVia("via1.drawing", right_pour->centre());
    main_layout->SetActiveLayerByName("met2.drawing");
    Polygon *met2_bar = StraightLineBetweenLayers(
        db,
        left_pour->centre(), right_pour->centre(),
        "via1.drawing", "met2.drawing", "via1.drawing",
        main_layout);
    main_layout->SavePoint(
        "final_output",
        met2_bar->GetBoundingBox().centre());

    Rectangle met2_bb = met2_bar->GetBoundingBox();
    main_layout->AddPort(geometry::Port(
        met2_bb.centre(), met2_bb.Height(), met2_bb.Height(),
        met2_bar->layer(), "Z"));
  }
}

}  // namespace

bfg::Layout *Sky130Mux::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const IntraLayerConstraints &poly_rules = db.Rules("poly.drawing");

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  std::unique_ptr<bfg::Layout> mux4_layout(
      new bfg::Layout(design_db_->physical_db()));

  const std::string poly_contact = "polycon.drawing";

  Mux2Parameters mux2_params_n = {
    .diff_layer_name = "ndiff.drawing",
    .diff_contact_layer_name = "ncon.drawing",
    .fet_0_width = db.ToInternalUnits(450),
    .fet_1_width = db.ToInternalUnits(450),
    .fet_2_width = db.ToInternalUnits(450),
    .fet_3_width = db.ToInternalUnits(450),
    .fet_4_width = db.ToInternalUnits(450),
    .fet_5_width = db.ToInternalUnits(450),
    .fet_0_length = db.ToInternalUnits(170),
    .fet_1_length = db.ToInternalUnits(170),
    .fet_2_length = db.ToInternalUnits(170),
    .fet_3_length = db.ToInternalUnits(170),
    .fet_4_length = db.ToInternalUnits(170),
    .fet_5_length = db.ToInternalUnits(170),
    .fet_4_5_offset_y = 0,
    .add_input_wires = true,
    .col_0_poly_overhang_top = std::nullopt,
    .col_0_poly_overhang_bottom = std::nullopt,
    .col_1_poly_overhang_top = std::nullopt,
    .col_1_poly_overhang_bottom = std::nullopt,
    .col_2_poly_overhang_top = std::nullopt,
    .col_2_poly_overhang_bottom = std::nullopt,
    .col_3_poly_overhang_top = std::nullopt,
    .col_3_poly_overhang_bottom = std::nullopt,
    .input_0 = std::nullopt,
    .input_1 = std::nullopt,
    .input_2 = std::nullopt,
    .input_3 = std::nullopt,
    .input_x_padding = db.ToInternalUnits(-200),
    .input_y_padding = 0 //db.ToInternalUnits(200)
  };

  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout(mux2_params_n));

  Rectangle mux2_bounding_box = mux2_layout->GetBoundingBox();
  int64_t mux2_width = (1 +
      mux2_bounding_box.Width() / poly_rules.min_pitch) * poly_rules.min_pitch;
  int64_t mux2_height = mux2_bounding_box.Height();
  // There is a min_separation rule between N-type diff.drawing (a virtual
  // layer) and the N-well.
  int64_t intra_spacing = 340;
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
  const auto &li_rules = db.Rules("li.drawing");
  const auto &licon_rules = db.Rules("licon.drawing");
  const auto &mcon_rules = db.Rules("mcon.drawing");
  const auto &met1_rules = db.Rules("met1.drawing");
  const auto &li_licon_rules = db.Rules("li.drawing", "licon.drawing");
  const auto &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");
  const auto &poly_ncon_rules = db.Rules("poly.drawing", "ncon.drawing");
  const auto &met1_mcon_rules = db.Rules("met1.drawing", "mcon.drawing");
  const auto &ndiff_nsdm_rules = db.Rules("ndiff.drawing", "nsdm.drawing");
  const auto &pdiff_psdm_rules = db.Rules("pdiff.drawing", "psdm.drawing");
  const auto &ndiff_nwell_rules = db.Rules("ndiff.drawing", "nwell.drawing");

  // Add diffusion qualifying layers, wells, etc.
  //
  // Left side is N.
  int64_t diff_padding = ndiff_nsdm_rules.min_enclosure;
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle({
      layout->GetPoint("lower_left.diff_ul") - Point(
          diff_padding, diff_padding),
      layout->GetPoint("upper_left.diff_ur") + Point(
          diff_padding, diff_padding)
  });

  // Right side is P.
  diff_padding = pdiff_psdm_rules.min_enclosure;
  layout->SetActiveLayerByName("psdm.drawing");
  Rectangle *pdiff = layout->AddRectangle({
      layout->GetPoint("lower_right.diff_ur") - Point(
          diff_padding, diff_padding),
      layout->GetPoint("upper_right.diff_ul") + Point(
          diff_padding, diff_padding)
  });

  // Add N-Well.
  diff_padding = ndiff_nwell_rules.min_enclosure;
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle({
      pdiff->lower_left() - Point(diff_padding, diff_padding),
      pdiff->upper_right() + Point(diff_padding, diff_padding)
  });

  // Generate vertical metal columns.
  //
  // Our mux can fit N tracks:
  //
  // +------------------------------------+
  // |            ^           ^           |
  // | < offset > | < pitch > | < pitch > |
  // |            v           v           |
  // +------------------------------------+
  //
  // The second half of the list is right-hand-side columns. They should be
  // moved one over depending on the overall width of the mux.
  int64_t width = static_cast<int64_t>(bounding_box.Width());
  int64_t column_pitch_max = std::max(
      met1_rules.min_width / 2 + met1_rules.min_separation + std::max(
          met1_rules.min_width / 2,
          mcon_rules.via_width / 2 + met1_mcon_rules.via_overhang_wide
      ),
      mcon_rules.via_width + (
          li_mcon_rules.via_overhang_wide +
          li_mcon_rules.via_overhang +
          li_rules.min_separation
      )
  );

  int64_t column_pitch_min = std::min(
      met1_rules.min_width / 2 + met1_rules.min_separation + std::max(
          met1_rules.min_width / 2,
          mcon_rules.via_width / 2 + met1_mcon_rules.via_overhang_wide
      ),
      mcon_rules.via_width + (
          li_mcon_rules.via_overhang_wide * 2 +
          li_rules.min_separation
      )
  );


  // Draw a regular set of vertical metal columns such that:
  // - there is a single vertical column in the centre, connectible to the
  // output
  // - the PMOS and NMOS sides have an equal number of extending out from the
  // centre
  // - the pitches between columns vary according to our need for their
  // positioning
  // - space is made for these columns but only some of them are drawn here

  int64_t central_column_x = width / 2;
  constexpr size_t kNumColumnsPerFlank = 6;

  // e.g. if 6 columns per flank, including the centre there are 13 columns, so
  // the index is 12.
  size_t last_column = 2 * kNumColumnsPerFlank;
  std::set<size_t> drawn_columns = {
      0,
      1,
      2,
      5,
      last_column - 5,
      last_column - 2,
      last_column - 1,
      last_column - 0
  };
  std::map<size_t, std::unique_ptr<PolyLine>> column_lines;
  std::map<size_t, int64_t> column_x;

  // The pitches of columns in order extending outward from the centre.
  std::vector<int64_t> pitches = {
      column_pitch_min,
      // Push the 2nd column out over the centre of the output muxes in each
      // quadrant (TODO(aryap): This should really be computed as a function of
      // where those points are.)
      column_pitch_max + li_rules.min_separation,
      column_pitch_max + (li_rules.min_separation - met1_rules.min_separation),
      column_pitch_max,
      column_pitch_max,
      2 * column_pitch_max
  };

  int64_t mux_top_y = static_cast<int64_t>(bounding_box.Height());
  int64_t mux_bottom_y = 0;
  {
    // Add vertical selector connections.
    int64_t extension = 0;
    layout->SetActiveLayerByName("met1.drawing");

    int64_t last_x_left = central_column_x;
    int64_t last_x_right = central_column_x;

    for (size_t i = 0; i < kNumColumnsPerFlank; ++i) {
      // Left and right k.
      size_t k_values[] = {
          kNumColumnsPerFlank - i - 1, 1 + kNumColumnsPerFlank + i};
      int64_t x_values[] = {
          last_x_left - pitches[i], last_x_right + pitches[i]};
      last_x_left = x_values[0];
      last_x_right = x_values[1];
      for (size_t j = 0; j < 2; ++j) {
        // Draw the left columns for j == 0, right columns for j ==1.
        size_t k = k_values[j];
        int64_t x = x_values[j];

        column_x[k] = x;
        Point bottom = {x, -extension};
        Point top = {x, mux_top_y + extension};
        if (drawn_columns.find(k) != drawn_columns.end()) {
          PolyLine *column_line = new PolyLine({bottom, top});
          column_lines[k].reset(column_line);
          column_line->SetWidth(met1_rules.min_width);
        }
      }
    }
  }

  // Connect poly to metal columns.
  // Along the top of the mux:
  //
  // v metal line (m)
  // m  p   p  m
  // x  p   +--x
  // |  p   |  m ... and so on.
  // +--x   x  m
  // m  p   p  m
  // m  p   p  m
  //    ^ poly (p)
  ConnectNamedPointsToColumns(
      db,
      {
       {"upper_left.column_0_centre_top_via",
         column_lines[0].get(),
         false},
        {"upper_left.column_1_centre_top_via",
         column_lines[1].get(),
         true},
        {"upper_left.column_2_centre_top_via",
         column_lines[2].get(),
         false},
        {"upper_left.column_3_centre_top_via",
         column_lines[5].get(),
         true},
        {"upper_right.column_3_centre_top_via",
         column_lines[last_column - 5].get(),
         true},
        {"upper_right.column_2_centre_top_via",
         column_lines[last_column - 2].get(),
         false},
        {"upper_right.column_1_centre_top_via",
         column_lines[last_column - 1].get(),
         true},
        {"upper_right.column_0_centre_top_via",
         column_lines[last_column - 0].get(),
         false}
      },
      poly_contact,
      true,  // point up
      layout.get());

  // Along the bottom of the mux:
  ConnectNamedPointsToColumns(
      db,
      {
       {"lower_left.column_0_centre_top_via",
         column_lines[0].get(),
         false},
        {"lower_left.column_1_centre_top_via",
         column_lines[1].get(),
         true},
        {"lower_left.column_2_centre_top_via",
         column_lines[2].get(),
         false},
        {"lower_left.column_3_centre_top_via",
         column_lines[5].get(),
         true},
        {"lower_right.column_3_centre_top_via",
         column_lines[last_column - 5].get(),
         true},
        {"lower_right.column_2_centre_top_via",
         column_lines[last_column - 2].get(),
         false},
        {"lower_right.column_1_centre_top_via",
         column_lines[last_column - 1].get(),
         true},
        {"lower_right.column_0_centre_top_via",
         column_lines[last_column - 0].get(),
         false}
      },
      poly_contact,
      false,  // point up
      layout.get());

  PolyLineInflator inflator(db);
  for (auto &entry : column_lines) {
    PolyLine *line = entry.second.get();
    std::optional<Polygon> polygon = inflator.InflatePolyLine(*line);
    if (polygon) {
      layout->AddPolygon(*polygon);
    }
  }

  //LOG(INFO) << layout->Describe();

  GenerateOutput2To1Mux(
      db,
      column_x[3],
      column_x[4],
      column_x[9],
      column_x[8],
      mux_top_y,
      mux_bottom_y,
      layout.get());

  // Translate sub-layout ports to external-facing ports:
  layout->SetActiveLayerByName("li.drawing");
  //std::vector<std::pair<std::string, std::string>> ports = {
  //  {"lower_left.input_0", "input_0"},
  //  {"lower_left.input_1", "input_1"},
  //  {"lower_left.input_2", "input_2"},
  //  {"lower_left.input_3", "input_3"},
  //  {"lower_right.input_0", "input_0"},
  //  {"lower_right.input_1", "input_1"},
  //  {"lower_right.input_2", "input_2"},
  //  {"lower_right.input_3", "input_3"},
  //  {"upper_left.input_0", "input_4"},
  //  {"upper_left.input_1", "input_5"},
  //  {"upper_left.input_2", "input_6"},
  //  {"upper_left.input_3", "input_7"},
  //  {"upper_right.input_0", "input_4"},
  //  {"upper_right.input_1", "input_5"},
  //  {"upper_right.input_2", "input_6"},
  //  {"upper_right.input_3", "input_7"}
  //};
  //for (const auto &entry : ports) {
  //  const std::string &layout_port = entry.first;
  //  const std::string &net = entry.second;
  //  Point centre = layout->GetPoint(layout_port);
  //  geometry::Layer layer = centre.layer();
  //  int64_t via_size = db.Rules(layer).via_width;
  //  layout->AddPort(geometry::Port(
  //      layout->GetPoint(layout_port), via_size, via_size, layer, net));
  //}

  return layout.release();
}


// Stack up for our purposes:
//  top    - met1
//         - mcon
//         - met0 (li)
//         - licon
//  bottom - poly | diff
bfg::Layout *Sky130Mux::GenerateMux2Layout(const Mux2Parameters &params) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  const auto &diff_rules = db.Rules(params.diff_layer_name);
  int64_t diffusion_min_distance = diff_rules.min_separation;
  const auto &dcon_rules = db.Rules(params.diff_contact_layer_name);
  const auto &licon_rules = db.Rules("licon.drawing");
  const auto &polycon_rules = db.Rules("polycon.drawing");
  const auto &mcon_rules = db.Rules("mcon.drawing");

  const auto &li_rules = db.Rules("li.drawing");
  const auto &poly_rules = db.Rules("poly.drawing");
  const auto &met1_rules = db.Rules("met1.drawing");

  const auto &li_dcon_rules = db.Rules(
      "li.drawing", params.diff_contact_layer_name);
  const auto &li_polycon_rules = db.Rules("li.drawing", "polycon.drawing");
  const auto &poly_polycon_rules = db.Rules("poly.drawing", "polycon.drawing");
  const auto &poly_dcon_rules = db.Rules(
      "poly.drawing", params.diff_contact_layer_name);
  const auto &diff_dcon_rules = db.Rules(
      params.diff_layer_name, params.diff_contact_layer_name);
  const auto &diff_polycon_rules = db.Rules(
      params.diff_layer_name, "polycon.drawing");

  int64_t min_lenth = std::min({
      params.fet_0_length,
      params.fet_1_length,
      params.fet_2_length,
      params.fet_3_length,
      params.fet_4_length,
      params.fet_5_length
  });
  LOG_IF(FATAL, min_lenth < poly_rules.min_width)
      << "Least given transistor length is smaller than poly_rules.min_width";

  int64_t via_side = dcon_rules.via_width;
  int64_t via_encap_width = via_side + 2 * li_dcon_rules.via_overhang_wide;
  int64_t via_encap_length = via_side + 2 * li_dcon_rules.via_overhang;

  int64_t height = std::max({
      std::max(params.fet_0_width, params.fet_2_width) +
          std::max(params.fet_1_width, params.fet_3_width) +
          diff_rules.min_separation,
      2 * (diff_dcon_rules.min_enclosure + dcon_rules.via_width +
          li_dcon_rules.via_overhang + li_rules.min_separation +
          li_rules.min_width) +
      li_rules.min_separation
  });

  // Determining where to put the inputs is very contextual. In some situation
  // it is enough to distribute them vertically alone the height of the
  // structure (with some padding), in other cases you might want to minimise
  // the vertical space consumed. Concern for the access positions and for the
  // routing to those positions are separate.
  constexpr int kNumInputs = 4;
  int64_t inputs_y[kNumInputs] = {0};
  int64_t input_x_padding = params.input_x_padding;
  int64_t input_y_padding = params.input_y_padding;
  int64_t input_y_span = height - 2 * input_y_padding;
  int64_t input_y_spacing = input_y_span / (kNumInputs - 1); 
  for (size_t i = 0; i < kNumInputs; ++i) {
    inputs_y[i] = (height - input_y_span) / 2 + (i * input_y_spacing);
  }

  //   std::max(params.fet_1_width, params.fet_3_width) +

  //int64_t height = std::max(params.fet_0_width, params.fet_2_width) +
  //   std::max(params.fet_1_width, params.fet_3_width) +
  //   std::max({diff_rules.min_separation,
  //             li_rules.min_separation + 2 * li_rules.min_width});

  int64_t diff_wing = via_side + poly_dcon_rules.min_separation +
      diff_dcon_rules.min_enclosure;

  // This is the distance poly extends from diffusion.
  int64_t default_poly_overhang_top =
      2 * (li_rules.min_width + li_rules.min_separation) +
      polycon_rules.via_width + li_polycon_rules.via_overhang;
  int64_t default_poly_overhang_bottom =
      li_rules.min_width + li_rules.min_separation +
      polycon_rules.via_width + li_polycon_rules.via_overhang;
  int64_t min_poly_overhang = // TODO(aryap): find this DRC rule.
      diff_polycon_rules.min_enclosure;

  layout->SetActiveLayerByName("poly.drawing");
  int64_t column_0_x = 0;
  int64_t mid_y = height / 2;
  PolyLine column_0_line = PolyLine(
      {column_0_x,
          -params.col_0_poly_overhang_bottom.value_or(
              default_poly_overhang_bottom)},
      {
          {{column_0_x, mid_y}, static_cast<uint64_t>(params.fet_0_length)},
          {{column_0_x, height +
              params.col_0_poly_overhang_top.value_or(
                  default_poly_overhang_top)},
          static_cast<uint64_t>(params.fet_1_length)}
      });
  Polygon *column_0_polygon = layout->AddPolygon(
      InflatePolyLineOrDie(db, column_0_line));
  Rectangle column_0 = column_0_polygon->GetBoundingBox();

  int64_t via_centre_to_poly_edge =
      polycon_rules.via_width / 2 + poly_polycon_rules.min_separation;

  int64_t poly_gap = std::max({
      dcon_rules.via_width + 2 * poly_dcon_rules.min_separation +
          std::max(params.fet_2_length, params.fet_3_length) / 2,
      poly_rules.min_separation,
      poly_rules.min_pitch - poly_rules.min_width
  });
  int64_t column_1_x = column_0.upper_right().x() + poly_gap;
  int64_t column_1_y_max = height + params.col_1_poly_overhang_top.value_or(
      default_poly_overhang_top);
  // We want the 2nd column to stick out above the left column by enough
  // distance to clear a 2nd li track horizontally.
  PolyLine column_1_line = PolyLine(
      {column_1_x,
       -params.col_1_poly_overhang_bottom.value_or(
           default_poly_overhang_bottom)}, {
      {{column_1_x, mid_y}, static_cast<uint64_t>(params.fet_2_length)},
      {{column_1_x, column_1_y_max},
          static_cast<uint64_t>(params.fet_3_length)}
  });
  Polygon *column_1_polygon = layout->AddPolygon(
      InflatePolyLineOrDie(db, column_1_line));
  Rectangle column_1 = column_1_polygon->GetBoundingBox();

  int64_t height_fet_4_5 = height + params.fet_4_5_offset_y;
  int64_t poly_width = poly_rules.min_width;
  int64_t column_2_x = column_1_x + std::max(
      diff_wing * 2 + diff_rules.min_separation + params.fet_4_length,
      poly_rules.min_pitch);
  Rectangle *column_2 = layout->AddRectangle(Rectangle(
      Point(column_2_x, height_fet_4_5 - params.fet_4_width - 
          params.col_2_poly_overhang_bottom.value_or(min_poly_overhang)),
      Point(column_2_x + params.fet_4_length, height_fet_4_5 +
          params.col_2_poly_overhang_top.value_or(
              default_poly_overhang_top))));

  int64_t column_3_x = column_2_x + params.fet_5_length / 2 + poly_gap;
  Rectangle *column_3 = layout->AddRectangle(Rectangle(
      Point(column_3_x, height_fet_4_5 - params.fet_5_width -
          params.col_3_poly_overhang_bottom.value_or(min_poly_overhang)),
      Point(column_3_x + params.fet_5_length, height_fet_4_5 +
          params.col_3_poly_overhang_top.value_or(
              default_poly_overhang_top))));

  // +---------+---------+   +---------+---------+
  // | pfet 1  | pfet 3  |   | pfet 4  | pfet 5  |
  // +---------+---------+   +---------+---------+
  //
  // +---------+---------+
  // | pfet 0  | pfet 2  |
  // +---------+---------+
  // FIXME(aryap): The diff pour should come last because we're constrained by
  // the wiring on the metal/li layers.
  layout->SetActiveLayerByName(params.diff_layer_name);
  // pfet 0
  int64_t column_0_1_mid_x = (
      column_0.upper_right().x() + column_1.lower_left().x()) / 2;
  Rectangle *pfet_0_diff = layout->AddRectangle(Rectangle(
      Point(column_0.lower_left().x() - diff_wing, 0),
      Point(column_0_1_mid_x, params.fet_0_width)));

  // pfet 1
  Rectangle *pfet_1_diff = layout->AddRectangle(Rectangle(
      Point(column_0.lower_left().x() - diff_wing, height - params.fet_1_width),
      Point(column_0_1_mid_x, height)));

  // pfet 2
  Rectangle *pfet_2_diff = layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, 0),
      Point(column_1.upper_right().x() + diff_wing, params.fet_2_width)));

  // pfet 3
  Rectangle *pfet_3_diff = layout->AddRectangle(Rectangle(
      Point(column_0_1_mid_x, height - params.fet_3_width),
      Point(column_1.upper_right().x() + diff_wing, height)));

  // pfet 4
  int64_t column_2_3_mid_x = (
      column_2->GetBoundingBox().upper_right().x() + column_3_x) / 2;
  Rectangle *pfet_4_diff = layout->AddRectangle(Rectangle(
      Point(column_2->lower_left().x() - diff_wing,
            height_fet_4_5 - params.fet_4_width),
      Point(column_2_3_mid_x, height_fet_4_5)));

  // pfet 5
  Rectangle *pfet_5_diff = layout->AddRectangle(Rectangle(
      Point(column_2_3_mid_x, height_fet_4_5 - params.fet_5_width),
      Point(column_3->upper_right().x() + diff_wing, height_fet_4_5)));

  // diff/met0 vias
  // Vias are named with indices according to their (column, row):
  //
  //   x (0, 1)   x (1, 1)   x (2, 1)   x (3, 1)
  //
  //   x (0, 0)   x (1, 0)
  //
  int64_t via_centre_to_diff_edge =
      dcon_rules.via_width / 2 + diff_dcon_rules.min_enclosure;
  layout->SetActiveLayerByName(params.diff_contact_layer_name);
  int64_t via_column_0_x = column_0.lower_left().x() - (
      via_side / 2 + poly_dcon_rules.min_separation);
  int64_t via_0_0_y = pfet_0_diff->upper_right().y() - via_centre_to_diff_edge;
  Rectangle *via_0_0 = layout->AddSquare(
      Point(via_column_0_x, via_0_0_y), via_side);

  int64_t via_0_1_y = pfet_1_diff->lower_left().y() + via_centre_to_diff_edge;
  Rectangle *via_0_1 = layout->AddSquare(
      Point(via_column_0_x, via_0_1_y), via_side);

  int64_t via_column_1_x = column_0_1_mid_x;
  int64_t via_1_0_y = std::min(
      pfet_0_diff->upper_right().y(), pfet_2_diff->upper_right().y()) -
      via_centre_to_diff_edge;
  Rectangle *via_1_0 = layout->AddSquare(
      Point(via_column_1_x, via_1_0_y), via_side);
  int64_t via_1_1_y = std::max(
      pfet_1_diff->lower_left().y(), pfet_3_diff->lower_left().y()) +
      via_centre_to_diff_edge;
  Rectangle *via_1_1 = layout->AddSquare(
      Point(via_column_1_x, via_1_1_y), via_side);

  int64_t via_column_2_x =
      pfet_2_diff->upper_right().x() - via_centre_to_diff_edge;
  Rectangle *via_2_0 = layout->AddSquare(
      Point(via_column_2_x,
            pfet_2_diff->lower_left().y() + via_centre_to_diff_edge), via_side);
  Rectangle *via_2_1 = layout->AddSquare(
      Point(via_column_2_x,
            pfet_3_diff->upper_right().y() - via_centre_to_diff_edge),
      via_side);

  int64_t via_column_3_x = column_2->lower_left().x() - (
      poly_polycon_rules.min_separation + dcon_rules.via_width / 2);
  Point via_3_1_bottom_option = Point(
      via_column_3_x,
      pfet_4_diff->lower_left().y() + via_centre_to_diff_edge);
  Point via_3_1_middle_option = Point(
      via_column_3_x, pfet_4_diff->centre().y());

  int64_t via_column_4_x =
      pfet_5_diff->upper_right().x() - via_centre_to_diff_edge;
  int64_t via_4_1_y =
      height_fet_4_5 - params.fet_5_width + via_centre_to_diff_edge;
  Rectangle *via_4_1 = layout->AddSquare(
      Point(via_column_4_x, via_4_1_y), via_side);

  // Wires connecting output of first stage muxes to inputs of second stage mux.

  int64_t inner_bottom_wire_line_y = 0;
  // Bottom wire:
  //                                 + p_4
  //       p_2                       |
  //         +-----------------------+ p_3  <-- inner_bottom_wire_line_y
  //   p_0   |
  //    +----+ p_1
  {
    layout->SetActiveLayerByName("li.drawing");
    Point p_0 = via_1_0->centre();
    Point p_1 = Point(via_2_0->lower_left().x() - (
        li_dcon_rules.via_overhang_wide + li_rules.min_separation +
            li_rules.min_width / 2),
        p_0.y());
    inner_bottom_wire_line_y = std::max({
        via_2_0->upper_right().y() + li_dcon_rules.via_overhang +
            li_rules.min_separation + li_rules.min_width / 2,
        
        p_0.y()  // This one is kinda dumb. Forces a straight line out.
    });
    //  std::max(
    //    pfet_0_diff->upper_right().y(), pfet_2_diff->upper_right().y()) +
    //    li_rules.min_width / 2;
    Point p_2 = Point(p_1.x(), inner_bottom_wire_line_y);
    Point p_3 = Point(via_4_1->centre().x(), p_2.y());
    Point p_4 = via_4_1->centre();
    PolyLine input_2_3_line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(via_encap_width)},
        LineSegment {p_2, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_3, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_4, static_cast<uint64_t>(via_encap_width)}
    });
    input_2_3_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_2_3_line.InsertBulge(p_4, via_encap_width, via_encap_length);
    layout->AddPolyLine(input_2_3_line);
    layout->SavePoint("li_corner_se_centre", p_2);
  }

  // Top wire:
  //   p_0  p_1
  //    +---+            p_5
  //        |        +---+
  //        |        |
  //        +--------+ p_3
  Point selected_via_3_1_centre;
  {
    int64_t via_3_1_to_bottom_wire_separation = 
        (via_3_1_bottom_option.y() -
            (dcon_rules.via_width / 2 +
             li_dcon_rules.via_overhang_wide)) -
        (inner_bottom_wire_line_y + li_rules.min_width);
    if (via_3_1_to_bottom_wire_separation < li_rules.min_separation) {
      // TODO(aryap): No facility to delete shapes?
      selected_via_3_1_centre = via_3_1_middle_option;
    } else {
      selected_via_3_1_centre = via_3_1_bottom_option;
    }

    int64_t via_padding_x = li_rules.min_separation +
        li_rules.min_width / 2 +
        li_dcon_rules.via_overhang_wide;
    int64_t via_padding_y = li_rules.min_separation +
        li_rules.min_width / 2 +
        li_dcon_rules.via_overhang;

    layout->SetActiveLayerByName("li.drawing");

    Point p_0 = via_1_1->centre();
    Point p_1 = Point(via_2_1->lower_left().x() - via_padding_x, p_0.y());
    Point p_2 = Point(p_1.x(),
        std::min(via_2_1->lower_left().y() - via_padding_y, p_1.y()));
    Point p_3 = Point(via_2_1->LowerRight().x() + via_padding_x, p_2.y());
    Point p_4 = Point(p_3.x(), selected_via_3_1_centre.y());
    Point p_5 = selected_via_3_1_centre;
    PolyLine input_0_1_line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(via_encap_width)},
        LineSegment {p_2, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_3, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_4, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_5, static_cast<uint64_t>(via_encap_width)}
    });
    input_0_1_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_0_1_line.InsertBulge(p_5, via_encap_width, via_encap_length);

    layout->AddPolyLine(input_0_1_line);
    layout->SavePoint("li_corner_ne_centre", p_5 + Point(0, via_encap_length));
  }

  Rectangle *via_3_1 = layout->AddSquare(
      selected_via_3_1_centre, via_side);
            // + dcon_rules.via_width / 2 + diff_dcon_rules.min_enclosure

  if (!params.add_input_wires) {
    return layout.release();
  }

  // The four input wires:
  //
  //  (Top left)
  //
  //  input_1   +---------------+
  //                            |
  //  input_0   +---+    +      +
  //                     |
  //                     + -------------- (inner wire)
  //
  //                     + -------------- (inner wire)
  //                     |
  //  input_2   +---+    +      +
  //                            |
  //  input_3   +---------------+
  //
  //  (Bottom left)
  //
  // Sometimes the outer arms can go straight out, but they have to avoid
  // interfering with the inner wires from above.

  Polygon *input_2_met_0;
  {
    // Input 2 metal.
    layout->SetActiveLayerByName("li.drawing"); 

    Point p_0 = via_0_0->centre();
    Point p_1 = Point(p_0.x(), inputs_y[1]);
    Point p_2 = Point(p_0.x() - met1_rules.min_separation + input_x_padding,
                      p_1.y());
    PolyLine input_2_line = PolyLine({p_0, p_1, p_2});
    input_2_line.SetWidth(via_encap_width);
    input_2_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_2_line.InsertBulge(p_2, via_encap_width, via_encap_length);
    Polygon input_2_template = InflatePolyLineOrDie(db, input_2_line);
    // This is the installed object.
    input_2_met_0 = layout->AddPolygon(input_2_template);
    if (params.input_2) {
      *params.input_2.value() = input_2_met_0;
    }

    layout->SetActiveLayerByName("li.pin");
    geometry::Rectangle *via = layout->AddSquare(p_2, via_side);
    layout->SavePoint("input_2", via->centre());
  }

  Polygon *input_3_met_0;
  int64_t input_3_line_y;
  {
    // Input 3 metal.
    layout->SetActiveLayerByName("li.drawing");
    int64_t metal_width = li_rules.min_width;
    //int64_t lower_left_y = input_2_met_0->GetBoundingBox().lower_left().y() -
    //                       li_dcon_rules.via_overhang_wide -
    //                       (metal_width / 2) - li_rules.min_separation;

    int64_t input_3_line_y = std::min(
        std::min(via_0_0->lower_left().y(), via_0_1->lower_left().y()) - (
            li_dcon_rules.via_overhang_wide +
            li_dcon_rules.via_overhang +
            li_rules.min_separation +
            dcon_rules.via_width / 2),
        via_2_0->centre().y());

    Point p_0 = Point(
        via_0_0->centre().x() - met1_rules.min_separation + input_x_padding,
        inputs_y[0]);

    Point p_3 = via_2_0->centre();

    Point p_1 = Point(p_0.x(), input_3_line_y);
    Point p_2 = Point(p_3.x(), input_3_line_y);

    PolyLine input_3_line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(metal_width)},
        LineSegment {p_2, static_cast<uint64_t>(metal_width)},
        LineSegment {p_3, static_cast<uint64_t>(via_encap_width)}
    });
    input_3_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_3_line.InsertBulge(p_3, via_encap_width, via_encap_length);

    Polygon input_3_template  = InflatePolyLineOrDie(db, input_3_line);
    input_3_met_0 = layout->AddPolygon(input_3_template);
    if (params.input_3) {
      *params.input_3.value() = input_3_met_0;
    }

    layout->SetActiveLayerByName("li.pin");
    geometry::Rectangle *via = layout->AddSquare(p_0, via_side);
    layout->SavePoint("input_3", via->centre());
  }

  {
    // Input 0 metal.
    layout->SetActiveLayerByName("li.drawing"); 

    Point p_0 = via_0_1->centre();
    Point p_1 = Point(p_0.x(), inputs_y[2]);
    Point p_2 = Point(
        p_0.x() - met1_rules.min_separation + input_x_padding,
        p_1.y());

    PolyLine input_0_line = PolyLine({p_0, p_1, p_2});
    input_0_line.SetWidth(via_encap_width);
    input_0_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_0_line.InsertBulge(p_2, via_encap_width, via_encap_length);

    Polygon *input_0_met_0 = layout->AddPolygon(
        InflatePolyLineOrDie(db, input_0_line));
    if (params.input_0) {
      *params.input_0.value() = input_0_met_0;
    }

    layout->SetActiveLayerByName("li.pin");
    geometry::Rectangle *via = layout->AddSquare(p_2, via_side);
    layout->SavePoint("input_0", via->centre());
  }

  Polygon *input_1_met_0;
  int64_t input_1_line_y;
  {
    // Test if we can head straight out.
    input_1_line_y = std::max(
        // Closest we can go to the vias + li over the diff:
        std::max(via_1_1->upper_right().y(), via_0_1->upper_right().y()) + (
            li_dcon_rules.via_overhang_wide +
            li_dcon_rules.via_overhang +
            li_rules.min_separation +
            dcon_rules.via_width / 2),
        via_2_1->centre().y());

    // Input 1 metal.
    layout->SetActiveLayerByName("li.drawing");

    Point p_0 = Point(
        via_0_0->centre().x() - met1_rules.min_separation + input_x_padding,
        inputs_y[3]);

    Point p_3 = via_2_1->centre();

    Point p_1 = Point(p_0.x(), input_1_line_y);
    Point p_2 = Point(p_3.x(), input_1_line_y);

    PolyLine input_1_line = PolyLine(p_0, {
        LineSegment {p_1, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_2, static_cast<uint64_t>(li_rules.min_width)},
        LineSegment {p_3, static_cast<uint64_t>(via_encap_width)}
    });
    input_1_line.InsertBulge(p_0, via_encap_width, via_encap_length);
    input_1_line.InsertBulge(p_3, via_encap_width, via_encap_length);

    input_1_met_0 = layout->AddPolygon(
        InflatePolyLineOrDie(db, input_1_line));
    if (params.input_1) {
      *params.input_1.value() = input_1_met_0;
    }

    layout->SetActiveLayerByName("li.pin");
    geometry::Rectangle *via = layout->AddSquare(p_0, via_side);
    layout->SavePoint("input_1", via->centre());
  }

  int64_t poly_contact_to_diff =
      diff_polycon_rules.min_separation + polycon_rules.via_width / 2;
  int64_t licon_via_to_li_end =
      polycon_rules.via_width / 2 + li_polycon_rules.min_enclosure;
  int64_t poly_contact_to_li = li_polycon_rules.via_overhang +
      li_rules.min_separation + licon_rules.via_width / 2;

  std::map<const std::string, const Point> export_points = {
      {"column_0_centre_bottom_via", Point(
          column_0.centre().x(),
          // The contact can't be too close to the end of the poly -
          // TODO(aryap): but maybe we don't want to enforce this constraint?
          std::max(
              // The poly contact can't be too close to the diffusion.
              std::min(
                  pfet_0_diff->lower_left().y() - poly_contact_to_diff,
                  input_3_met_0->GetBoundingBox().lower_left().y() -
                  poly_contact_to_li
              ),
              column_0.lower_left().y() + licon_via_to_li_end
          ))
      },
      {"column_0_centre_centre_via", Point(
          column_0.centre().x(),
          (pfet_1_diff->lower_left().y() +
              pfet_0_diff->upper_right().y()) / 2)},
      {"column_0_centre_top_via", Point(
          column_0.centre().x(),
          std::min(
              std::max(
                  pfet_1_diff->upper_right().y() + poly_contact_to_diff,
                  input_1_line_y + li_rules.min_width / 2 +
                  poly_contact_to_li
              ),
              column_0.upper_right().y() - licon_via_to_li_end
          ))
      },

      {"column_1_centre_bottom_via", Point(
          column_1.centre().x(),
          std::max(
              // The poly contact can't be too close to the diffusion.
              std::min(
                  pfet_2_diff->lower_left().y() - poly_contact_to_diff,
                  input_3_met_0->GetBoundingBox().lower_left().y() -
                  poly_contact_to_li
              ),
              column_1.lower_left().y() + licon_via_to_li_end
          ))
      },
      {"column_1_centre_centre_via", Point(
          column_1.centre().x(),
          (pfet_3_diff->lower_left().y() +
              pfet_2_diff->upper_right().y()) / 2)},
      {"column_1_centre_top_via", Point(
          column_1.centre().x(),
          std::min(
              std::max(
                  pfet_3_diff->upper_right().y() + poly_contact_to_diff,
                  input_1_line_y + li_rules.min_width / 2 +
                  poly_contact_to_li
              ),
              column_1.upper_right().y() - licon_via_to_li_end
          ))
      },

      {"column_2_centre_bottom_via", Point(
          column_2->centre().x(),
          std::max(
              pfet_4_diff->lower_left().y() - poly_contact_to_diff,
              column_2->lower_left().y() + licon_via_to_li_end
          ))
      },
      {"column_2_centre_top_via", Point(
          column_2->centre().x(),
          std::min(
              std::max(
                  pfet_4_diff->upper_right().y() + poly_contact_to_diff,
                  input_1_line_y + li_rules.min_width / 2 +
                  poly_contact_to_li
              ),
              column_2->upper_right().y() - licon_via_to_li_end
          ))
      },

      {"column_3_centre_bottom_via", Point(
          column_3->centre().x(),
          std::max(
              pfet_5_diff->lower_left().y() - poly_contact_to_diff,
              column_3->lower_left().y() + licon_via_to_li_end
          ))
      },
      {"column_3_centre_top_via", Point(
          column_3->centre().x(),
          std::min(
              std::max(
                  pfet_5_diff->upper_right().y() + poly_contact_to_diff,
                  input_1_line_y + li_rules.min_width / 2 + poly_contact_to_li
              ),
              column_3->upper_right().y() - licon_via_to_li_end
          ))
      },

      {"column_0_centre_bottom", Point(
          column_0.centre().x(), column_0.lower_left().y())},
      {"column_0_centre_top", Point(
          column_0.centre().x(), column_0.upper_right().y())},

      {"column_1_centre_bottom", Point(
          column_1.centre().x(), column_1.lower_left().y())},
      {"column_1_centre_top", Point(
          column_1.centre().x(), column_1.upper_right().y())},

      {"column_2_centre_bottom", Point(
          column_2->centre().x(), column_2->lower_left().y())},
      {"column_2_centre_top", Point(
          column_2->centre().x(), column_2->upper_right().y())},
      {"column_3_centre_bottom", Point(
          column_3->centre().x(), column_3->lower_left().y())},
      {"column_3_centre_top", Point(
          column_3->centre().x(), column_3->upper_right().y())},

      {"output", Point(
            pfet_4_diff->upper_right().x(),
            pfet_4_diff->upper_right().y() - (
                dcon_rules.via_width / 2 + diff_dcon_rules.min_enclosure))},

      {"via_3_1_ur", via_3_1->upper_right()},

      // Save some bounds for the diff regions.
      // TODO(aryap): It would be convenient to get the bounding box for all the
      // shapes on diff.drawing at this point and use that for this:
      {"diff_ll", pfet_0_diff->lower_left()},
      {"diff_ul", pfet_1_diff->UpperLeft()},
      {"diff_ur", pfet_5_diff->upper_right()},
      {"diff_lr", Point(
          pfet_5_diff->upper_right().x(),
          pfet_0_diff->lower_left().y())}
  };
  layout->SavePoints(export_points);



  return layout.release();
}

bfg::Circuit *Sky130Mux::GenerateMux2Circuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());


  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
