#include "sky130_decap.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../circuit/wire.h"
#include "../cell.h"
#include "../geometry/layer.h"
#include "../geometry/point.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../layout.h"
#include "../scoped_layer.h"

#include "proto/parameters/sky130_decap.pb.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Layer;

void Sky130Decap::Parameters::ToProto(
    proto::parameters::Sky130Decap *pb) const {
  pb->set_width_nm(width_nm);
  pb->set_height_nm(height_nm);

  if (nfet_0_width_nm) {
    pb->set_nfet_0_width_nm(*nfet_0_width_nm);
  } else {
    pb->clear_nfet_0_width_nm();
  }

  if (nfet_0_length_nm) {
    pb->set_nfet_0_length_nm(*nfet_0_length_nm);
  } else {
    pb->clear_nfet_0_length_nm();
  }

  if (pfet_0_width_nm) {
    pb->set_pfet_0_width_nm(*pfet_0_width_nm);
  } else {
    pb->clear_pfet_0_width_nm();
  }

  if (pfet_0_length_nm) {
    pb->set_pfet_0_length_nm(*pfet_0_length_nm);
  } else {
    pb->clear_pfet_0_length_nm();
  }

  pb->set_label_pins(label_pins);
  pb->set_draw_overflowing_vias_and_pins(draw_overflowing_vias_and_pins);
}

void Sky130Decap::Parameters::FromProto(
    const proto::parameters::Sky130Decap &pb) {
  if (pb.has_width_nm()) {
    width_nm = pb.width_nm();
  }

  if (pb.has_height_nm()) {
    height_nm = pb.height_nm();
  }

  if (pb.has_nfet_0_width_nm()) {
    nfet_0_width_nm = pb.nfet_0_width_nm();
  } else {
    nfet_0_width_nm.reset();
  }

  if (pb.has_nfet_0_length_nm()) {
    nfet_0_length_nm = pb.nfet_0_length_nm();
  } else {
    nfet_0_length_nm.reset();
  }

  if (pb.has_pfet_0_width_nm()) {
    pfet_0_width_nm = pb.pfet_0_width_nm();
  } else {
    pfet_0_width_nm.reset();
  }

  if (pb.has_pfet_0_length_nm()) {
    pfet_0_length_nm = pb.pfet_0_length_nm();
  } else {
    pfet_0_length_nm.reset();
  }

  if (pb.has_label_pins()) {
     label_pins = pb.label_pins();
  }

  if (pb.has_draw_overflowing_vias_and_pins()) {
    draw_overflowing_vias_and_pins = pb.draw_overflowing_vias_and_pins();
  }
}

