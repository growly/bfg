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
  const IntraLayerConstraints &poly_rules = db.Rules("poly.drawing");
  const InterLayerConstraints &diff_nsdm_rules = db.Rules(
      "diff.drawing", "nsdm.drawing");
  const InterLayerConstraints &diff_psdm_rules = db.Rules(
      "diff.drawing", "psdm.drawing");
  const InterLayerConstraints &psdm_nwell_rules = db.Rules(
      "psdm.drawing", "nwell.drawing");
  const InterLayerConstraints &nwell_ndiff_rules = db.Rules(
      "nwell.drawing", "ndiff.drawing");
  const InterLayerConstraints &pdiff_nwell_rules = db.Rules(
      "pdiff.drawing", "nwell.drawing");


  Mux2Parameters mux2_params_n = {
    .diff_layer_name = "ndiff.drawing",
    .diff_contact_layer_name = "ncon.drawing",
    .fet_0_width = 640,
    .fet_1_width = 640,
    .fet_2_width = 640,
    .fet_3_width = 640,
    .fet_4_width = 640,
    .fet_5_width = 640,
    .fet_0_length = 280,
    .fet_1_length = 280,
    .fet_2_length = 280,
    .fet_3_length = 280,
    .fet_4_length = 280,
    .fet_5_length = 280
  };
  std::unique_ptr<bfg::Layout> mux2_layout(GenerateMux2Layout(mux2_params_n));

  Rectangle mux2_bounding_box = mux2_layout->GetBoundingBox();
  int64_t mux2_width = (1 +
      mux2_bounding_box.Width() / poly_rules.min_pitch) * poly_rules.min_pitch;

  int64_t intra_spacing = psdm_nwell_rules.min_separation +
      diff_psdm_rules.min_enclosure +
      std::max(diff_nsdm_rules.min_enclosure,
               nwell_ndiff_rules.min_separation);

  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));


  mux2_layout->ResetOrigin();
  layout->AddLayout(*mux2_layout, "left");

  Mux2Parameters mux2_params_p = {
    .diff_layer_name = "pdiff.drawing",
    .diff_contact_layer_name = "pcon.drawing",
    .fet_0_width = 640,
    .fet_1_width = 640,
    .fet_2_width = 640,
    .fet_3_width = 640,
    .fet_4_width = 640,
    .fet_5_width = 640,
    .fet_0_length = 280,
    .fet_1_length = 280,
    .fet_2_length = 280,
    .fet_3_length = 280,
    .fet_4_length = 280,
    .fet_5_length = 280
  };
  mux2_layout.reset(GenerateMux2Layout(mux2_params_p));

  mux2_layout->FlipHorizontal();
  mux2_layout->MoveLowerLeftTo(Point(mux2_width + intra_spacing, 0));
  layout->AddLayout(*mux2_layout, "right");

  // Add diffusion qualifying layers, wells, etc.
  //
  // Left side is N.

  int64_t diff_padding = diff_nsdm_rules.min_enclosure;
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle({
      layout->GetPoint("left.diff_ll") - Point(
          diff_padding, diff_padding),
      layout->GetPoint("left.diff_ur") + Point(
          diff_padding, diff_padding)
  });

  // Right side is P, but note that that layout has been horizontally flipped
  // so the coordinates we use are also flipped.
  diff_padding = diff_psdm_rules.min_enclosure;
  layout->SetActiveLayerByName("psdm.drawing");
  Rectangle *psdm = layout->AddRectangle({
      layout->GetPoint("right.diff_lr") - Point(
          diff_padding, diff_padding),
      layout->GetPoint("right.diff_ul") + Point(
          diff_padding, diff_padding)
  });

  // Add N-Well.
  diff_padding = std::max(
      psdm_nwell_rules.min_enclosure + diff_psdm_rules.min_enclosure,
      pdiff_nwell_rules.min_enclosure);
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle({
      layout->GetPoint("right.diff_lr") - Point(
          diff_padding, diff_padding),
      layout->GetPoint("right.diff_ul") + Point(
          diff_padding, diff_padding)
  });

  return layout.release();
}

}   // namespace bfg
}   // namespace atoms
