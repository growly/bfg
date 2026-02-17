#include "sky130_xor2.h"

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

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Layer;

bfg::Cell *Sky130Xor2::Generate() {
  // A 2-input XOR gate implements the function:
  //               _   _
  //   X = A⊕B = A·B + A·B

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_xor2": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  return cell.release();
}

bfg::Circuit *Sky130Xor2::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  circuit::Wire X = circuit->AddSignal("X");
  circuit::Wire A = circuit->AddSignal("A");
  circuit::Wire B = circuit->AddSignal("B");
  circuit::Wire VPWR = circuit->AddSignal(parameters_.power_net);
  circuit::Wire VGND = circuit->AddSignal(parameters_.ground_net);
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(X);
  circuit->AddPort(A);
  circuit->AddPort(B);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  bfg::Circuit *nfet_01v8 =
      design_db_->FindCellOrDie("sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  bfg::Circuit *pfet_01v8 =
      design_db_->FindCellOrDie(
          "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();

  // TODO(aryap): Implement XOR logic using transistors.
  // A typical XOR2 implementation would require multiple transistors
  // configured to implement X = A⊕B.
  //
  // For now, this is a placeholder structure.

  // nfet_0
  circuit::Instance *nfet_0 = circuit->AddInstance("nfet_0", nfet_01v8);
  // nfet_1
  circuit::Instance *nfet_1 = circuit->AddInstance("nfet_1", nfet_01v8);
  // nfet_2
  circuit::Instance *nfet_2 = circuit->AddInstance("nfet_2", nfet_01v8);
  // nfet_3
  circuit::Instance *nfet_3 = circuit->AddInstance("nfet_3", nfet_01v8);
  // nfet_4
  circuit::Instance *nfet_4 = circuit->AddInstance("nfet_4", nfet_01v8);

  // pfet_0
  circuit::Instance *pfet_0 = circuit->AddInstance("pfet_0", pfet_01v8);
  // pfet_1
  circuit::Instance *pfet_1 = circuit->AddInstance("pfet_1", pfet_01v8);
  // pfet_2
  circuit::Instance *pfet_2 = circuit->AddInstance("pfet_2", pfet_01v8);
  // pfet_3
  circuit::Instance *pfet_3 = circuit->AddInstance("pfet_3", pfet_01v8);
  // pfet_4
  circuit::Instance *pfet_4 = circuit->AddInstance("pfet_4", pfet_01v8);

  struct FetParameters {
    circuit::Instance *instance;
    uint64_t width_nm;
    uint64_t length_nm;
  };
  std::array<FetParameters, 10> fet_parameters = {
    FetParameters {
      nfet_0, parameters_.nfet_0_width_nm, parameters_.nfet_0_length_nm
    },
    FetParameters {
      nfet_1, parameters_.nfet_1_width_nm, parameters_.nfet_1_length_nm
    },
    FetParameters {
      nfet_2, parameters_.nfet_2_width_nm, parameters_.nfet_2_length_nm
    },
    FetParameters {
      nfet_3, parameters_.nfet_3_width_nm, parameters_.nfet_3_length_nm
    },
    FetParameters {
      nfet_4, parameters_.nfet_4_width_nm, parameters_.nfet_4_length_nm
    },
    FetParameters {
      pfet_0, parameters_.pfet_0_width_nm, parameters_.pfet_0_length_nm
    },
    FetParameters {
      pfet_1, parameters_.pfet_1_width_nm, parameters_.pfet_1_length_nm
    },
    FetParameters {
      pfet_2, parameters_.pfet_2_width_nm, parameters_.pfet_2_length_nm
    },
    FetParameters {
      pfet_3, parameters_.pfet_3_width_nm, parameters_.pfet_3_length_nm
    },
    FetParameters {
      pfet_4, parameters_.pfet_4_width_nm, parameters_.pfet_4_length_nm
    }
  };
  for (size_t i = 0; i < fet_parameters.size(); ++i) {
    circuit::Instance *fet = fet_parameters[i].instance;
    fet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(fet_parameters[i].width_nm),
            Parameter::SIUnitPrefix::NANO));
    fet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(fet_parameters[i].length_nm),
            Parameter::SIUnitPrefix::NANO));
  }

  // TODO(aryap): Wire up the transistors to implement XOR logic.

  return circuit.release();
}

