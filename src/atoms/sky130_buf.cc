#include "sky130_buf.h"

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

bfg::Cell *Sky130Buf::Generate() {
  // A buffer is two back-to-back inverters:
  //
  //          /         /
  //         _|        _|
  //      +o|_ X1   +o|_  X3
  //      |   |     |   |
  // A ---+   +--P--+   +--- X
  //      |  _|     |  _|
  //      +-|_ X2   +-|_  X0
  //          |         |
  //          V         V

  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_buf"));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  // TODO(growly): std::move?
  return cell.release();
}

bfg::Circuit *Sky130Buf::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  //  X   ,
  //  A   ,
  //  VPWR,
  //  VGND,
  //  VPB ,
  //  VNB

  circuit::Wire X = circuit->AddSignal("X");
  circuit::Wire P = circuit->AddSignal("P");
  circuit::Wire A = circuit->AddSignal("A");
  circuit::Wire VPWR = circuit->AddSignal("VPWR");
  circuit::Wire VGND = circuit->AddSignal("VGND");
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  // We need handles to the Sky130 P/N fets.

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
  //  X0 VGND a_27_47# X VNB sky130_fd_pr__nfet_01v8 w=520000u l=150000u
  //  X1 a_27_47# A VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=790000u l=150000u
  //  X2 a_27_47# A VGND VNB sky130_fd_pr__nfet_01v8 w=520000u l=150000u
  //  X3 VPWR a_27_47# X VPB sky130_fd_pr__pfet_01v8_hvt w=790000u l=150000u
  //  .ends
  //
  // Model sky130_fd_pr__nfet_01v8__model has ports "d g s b":
  //  drain, gate, source, substrate bias
  circuit::Instance *X0 = circuit->AddInstance("X0", nullptr);
  circuit::Instance *X1 = circuit->AddInstance("X1", nullptr);
  circuit::Instance *X2 = circuit->AddInstance("X2", nullptr);
  circuit::Instance *X3 = circuit->AddInstance("X3", nullptr);

  X0->Connect("d", VGND);
  X0->Connect("g", P);
  X0->Connect("s", X);
  X0->Connect("b", VNB);
  //X0->SetParameter("l", Parameter {

  X3->Connect({{"d", X}, {"g", P}, {"s", VPWR}, {"b", VPB}});
  X2->Connect({{"d", P}, {"g", A}, {"s", VGND}, {"b", VNB}});
  X1->Connect({{"d", P}, {"g", A}, {"s", VPWR}, {"b", VPB}});

  return circuit.release();
}

bfg::Layout *Sky130Buf::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(design_db_->physical_db()));

  uint64_t width =
      internal_units_per_nm_ * static_cast<double>(parameters_.width_nm);
  uint64_t height =
      internal_units_per_nm_ * static_cast<double>(parameters_.height_nm);

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  layout->AddRectangle(Rectangle(Point(0, 0), width, height));

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

  // licon.drawing 66/44
  // Contacts from li layer to diffusion.
  layout->SetActiveLayerByName("licon.drawing");
  layout->AddRectangle(Rectangle(Point(0, 1380), Point(975, 1410)));
                 

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
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle({{0, 1420}, {1380, 2910}});

  // psdm.drawing 94/20
  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddRectangle({{0, -190}, {1380, 1015}});

  // diff.drawing 65/20
  // Diffusion. Intersection with gate material layer defines gate size.
  // nsdm/psdm define N/P-type diffusion.
  layout->SetActiveLayerByName("diff.drawing");
  // X0
  uint64_t x0_width =
      internal_units_per_nm_ * static_cast<double>(parameters_.x0_width_nm);
  layout->AddRectangle(Rectangle(Point(135, 235),
                                 Point(135 + 410 + 145, 235 + x0_width)));
  // X2
  uint64_t x2_width =
      internal_units_per_nm_ * static_cast<double>(parameters_.x2_width_nm);
  layout->AddRectangle(Rectangle(Point(135 + 410 + 145, 235),
                                 Point(1245, 235 + x2_width)));
  // X1
  uint64_t x1_width =
      internal_units_per_nm_ * static_cast<double>(parameters_.x1_width_nm);
  layout->AddRectangle(Rectangle(Point(135, 1695),
                                 Point(135 + 410 + 145, 1695 + x1_width)));
  // X3
  uint64_t x3_width =
      internal_units_per_nm_ * static_cast<double>(parameters_.x3_width_nm);
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

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
