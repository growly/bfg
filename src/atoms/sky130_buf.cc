#include "sky130_buf.h"

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

bfg::Cell *Sky130Buf::Generate() {
  // A buffer is two back-to-back inverters:
  //
  //          /             /
  //      g  _| s       g  _| s
  //      +o|_ pfet_0   +o|_  pfet_1
  //      |   | d       |   | d
  // A ---+   +------P--+   +--- X
  //      |  _| d       |  _| d
  //      +-|_ nfet_0   +-|_  nfet_1
  //      g   | s       g   | s
  //          V             V
  // P = ~A
  // X = ~~A

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_buf": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  // TODO(growly): std::move?
  return cell.release();
}

bfg::Circuit *Sky130Buf::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  circuit::Wire X = circuit->AddSignal("X");
  circuit::Wire P = circuit->AddSignal("P");
  circuit::Wire A = circuit->AddSignal("A");
  circuit::Wire VPWR = circuit->AddSignal("VPWR");
  circuit::Wire VGND = circuit->AddSignal("VGND");
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(X);
  circuit->AddPort(P);
  circuit->AddPort(A);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  bfg::Circuit *nfet_01v8 =
      design_db_->FindCellOrDie("sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  bfg::Circuit *pfet_01v8 =
      design_db_->FindCellOrDie(
          "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();

  // TODO(aryap): Define circuit primitives within the Circuit schema per PDK.
  // We need models of different transistors and capacitors, resistors, etc.
  // I think this should be a part of the PDK proto message, but only
  // references to VLSIR parts. e.g. the PDK should contain a list of
  // of primitive modules by name, perhaps some overriding parameters.
  // Independently we need a collection of Modules defined for the PDK fed to
  // us. Then maybe a higher-level PDKDatabase or something tracks both
  // Physical (layout) and circuit properties, and can give us a handle to the
  // appropriate Object by name (e.g. "nmos_rvt").

  // The spice netlist in the PDK is:
  // ~/src/skywater-pdk/libraries/sky130_fd_sc_hd/latest/cells/buf/sky130_fd_sc_hd__buf_1.spice
  //
  //  .subckt sky130_fd_sc_hd__buf_1 A VGND VNB VPB VPWR X
  //  nfet_0 VGND a_27_47# X VNB sky130_fd_pr__nfet_01v8 w=520000u l=150000u
  //  pfet_0 a_27_47# A VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=790000u l=150000u
  //  nfet_1 a_27_47# A VGND VNB sky130_fd_pr__nfet_01v8 w=520000u l=150000u
  //  pfet_1 VPWR a_27_47# X VPB sky130_fd_pr__pfet_01v8_hvt w=790000u l=150000u
  //  .ends
  //
  // Model sky130_fd_pr__nfet_01v8__model has ports "d g s b":
  //  drain, gate, source, substrate bias
  // nfet_0
  circuit::Instance *nfet_0 = circuit->AddInstance("nfet_0", nfet_01v8);
  // pfet_0
  circuit::Instance *pfet_0 = circuit->AddInstance("pfet_0", pfet_01v8);
  // nfet_1
  circuit::Instance *nfet_1 = circuit->AddInstance("nfet_1", nfet_01v8);
  // pfet_1
  circuit::Instance *pfet_1 = circuit->AddInstance("pfet_1", pfet_01v8);

  struct FetParameters {
    circuit::Instance *instance;
    uint64_t width_nm;
    uint64_t length_nm;
  };
  std::array<FetParameters, 4> fet_parameters = {
    FetParameters {
      nfet_0, parameters_.nfet_0_width_nm, parameters_.nfet_0_length_nm
    },
    FetParameters {
      nfet_1, parameters_.nfet_1_width_nm, parameters_.nfet_1_length_nm
    },
    FetParameters {
      pfet_0, parameters_.pfet_0_width_nm, parameters_.pfet_0_length_nm
    },
    FetParameters {
      pfet_1, parameters_.pfet_1_width_nm, parameters_.pfet_1_length_nm
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

  pfet_0->Connect({{"d", P}, {"g", A}, {"s", VPWR}, {"b", VPB}});
  nfet_0->Connect({{"d", P}, {"g", A}, {"s", VGND}, {"b", VNB}});

  pfet_1->Connect({{"d", X}, {"g", P}, {"s", VPWR}, {"b", VPB}});
  nfet_1->Connect({{"d", X}, {"g", P}, {"s", VGND}, {"b", VNB}});

  return circuit.release();
}

bfg::Layout *Sky130Buf::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  uint64_t width =
      design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  uint64_t height =
      design_db_->physical_db().ToInternalUnits(parameters_.height_nm);

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  Rectangle *tiling_bounds = layout->AddRectangle(Rectangle(Point(0, 0), width, height));
  layout->SetTilingBounds(*tiling_bounds);

  // met1.drawing 68/20
  // The second "metal" layer.
  layout->SetActiveLayerByName("met1.drawing");
  layout->AddRectangle(Rectangle(Point(0, -240), width, 480));

  layout->AddRectangle(Rectangle(Point(0, height - 240), width, 480));

  // li.drawing 67/20
  // The first "metal" layer.
  layout->SetActiveLayerByName("li.drawing");
  layout->AddPolygon(Polygon({Point(0, -85),
                              Point(0, 85),
                              Point(525, 85),
                              Point(525, 465),
                              Point(855, 465),
                              Point(855, 85),
                              Point(width, 85),
                              Point(width, -85) }));

  layout->AddPolygon(Polygon({Point(175, 255),     // metal width 170 or 180
                              Point(175, 805),     // 255 is 170 to nearest wire
                              Point(670, 805),
                              Point(670, 1535),
                              Point(165, 1535),
                              Point(165, 2465),
                              Point(345, 2465),
                              Point(345, 1705),
                              Point(840, 1705),
                              Point(840, 1390),
                              Point(945, 1390),
                              Point(945, 1060),
                              Point(840, 1060),
                              Point(840, 635),
                              Point(345, 635),
                              Point(345, 255)}));

  layout->AddRectangle(Rectangle(Point(105, 985), Point(445, 1355)));

  // Again the distance to neighbouring metal seems tobe 170 or 180.
  layout->AddPolygon(Polygon({Point(1035, 255),
                              Point(1035, 760),
                              Point(1115, 760),
                              Point(1115, 1560),
                              Point(1025, 1560),
                              Point(1025, 2465),
                              Point(1295, 2465),
                              Point(1295, 255)}));

  layout->AddPolygon(Polygon({Point(525, 1875),
                              Point(525, 2635),
                              Point(0, 2635),
                              Point(0, 2805),
                              Point(1380, 2805),
                              Point(1380, 2635),
                              Point(855, 2635),
                              Point(855, 1875)}));

  // mcon.drawing 67/44
  // Metal to li1.drawing contacts (VPWR side).
  layout->SetActiveLayerByName("mcon.drawing");
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));
  layout->AddRectangle(Rectangle(Point(605, 2635), Point(775, 2805)));
  layout->AddRectangle(Rectangle(Point(1065, 2635), Point(1235, 2805)));

  // Metal to li1.drawing contacts (VGND side).
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));
  layout->AddRectangle(Rectangle(Point(605, -85), Point(775, 85)));
  layout->AddRectangle(Rectangle(Point(1065, -85), Point(1235, 85)));

  // licon.drawing 66/44
  // Contacts from li layer to diffusion.
  layout->SetActiveLayerByName("licon.drawing");
  // Input and output.
  layout->AddRectangle(Rectangle(Point(185, 1075), Point(355, 1245)));
  layout->AddRectangle(Rectangle(Point(775, 1140), Point(945, 1310)));

  // TODO(aryap): These are a function of transistor width.
  layout->AddSquare(Point(260, 2300), 170);
  layout->AddSquare(Point(260, 1960), 170);

  layout->AddSquare(Point(690, 2300), 170);
  layout->AddSquare(Point(690, 1960), 170);

  layout->AddSquare(Point(1120, 2300), 170);
  layout->AddSquare(Point(1120, 1895), 170);

  // TODO(aryap): So are these!
  layout->AddSquare(Point(260, 445), 170);
  layout->AddSquare(Point(690, 380), 170);
  layout->AddSquare(Point(1120, 530), 170);

  // npc.drawing 95/20
  // "The SKY130 process requires an 'NPC' layer to enclose all poly contacts."
  //  - tok on https://codeberg.org/tok/librecell/issues/11
  // There are "minium size, spacing and enclosure" rules.
  layout->SetActiveLayerByName("npc.drawing");
  layout->AddRectangle(Rectangle(Point(0, 975), Point(1380, 1410)));

  // hvtp.drawing 78/44
  layout->SetActiveLayerByName("hvtp.drawing");
  layout->AddRectangle(Rectangle(Point(0, 1250), Point(1380, 2720)));

  // poly.drawing 66/20
  // Polysilicon, more generally gate material.
  layout->SetActiveLayerByName("poly.drawing");
  layout->AddPolygon(Polygon({Point(395, 105),
                              Point(395, 830),
                              Point(365, 830),
                              Point(365, 995),
                              Point(135, 995),
                              Point(135, 1325),
                              Point(365, 1325),
                              Point(365, 1620),
                              Point(395, 1620),
                              Point(395, 2615),
                              Point(545, 2615),
                              Point(545, 1500),
                              Point(515, 1500),
                              Point(515, 950),
                              Point(545, 950),
                              Point(545, 105)}));

  layout->AddPolygon(Polygon({Point(835, 105),
                              Point(835, 1060),
                              Point(725, 1060),
                              Point(725, 1390),
                              Point(835, 1390),
                              Point(835, 2615),
                              Point(985, 2615),
                              Point(985, 1390),
                              Point(995, 1390),
                              Point(995, 1060),
                              Point(985, 1060),
                              Point(985, 105)}));

  // nsdm.drawing 93/44
  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddRectangle({{0, 1420}, {1380, 2910}});

  // psdm.drawing 94/20
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle({{0, -190}, {1380, 1015}});

  // diff.drawing 65/20
  // Diffusion. Intersection with gate material layer defines gate size.
  // nsdm/psdm define N/P-type diffusion.
  layout->SetActiveLayerByName("diff.drawing");
  // nfet_0
  uint64_t x0_width =
      design_db_->physical_db().ToInternalUnits(parameters_.nfet_0_width_nm);
  layout->AddRectangle(Rectangle(Point(135, 235),
                                 Point(135 + 410 + 145, 235 + x0_width)));
  // nfet_1
  uint64_t x2_width =
      design_db_->physical_db().ToInternalUnits(parameters_.nfet_1_width_nm);
  layout->AddRectangle(Rectangle(Point(135 + 410 + 145, 235),
                                 Point(1245, 235 + x2_width)));
  // pfet_0
  uint64_t x1_width =
      design_db_->physical_db().ToInternalUnits(parameters_.pfet_0_width_nm);
  layout->AddRectangle(Rectangle(Point(135, 1695),
                                 Point(135 + 410 + 145, 1695 + x1_width)));
  // pfet_1
  uint64_t x3_width =
      design_db_->physical_db().ToInternalUnits(parameters_.pfet_1_width_nm);
  layout->AddRectangle(Rectangle(Point(135 + 410 + 145, 1695),
                                 Point(1245, 1695 + x3_width)));

  // nwell.pin 64/16
  layout->SetActiveLayerByName("nwell.pin");
  layout->AddRectangle({{145, 2635}, {315, 2805}});

  // nwell.drawing 64/20
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle({{-190, 1305}, {1570 , 2910}});

  // pwell.pin 122/16
  layout->SetActiveLayerByName("pwell.pin");
  layout->AddRectangle({{155, -85}, {325 , 85}});

  // li.pin
  layout->SetActiveLayerByName("li.pin");
  layout->AddRectangleAsPort(
      Rectangle(Point(145, 1105), Point(315, 1275)), "A");

  layout->AddRectangleAsPort(
      Rectangle(Point(735, 1140), Point(905, 1310)), "P");

  layout->AddRectangleAsPort(
      Rectangle(Point(1055, 425), Point(1225, 595)), "X");
  layout->AddRectangleAsPort(
      Rectangle(Point(1055, 1785), Point(1225, 1955)), "X");
  layout->AddRectangleAsPort(
      Rectangle(Point(1055, 2125), Point(1225, 2295)), "X");

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