bfg::Layout *Sky130Xor2::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  // TODO(arya): Might be useful to set this to a multiple of some unit value
  // (per usual std. cells).
  int64_t width =
      design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  int64_t height =
      design_db_->physical_db().ToInternalUnits(parameters_.height_nm);

  // met1.drawing 68/20
  // The second "metal" layer.
  layout->SetActiveLayerByName("met1.drawing");
  Rectangle *vgnd_bar = layout->AddRectangle(
      Rectangle(Point(0, -240), width, 480));
  vgnd_bar->set_net(parameters_.ground_net);

  Rectangle *vpwr_bar =
      layout->AddRectangle(Rectangle(Point(0, height - 240), width, 480));
  vpwr_bar->set_net(parameters_.power_net);

  // TODO(aryap): Implement full layout with diffusion, poly, contacts, etc.
  
  // poly.drawing
  layout->SetActiveLayerByName("poly.drawing");
  layout->AddPolygon(Polygon({
      {435, 105},
      {435, 995},
      {375, 995},
      {375, 1325},
      {435, 1325},
      {435, height - 105},
      {585, height - 105},
      {585, 1325},
      {645, 1325},
      {645, 995},
      {585, 995},
      {585, 105},
  }));
  layout->AddPolygon(Polygon({
      {855, 105},
      {855, height - 105},
      {1005, height - 105},
      {1005, 1325},
      {1275, 1325},
      {1275, height - 105},
      {1425, height - 105},
      {1425, 105},
      {1275, 105},
      {1275, 995},
      {1005, 995},
      {1005, 105},
  }));
  layout->AddPolygon(Polygon({
      {1695, 105},
      {1695, 995},
      {1635, 995},
      {1635, 1325},
      {1695, 1325},
      {1695, height - 105},
      {1845, height - 105},
      {1845, 1325},
      {1905, 1325},
      {1905, 995},
      {1845, 995},
      {1845, 105},
  }));
  layout->AddPolygon(Polygon({
      {2615, 105},
      {2615, 995},
      {2115, 995},
      {2115, 1325},
      {2635, 1325},
      {2635, height - 105},
      {2785, height - 105},
      {2785, 1160},
      {2765, 1160},
      {2765, 105},
  }));

  // diff.drawing
  layout->SetActiveLayerByName("diff.drawing");
  int64_t pdiff_top = height - 235;
  Rectangle *pdiff_0 = layout->AddRectangle(Rectangle(
      {175, pdiff_top - db.ToInternalUnits(parameters_.pfet_0_width_nm)},
      {(585 + 855) / 2, pdiff_top}));
  Rectangle *pdiff_1 = layout->AddRectangle(Rectangle(
      {(585 + 855) / 2, pdiff_top - db.ToInternalUnits(
          parameters_.pfet_1_width_nm)},
      {(1005 + 1254) / 2, pdiff_top}));
  Rectangle *pdiff_2 = layout->AddRectangle(Rectangle(
      {(1005 + 1254) / 2, pdiff_top - db.ToInternalUnits(
          parameters_.pfet_2_width_nm)},
      {(1425 + 1695) / 2, pdiff_top}));
  Rectangle *pdiff_3 = layout->AddRectangle(Rectangle(
      {(1425 + 1695) / 2, pdiff_top - db.ToInternalUnits(
          parameters_.pfet_3_width_nm)},
      {2105, pdiff_top}));
  Rectangle *pdiff_4 = layout->AddRectangle(Rectangle(
      {2375, pdiff_top - db.ToInternalUnits(parameters_.pfet_4_width_nm)},
      {3085, pdiff_top}));

  std::vector<Rectangle*> all_pdiffs = {
    pdiff_0, pdiff_1, pdiff_2, pdiff_3, pdiff_4};

  int64_t ndiff_bottom = 235;
  Rectangle *ndiff_0 = layout->AddRectangle(Rectangle(
      {175, ndiff_bottom},
      {(585 + 855) / 2,
          ndiff_bottom + db.ToInternalUnits(parameters_.nfet_0_width_nm)}));
  Rectangle *ndiff_1 = layout->AddRectangle(Rectangle(
      {(585 + 855) / 2, ndiff_bottom},
      {(1005 + 1254) / 2,
          ndiff_bottom + db.ToInternalUnits(parameters_.nfet_1_width_nm)}));
  Rectangle *ndiff_2 = layout->AddRectangle(Rectangle(
      {(1005 + 1254) / 2, ndiff_bottom},
      {(1425 + 1695) / 2,
          ndiff_bottom + db.ToInternalUnits(parameters_.nfet_2_width_nm)}));
  Rectangle *ndiff_3 = layout->AddRectangle(Rectangle(
      {(1425 + 1695) / 2, ndiff_bottom},
      {2105, ndiff_bottom + db.ToInternalUnits(parameters_.nfet_3_width_nm)}));
  Rectangle *ndiff_4 = layout->AddRectangle(Rectangle(
      {2105, ndiff_bottom},
      {3085, ndiff_bottom + db.ToInternalUnits(parameters_.nfet_4_width_nm)}));

  std::vector<Rectangle*> all_ndiffs = {
    ndiff_0, ndiff_1, ndiff_2, ndiff_3, ndiff_4};

  // li.drawing
  layout->SetActiveLayerByName("li.drawing");
  layout->AddPolygon(Polygon({
      {635, 335},
      {635, 655},
      {85, 655},
      {85, 2465},
      {465, 2465},
      {465, 1785},
      {255, 1785},
      {255, 825},
      {2105, 825},
      {2105, 1325},
      {2335, 1325},
      {2335, 655},
      {805, 655},
      {805, 335},
  }));
  layout->AddPolygon(Polygon({
      {425, 995},
      {425, 1615},
      {1730, 1615},
      {1730, 1245},
      {1935, 1245},
      {1935, 1075},
      {1560, 1075},
      {1560, 1445},
      {670, 1445},
      {670, 995},
  }));
  layout->AddPolygon(Polygon({
      {1395, 1785},
      {1395, 2465},
      {1725, 2465},
      {1725, 1955},
      {2235, 1955},
      {2235, 2465},
      {2635, 2465},
      {2635, 1785},
  }));
  layout->AddPolygon(Polygon({
      {1720, 315},
      {1720, 485},
      {2505, 485},
      {2505, 1535},
      {2815, 1535},
      {2815, 2465},
      {3135, 2465},
      {3135, 1365},
      {2675, 1365},
      {2675, 315},
  }));

  layout->MakeVia("ncon.drawing", {720, 500});
  layout->MakeVia("ncon.drawing", {1980, 400});
  layout->MakeVia("ncon.drawing", {2480, 400});
  layout->MakeVia("ncon.drawing", {2930, 415});
  layout->MakeVia("ncon.drawing", {2930, 755});

  layout->MakeVia("pcon.drawing", {300, 2340});
  layout->MakeVia("pcon.drawing", {300, 2000});
  layout->MakeVia("pcon.drawing", {1560, 2290});
  layout->MakeVia("pcon.drawing", {1560, 1950});
  layout->MakeVia("pcon.drawing", {2500, 2290});
  layout->MakeVia("pcon.drawing", {2500, 1950});
  layout->MakeVia("pcon.drawing", {2920, 2300});
  layout->MakeVia("pcon.drawing", {2920, 1960});
  layout->MakeVia("pcon.drawing", {2920, 1620});

  layout->MakeVia("polycon.drawing", {510, 1160});
  layout->MakeVia("polycon.drawing", {1095, 1160});
  layout->MakeVia("polycon.drawing", {1770, 1160});
  layout->MakeVia("polycon.drawing", {2250, 1160});

  // The polycon contacts need an npc.drawing perimeter around them.
  {
    // npc.drawing
    ScopedLayer sl(layout.get(), "npc.drawing");
    layout->AddRectangle(Rectangle({0, 975}, {width, 1345}));
  }

  // For ground contacts:
  layout->AddRectangle(Rectangle({135, 85}, {465, 475}));
  layout->AddRectangle(Rectangle({975, 85}, {1305, 475}));
  layout->AddRectangle(Rectangle({2845, 85}, {3135, 920}));

  layout->MakeVia("ncon.drawing", {300, 390});
  layout->MakeVia("ncon.drawing", {1140, 390});

  // For power contacts:
  layout->AddRectangle(Rectangle({1055, 1785}, {1225, 2635}));
  layout->AddRectangle(Rectangle({1895, 2125}, {2065, 2635}));

  layout->MakeVia("pcon.drawing", {1140, 2290});
  layout->MakeVia("pcon.drawing", {1140, 1950});
  layout->MakeVia("pcon.drawing", {1980, 2290});

  layout->AddRectangle(Rectangle({840, 1075}, {1390, 1275}));

  layout->AddRectangle(Rectangle({0, -85}, {width, 85}));
  layout->AddRectangle(Rectangle({0, height - 85}, {width, height + 85}));

  // li.pin
  // li.label
  layout->MakePin("B", {690, 1530}, "li.pin");
  layout->MakePin("A", {1150, 1190}, "li.pin");
  layout->MakePin("X", {2990, 1530}, "li.pin");

  // The following sections seems to be oft-repeated across cells and is largely
  // a consequence of the diff sizing and pin placements (smells like it should
  // be factored out!)

  Rectangle *nwell_pin = nullptr;
  Rectangle *pwell_pin = nullptr;
  if (parameters_.draw_overflowing_vias_and_pins) {
    static constexpr int64_t kMconViaPitch = 460;

    if (parameters_.draw_vpwr_vias) {
      // Metal to li1.drawing contacts (VPWR side).
      layout->StampVias(
          "mcon.drawing",
          {vpwr_bar->lower_left().x(), vpwr_bar->centre().y()},
          {vpwr_bar->upper_right().x(), vpwr_bar->centre().y()},
          kMconViaPitch);
    }

    // met1.pin
    layout->SetActiveLayerByName("met1.pin");
    layout->MakePin("VPWR", {230, static_cast<int64_t>(height)}, "met1.pin");
    layout->MakePin("VGND", {230, 0}, "met1.pin");

    if (parameters_.draw_vgnd_vias) {
      // Metal to li1.drawing contacts (VGND side).
      layout->StampVias(
          "mcon.drawing",
          {vgnd_bar->lower_left().x(), vgnd_bar->centre().y()},
          {vgnd_bar->upper_right().x(), vgnd_bar->centre().y()},
          kMconViaPitch);
    }

    // nwell.pin 64/16
    layout->SetActiveLayerByName("nwell.pin");
    nwell_pin = layout->AddSquare(
        {kMconViaPitch / 2, vpwr_bar->centre().y()}, 170);
    nwell_pin->set_net("VPB");

    // pwell.pin 122/16
    layout->SetActiveLayerByName("pwell.pin");
    pwell_pin = layout->AddSquare(
        {kMconViaPitch / 2, vgnd_bar->centre().y()}, 170);
    // FIXME(aryap): This still breaks proto2gds; see sky130_decap.cc for same
    // note!
    //pwell_pin->set_net("VNB");
  }

  Rectangle pdiff_cover = *Rectangle::AccumulatedOver(all_pdiffs);

  int64_t nwell_y_max = nwell_pin ? nwell_pin->upper_right().y() : height;
  {
    ScopedLayer scoped_layer(layout.get(), "nwell.drawing");
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    Rectangle nwell_rectangle = pdiff_cover.WithPadding(nwell_margin);
    // Extend the nwell to the top of the cell.
    nwell_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(nwell_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "psdm.drawing");
    int64_t psdm_margin = db.Rules(
        "psdm.drawing", "pdiff.drawing").min_enclosure;
    Rectangle psdm_rectangle = pdiff_cover.WithPadding(psdm_margin);
    psdm_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(psdm_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "hvtp.drawing");
    int64_t hvtp_margin = db.Rules(
        "hvtp.drawing", "pdiff.drawing").min_enclosure;
    Rectangle hvtp_rectangle = pdiff_cover.WithPadding(hvtp_margin);
    hvtp_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(hvtp_rectangle);
  }

  Rectangle ndiff_cover = *Rectangle::AccumulatedOver(all_ndiffs);

  int64_t psdm_y_min = pwell_pin ? pwell_pin->lower_left().y() : 0;
  {
    ScopedLayer layer(layout.get(), "nsdm.drawing");
    int64_t nsdm_margin = db.Rules(
        "nsdm.drawing", "ndiff.drawing").min_enclosure;
    Rectangle nsdm_rectangle = ndiff_cover.WithPadding(nsdm_margin);
    nsdm_rectangle.lower_left().set_y(psdm_y_min);
    layout->AddRectangle(nsdm_rectangle);
  }

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  Rectangle *tiling_bounds = layout->AddRectangle(
      Rectangle(Point(0, 0), width, height));
  layout->SetTilingBounds(*tiling_bounds);

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