bfg::Cell *Sky130Decap::Generate() {
  // A decap (decoupling capacitor) cell uses the P/NMOS transistor gate
  // capacitances as capacitors:
  //
  //     /     /  VPWR/VDD
  //     |_   _|
  //   d  |___|  s
  //       ___    pfet_0
  //        o
  //        |
  //        V     VGND/VSS
  //
  //        /     VPWR/VDD
  //       _|_
  //       ___    nfet_0
  //   d _|   |_ s
  //     |     |
  //     V     V  VGND/VSS

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_buf": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *Sky130Decap::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

bfg::Layout *Sky130Decap::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  int64_t width = db.ToInternalUnits(parameters_.width_nm);
  int64_t height = db.ToInternalUnits(parameters_.height_nm);

  // areaid.standardc 81/4
  {
    ScopedLayer scoped_layer(layout.get(), "areaid.standardc");
    // Boundary for tiling; when abutting to others, this cannot be overlapped.
    Rectangle tiling_bounds({0, 0}, {width, height});
    layout->AddRectangle(tiling_bounds);
    layout->SetTilingBounds(tiling_bounds);
  }

  // met1.drawing 68/20
  Rectangle *vpwr_rectangle;
  Rectangle *vgnd_rectangle;
  {
    // The second "metal" layer.
    ScopedLayer scoped_layer(layout.get(), "met1.drawing");
    vgnd_rectangle =
        layout->AddRectangle({{0, -240}, {width, 240}});
    vgnd_rectangle->set_net(parameters_.ground_net);
    // vgnd_rectangle->set_is_connectable(true);
    vpwr_rectangle =
        layout->AddRectangle({{0, height - 240}, {width, height + 240}});
    vpwr_rectangle->set_net(parameters_.power_net);
    // vpwr_rectangle->set_is_connectable(true);
  }

  int64_t ncon_width = std::max(
      db.Rules("ncon.drawing").via_width,
      db.Rules("ncon.drawing").via_height);

  int64_t poly_separation = db.Rules("poly.drawing").min_separation;
  int64_t poly_to_edge = poly_separation / 2;

  const auto &ncon_ndiff_rules = db.Rules("ncon.drawing", "ndiff.drawing");
  int64_t diff_to_poly = std::max(
      db.Rules("poly.drawing", "ndiff.drawing").min_extension,
      ncon_ndiff_rules.min_enclosure + ncon_ndiff_rules.min_enclosure_alt +
          ncon_width);

  int64_t poly_to_diff = db.Rules(
      "poly.drawing", "ndiff.drawing").min_enclosure;
  int64_t diff_to_edge = db.Rules("diff.drawing").min_separation / 2;

  Rectangle *ndiff;
  {
    ScopedLayer scoped_layer(layout.get(), "ndiff.drawing");
    // nfet_0.
    int64_t diff_y_low = poly_to_edge + poly_to_diff;
    ndiff = layout->AddRectangle({
        {diff_to_edge, diff_y_low},
        {width - diff_to_edge, diff_y_low + NDiffHeight()}});
  }

  Rectangle *pdiff;
  {
    ScopedLayer scoped_layer(layout.get(), "pdiff.drawing");
    // nfet_0.
    int64_t diff_y_high = height - (poly_to_edge + poly_to_diff);
    pdiff = layout->AddRectangle({
        {diff_to_edge, diff_y_high - PDiffHeight()},
        {width - diff_to_edge, diff_y_high}});
  }

  int64_t li_separation = db.Rules("li.drawing").min_separation;
  int64_t li_to_edge = li_separation / 2;
  // NOTE(aryap): Even though Skywater 130 uses "licon" to connect to both poly
  // and diff, we separate them in the rules and give "licon" the name
  // "polycon" when it is used to connect to poly. We do the same thing for
  // "ncon" and "dcon" in places where "licon" connects to "ndiff" and "pdiff"
  // respectively.
  //
  // NOTE(aryap): This subsequently assumes pcon and ncon are identical.
  int64_t poly_polycon_overhang = std::max(
      db.Rules("poly.drawing", "polycon.drawing").via_overhang,
      db.Rules("poly.drawing", "polycon.drawing").via_overhang_wide);
  int64_t li_rail_width = std::max(
      db.Rules("li.drawing").min_width +
          2 * db.Rules("li.drawing", "ncon.drawing").via_overhang_wide,
      ncon_width);

  int64_t upper_poly_under_diff_x_min = pdiff->lower_left().x() + diff_to_poly;
  int64_t upper_poly_under_diff_y_min = pdiff->lower_left().y() - poly_to_diff;
  int64_t upper_poly_under_diff_x_max = pdiff->upper_right().x() - diff_to_poly;
  int64_t upper_poly_under_diff_y_max = pdiff->upper_right().y() + poly_to_diff;

  int64_t lower_poly_under_diff_x_min = ndiff->lower_left().x() + diff_to_poly;
  int64_t lower_poly_under_diff_y_min = ndiff->lower_left().y() - poly_to_diff;
  int64_t lower_poly_under_diff_x_max = ndiff->upper_right().x() - diff_to_poly;
  int64_t lower_poly_under_diff_y_max = ndiff->upper_right().y() + poly_to_diff;

  int64_t upper_poly_tab_y_min = lower_poly_under_diff_y_max + poly_separation;
  int64_t lower_poly_tab_y_max = upper_poly_under_diff_y_min - poly_separation;

  int64_t poly_tab_width = (width - 2 * poly_separation) / 2;

  int64_t upper_poly_tab_x_min = poly_to_edge;
  int64_t upper_poly_tab_x_max = upper_poly_tab_x_min + poly_tab_width;

  int64_t lower_poly_tab_x_max = width - poly_to_edge;
  int64_t lower_poly_tab_x_min = lower_poly_tab_x_max - poly_tab_width;

  {
    ScopedLayer scoped_layer(layout.get(), "poly.drawing");

    layout->AddPolygon({{
        {upper_poly_under_diff_x_min, upper_poly_under_diff_y_max},  // UL
        {upper_poly_under_diff_x_max, upper_poly_under_diff_y_max},  // UR
        {upper_poly_under_diff_x_max, upper_poly_under_diff_y_min},  // LR
        {upper_poly_tab_x_max, upper_poly_under_diff_y_min},
        {upper_poly_tab_x_max, upper_poly_tab_y_min},
        {upper_poly_tab_x_min, upper_poly_tab_y_min},
        {upper_poly_tab_x_min, upper_poly_under_diff_y_min},
        {upper_poly_under_diff_x_min, upper_poly_under_diff_y_min},
    }});

    layout->AddPolygon({{
        {lower_poly_under_diff_x_min, lower_poly_under_diff_y_min},
        {lower_poly_under_diff_x_min, lower_poly_under_diff_y_max},
        {lower_poly_tab_x_min, lower_poly_under_diff_y_max},
        {lower_poly_tab_x_min, lower_poly_tab_y_max},
        {lower_poly_tab_x_max, lower_poly_tab_y_max},
        {lower_poly_tab_x_max, lower_poly_under_diff_y_max},
        {lower_poly_under_diff_x_max, lower_poly_under_diff_y_max},
        {lower_poly_under_diff_x_max, lower_poly_under_diff_y_min}
    }});
  }

  // Add vias to connect poly and li layers.
  layout->MakeVia("polycon.drawing",
      {upper_poly_tab_x_min + ncon_width / 2 + poly_polycon_overhang,
       upper_poly_tab_y_min + ncon_width / 2 + poly_polycon_overhang});
  layout->MakeVia("polycon.drawing",
      {lower_poly_tab_x_max - ncon_width / 2 - poly_polycon_overhang,
       lower_poly_tab_y_max - ncon_width / 2 - poly_polycon_overhang});

  int64_t bottom_li_rail_y_high = li_rail_width / 2;
  int64_t top_li_rail_y_low = height - li_rail_width / 2;
  int64_t bottom_li_pour_y_high = std::max(
      ndiff->upper_right().y() + poly_to_diff + poly_polycon_overhang +
          poly_separation + ncon_width,
      upper_poly_under_diff_y_min - poly_polycon_overhang);

  int64_t top_li_pour_y_low = std::min(
      pdiff->lower_left().y() - poly_to_diff - poly_polycon_overhang -
          poly_separation - ncon_width,
      lower_poly_under_diff_y_max + poly_polycon_overhang);

  int64_t li_notch_width = (width - 2 * li_separation) / 2;

  {
    ScopedLayer scoped_layer(layout.get(), "li.drawing");
    // Wire under the VSS/VGND rail.
    layout->AddRectangle({
        {0, bottom_li_rail_y_high - li_rail_width},
        {width, bottom_li_rail_y_high}});
    // Wire under the VDD/VPWR rail.
    layout->AddRectangle({
        {0, top_li_rail_y_low},
        {width, top_li_rail_y_low + li_rail_width}});

    // Bottom li pour.
    // Dear reader! This is a Polygon being initialised with a std::vector,
    // hence the double '{'.
    layout->AddPolygon({{
        {li_to_edge,                  bottom_li_rail_y_high},
        {li_to_edge,                  bottom_li_pour_y_high},
        {li_to_edge + li_notch_width, bottom_li_pour_y_high},
        {li_to_edge + li_notch_width, top_li_pour_y_low - li_separation},
        {width - li_to_edge,          top_li_pour_y_low - li_separation},
        {width - li_to_edge,          bottom_li_rail_y_high}
    }});

    // Top li pour.
    layout->AddPolygon({{
        {li_to_edge,                          top_li_rail_y_low},
        {li_to_edge,
            bottom_li_pour_y_high + li_separation},
        {width - li_to_edge - li_notch_width,
            bottom_li_pour_y_high + li_separation},
        {width - li_to_edge - li_notch_width, top_li_pour_y_low},
        {width - li_to_edge,                  top_li_pour_y_low},
        {width - li_to_edge,                  top_li_rail_y_low}
    }});
  }

  int64_t ncon_centre_to_diff_edge_x =
      std::min(ncon_ndiff_rules.min_enclosure,
               ncon_ndiff_rules.min_enclosure_alt) +
      ncon_width / 2;

  // FIXME(aryap): I think these are too close to the polys? But my magic is
  // broken :@
  layout->DistributeVias(
      "pcon.drawing",
      {pdiff->lower_left().x() + ncon_centre_to_diff_edge_x,
          pdiff->lower_left().y()},
      {pdiff->lower_left().x() + ncon_centre_to_diff_edge_x,
          pdiff->upper_right().y()});
  layout->DistributeVias(
      "pcon.drawing",
      {pdiff->upper_right().x() - ncon_centre_to_diff_edge_x,
          pdiff->lower_left().y()},
      {pdiff->upper_right().x() - ncon_centre_to_diff_edge_x,
          pdiff->upper_right().y()});

  layout->DistributeVias(
      "ncon.drawing",
      {ndiff->lower_left().x() + ncon_centre_to_diff_edge_x,
          ndiff->lower_left().y()},
      {ndiff->lower_left().x() + ncon_centre_to_diff_edge_x,
          ndiff->upper_right().y()});
  layout->DistributeVias(
      "pcon.drawing",
      {ndiff->upper_right().x() - ncon_centre_to_diff_edge_x,
          ndiff->lower_left().y()},
      {ndiff->upper_right().x() - ncon_centre_to_diff_edge_x,
          ndiff->upper_right().y()});

  Rectangle *nwell_pin = nullptr;
  Rectangle *pwell_pin = nullptr;
  if (parameters_.draw_overflowing_vias_and_pins) {
    if (parameters_.draw_vpwr_vias) {
      layout->StampVias(
          "mcon.drawing",
          {vpwr_rectangle->lower_left().x(), vpwr_rectangle->centre().y()},
          {vpwr_rectangle->upper_right().x(), vpwr_rectangle->centre().y()},
          parameters_.mcon_via_pitch);
    }
    if (parameters_.draw_vgnd_vias) {
      layout->StampVias(
          "mcon.drawing",
          {vgnd_rectangle->lower_left().x(), vgnd_rectangle->centre().y()},
          {vgnd_rectangle->upper_right().x(), vgnd_rectangle->centre().y()},
          parameters_.mcon_via_pitch);
    }

    int64_t mcon_side = std::max(db.Rules("mcon.drawing").via_width,
                                 db.Rules("mcon.drawing").via_height);
    int64_t pin_x = parameters_.mcon_via_pitch / 2;

    // met1.pin 68/16
    layout->SetActiveLayerByName("met1.pin");
    // Apply VPWR or VGND label with pin:
    layout->MakePin(
        parameters_.power_net, {230, static_cast<int64_t>(height)}, "met1.pin");
    layout->MakePin(
        parameters_.ground_net, {230, 0}, "met1.pin");

    // nwell.pin 64/16
    layout->SetActiveLayerByName("nwell.pin");
    nwell_pin =
        layout->AddSquare({pin_x, vpwr_rectangle->centre().y()}, mcon_side);
    nwell_pin->set_net("VPB");

    // pwell.pin 122/16
    layout->SetActiveLayerByName("pwell.pin");
    Rectangle *pwell_pin =
        layout->AddSquare({pin_x, vgnd_rectangle->centre().y()}, mcon_side);
    // FIXME(aryap): This breaks proto2gds?
    //pwell_pin->set_net("VNB");
  }

  int64_t nwell_y_max = nwell_pin ? nwell_pin->upper_right().y() : height;
  {
    ScopedLayer scoped_layer(layout.get(), "nwell.drawing");
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    Rectangle nwell_rectangle = pdiff->WithPadding(nwell_margin);
    // Extend the nwell to the top of the cell.
    nwell_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(nwell_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "psdm.drawing");
    int64_t psdm_margin = db.Rules(
        "psdm.drawing", "pdiff.drawing").min_enclosure;
    Rectangle psdm_rectangle = pdiff->WithPadding(psdm_margin);
    psdm_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(psdm_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "hvtp.drawing");
    int64_t hvtp_margin = db.Rules(
        "hvtp.drawing", "pdiff.drawing").min_enclosure;
    Rectangle hvtp_rectangle = pdiff->WithPadding(hvtp_margin);
    hvtp_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(hvtp_rectangle);
  }

  int64_t psdm_y_min = pwell_pin ? pwell_pin->lower_left().y() : 0;
  {
    ScopedLayer layer(layout.get(), "nsdm.drawing");
    int64_t nsdm_margin = db.Rules(
        "nsdm.drawing", "ndiff.drawing").min_enclosure;
    Rectangle nsdm_rectangle = ndiff->WithPadding(nsdm_margin);
    nsdm_rectangle.lower_left().set_y(psdm_y_min);
    layout->AddRectangle(nsdm_rectangle);
  }

  return layout.release();
}

int64_t Sky130Decap::NDiffHeight() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  return db.ToInternalUnits(parameters_.nfet_0_width_nm.value_or(550));
}

int64_t Sky130Decap::PDiffHeight() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  return db.ToInternalUnits(parameters_.pfet_0_width_nm.value_or(870));
}

}  // namespace atoms
}  // namespace bfg
