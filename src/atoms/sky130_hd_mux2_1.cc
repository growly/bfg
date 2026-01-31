#include "sky130_hd_mux2_1.h"

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

bfg::Cell *Sky130HdMux21::Generate() {
  // This is the active 2:1 mux from the sky130 high-density standard cell
  // library:
  //                       /            /
  //      +-----------+   _|        _  _|
  //      |           +--|_  p1   --S-|_  p3
  //      |    /        I0 |         I2 |             /
  //      |   _|          _|           _|            _|
  //      +-o|_ p0   A0 o|_  p2   A1 o|_  p4  _  +-o|_  p5
  //      |    | _         |            |     X  |    |
  // S ---+    +-S---      +------------+--------+    +-- X
  //      |   _|          _|           _|        |   _|
  //      +--|_ n0   A1 -|_  n2   A0 -|_  n4     +--|_  n5
  //      |    |        I1 |         I3 |             |
  //      |    V          _|        _  _|             V
  //      |           +--|_  n1   --S-|_  n3
  //      +-----------+    |            |
  //                       V            V
  //
  // Per the sky130 PDK verilog description, the behaviour is:
  //
  // primitive sky130_fd_sc_hd__udp_mux_2to1 (
  //     X ,
  //     A0,
  //     A1,
  //     S
  // );
  //
  //     output X ;
  //     input  A0;
  //     input  A1;
  //     input  S ;
  //
  //     table
  //      //  A0  A1  S  :  X
  //          0   0   ?  :  0   ;
  //          1   1   ?  :  1   ;
  //          0   ?   0  :  0   ;
  //          1   ?   0  :  1   ;
  //          ?   0   1  :  0   ;
  //          ?   1   1  :  1   ;
  //     endtable
  // endprimitive

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_hd_mux2_1": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *Sky130HdMux21::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  circuit::Wire S = circuit->AddSignal("S");
  circuit::Wire A0 = circuit->AddSignal("A0");
  circuit::Wire A1 = circuit->AddSignal("A1");
  circuit::Wire X = circuit->AddSignal("X");
  circuit::Wire VPWR = circuit->AddSignal("VPWR");
  circuit::Wire VGND = circuit->AddSignal("VGND");
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(S);
  circuit->AddPort(A0);
  circuit->AddPort(A1);
  circuit->AddPort(X);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  bfg::Circuit *nfet_01v8 =
      design_db_->FindCellOrDie("sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  bfg::Circuit *pfet_01v8_hvt = design_db_->FindCellOrDie(
      "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();

  circuit::Instance *n0 = circuit->AddInstance("n0", nfet_01v8);
  circuit::Instance *n1 = circuit->AddInstance("n1", nfet_01v8);
  circuit::Instance *n2 = circuit->AddInstance("n2", nfet_01v8);
  circuit::Instance *n3 = circuit->AddInstance("n3", nfet_01v8);
  circuit::Instance *n4 = circuit->AddInstance("n4", nfet_01v8);
  circuit::Instance *n5 = circuit->AddInstance("n5", nfet_01v8);

  circuit::Instance *p0 = circuit->AddInstance("p0", pfet_01v8_hvt);
  circuit::Instance *p1 = circuit->AddInstance("p1", pfet_01v8_hvt);
  circuit::Instance *p2 = circuit->AddInstance("p2", pfet_01v8_hvt);
  circuit::Instance *p3 = circuit->AddInstance("p3", pfet_01v8_hvt);
  circuit::Instance *p4 = circuit->AddInstance("p4", pfet_01v8_hvt);
  circuit::Instance *p5 = circuit->AddInstance("p5", pfet_01v8_hvt);

  circuit::Wire S_B = circuit->AddSignal("S_B");
  circuit::Wire I0 = circuit->AddSignal("I0");
  circuit::Wire I1 = circuit->AddSignal("I1");
  circuit::Wire I2 = circuit->AddSignal("I2");
  circuit::Wire I3 = circuit->AddSignal("I3");
  circuit::Wire X_B = circuit->AddSignal("X_B");

  // For reference, this is the spice model for the sky130_fd_sc_hd__mux2_1 cell
  // given in the PDK:
  //
  // FET pin order:
  //    sky130_fd_pr__nfet_01v8 d g s b
  //    sky130_fd_pr__pfet_01v8 d g s b
  //    sky130_fd_pr__pfet_01v8_hvt d g s b
  //
  // .subckt sky130_fd_sc_hd__mux2_1 A0 A1 S VGND VNB VPB VPWR X
  // X0 VPWR S a_218_374# VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u
  // X1 a_76_199# A0 a_439_47# VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u
  // X2 a_535_374# a_505_21# VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u
  // X3 VPWR S a_505_21# VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u
  // X4 a_76_199# A1 a_535_374# VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u
  // X5 a_218_47# A1 a_76_199# VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u
  // X6 a_218_374# A0 a_76_199# VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u
  // X7 X a_76_199# VGND VNB sky130_fd_pr__nfet_01v8 w=650000u l=150000u
  // X8 X a_76_199# VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=1e+06u l=150000u
  // X9 VGND S a_218_47# VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u
  // X10 VGND S a_505_21# VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u
  // X11 a_439_47# a_505_21# VGND VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u
  //.ends
  //
  // If we rearrange this by hand and re-write the signal names according to
  // ours, we can do a manual graph isomorphism check and it makes more sense:
  //
  // .subckt sky130_fd_sc_hd__mux2_1 A0 A1 S VGND VNB VPB VPWR X
  // ; input buffer
  // X3 VPWR S S_B VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u    ; p0
  // X10 VGND S S_B VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u       ; n0
  //
  // X0 VPWR S I0 VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u     ; p1
  // X6 I0 A0 X_B VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u     ; p2
  // X5 I1 A1 X_B VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u         ; n2
  // X9 VGND S I1 VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u         ; n1
  //
  // X2 I2 S_B VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u   ; p3
  // X4 X_B A1 I2 VPB sky130_fd_pr__pfet_01v8_hvt w=420000u l=150000u     ; p4
  // X1 X_B A0 I3 VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u         ; n4
  // X11 I3 S_B VGND VNB sky130_fd_pr__nfet_01v8 w=420000u l=150000u      ; n3
  // 
  // ; output buffer
  // X8 X X_B VPWR VPB sky130_fd_pr__pfet_01v8_hvt w=1e+06u l=150000u     ; p5
  // X7 X X_B VGND VNB sky130_fd_pr__nfet_01v8 w=650000u l=150000u        ; n5
  //.ends

  p0->Connect({{"d", S_B}, {"s", VPWR}, {"g", S}, {"b", VPB}});
  n0->Connect({{"d", S_B}, {"s", VGND}, {"g", S}, {"b", VNB}});

  p5->Connect({{"d", X}, {"s", VPWR}, {"g", X_B}, {"b", VPB}});
  n5->Connect({{"d", X}, {"s", VGND}, {"g", X_B}, {"b", VNB}});

  // The substrate connections "b" correct on the inner transistors match those
  // in the sky130 PDK netlist.
  p1->Connect({{"d", I0}, {"s", VPWR}, {"g", S}, {"b", VPB}});
  p2->Connect({{"d", X_B}, {"s", I0}, {"g", A0}, {"b", VPB}});

  n2->Connect({{"d", X_B}, {"s", I1}, {"g", A1}, {"b", VNB}});
  n1->Connect({{"d", I1}, {"s", VGND}, {"g", S}, {"b", VNB}});

  p3->Connect({{"d", I2}, {"s", VPWR}, {"g", S_B}, {"b", VPB}});
  p4->Connect({{"d", X_B}, {"s", I2}, {"g", A1}, {"b", VPB}});

  n4->Connect({{"d", X_B}, {"s", I3}, {"g", A0}, {"b", VNB}});
  n3->Connect({{"d", I3}, {"s", VGND}, {"g", S_B}, {"b", VNB}});

  // TODO(aryap): This can definitely be factored out of here (and all the other
  // cells it appears in).
  struct FetParameters {
    circuit::Instance *instance;
    uint64_t width_nm;
    uint64_t length_nm;
  };
  std::array<FetParameters, 12> fet_parameters = {
    FetParameters {
      n0, parameters_.nfet_0_width_nm, parameters_.nfet_0_length_nm
    },
    FetParameters {
      n1, parameters_.nfet_1_width_nm, parameters_.nfet_1_length_nm
    },
    FetParameters {
      n2, parameters_.nfet_2_width_nm, parameters_.nfet_2_length_nm
    },
    FetParameters {
      n3, parameters_.nfet_3_width_nm, parameters_.nfet_3_length_nm
    },
    FetParameters {
      n4, parameters_.nfet_4_width_nm, parameters_.nfet_4_length_nm
    },
    FetParameters {
      n5, parameters_.nfet_5_width_nm, parameters_.nfet_5_length_nm
    },
    FetParameters {
      p0, parameters_.pfet_0_width_nm, parameters_.pfet_0_length_nm
    },
    FetParameters {
      p1, parameters_.pfet_1_width_nm, parameters_.pfet_1_length_nm
    },
    FetParameters {
      p2, parameters_.pfet_2_width_nm, parameters_.pfet_2_length_nm
    },
    FetParameters {
      p3, parameters_.pfet_3_width_nm, parameters_.pfet_3_length_nm
    },
    FetParameters {
      p4, parameters_.pfet_4_width_nm, parameters_.pfet_4_length_nm
    },
    FetParameters {
      p5, parameters_.pfet_5_width_nm, parameters_.pfet_5_length_nm
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

  return circuit.release();
}

bfg::Layout *Sky130HdMux21::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  // What is this?
  //layout->AddRectangle(Rectangle(Point(0, 0), Point(4140, 2720)));

  // nwell.pin [PIN] 64/16
  layout->SetActiveLayerByName("nwell.pin");
  layout->AddRectangle(Rectangle(Point(370, 2635), Point(540, 2805)));

  // pwell.pin [PIN] 122/16
  layout->SetActiveLayerByName("pwell.pin");
  layout->AddRectangle(Rectangle(Point(420, -85), Point(590, 85)));

  // met1.pin [PIN] 68/16
  layout->SetActiveLayerByName("met1.pin");
  layout->AddRectangle(Rectangle(Point(150, -85), Point(320, 85)));
  layout->AddRectangle(Rectangle(Point(150, 2635), Point(320, 2805)));

  // met1.drawing [DRAWING] 68/20
  layout->SetActiveLayerByName("met1.drawing");
  layout->AddRectangle(Rectangle(Point(0, 2480), Point(4140, 2960)));
  layout->AddRectangle(Rectangle(Point(0, -240), Point(4140, 240)));

  // li.pin [PIN] 67/16
  layout->SetActiveLayerByName("li.pin");
  geometry::Rectangle *pad = layout->AddRectangleAsPort(
      Rectangle(Point(150, 2125), Point(320, 2295)), "X");
  layout->SavePoint("port_X_centre_top", pad->centre());
  pad = layout->AddRectangleAsPort(
      Rectangle(Point(150, 1785), Point(320, 1955)), "X");
  layout->SavePoint("port_X_centre_middle", pad->centre());
  pad = layout->AddRectangleAsPort(
      Rectangle(Point(150, 425), Point(320, 595)), "X");
  layout->SavePoint("port_X_centre_bottom", pad->centre());
  layout->AddRectangleAsPort(
      Rectangle(Point(2450, 1105), Point(2620, 1275)), "A1");
  layout->AddRectangleAsPort(
      Rectangle(Point(2450, 765), Point(2620, 935)), "A1");
  layout->AddRectangleAsPort(
      Rectangle(Point(2910, 1445), Point(3080, 1615)), "S");
  layout->AddRectangleAsPort(
      Rectangle(Point(3370, 1445), Point(3540, 1615)), "S");
  layout->AddRectangleAsPort(
      Rectangle(Point(1990, 1105), Point(2160, 1275)), "A0");
  // Add a new A0 port that wasn't in the standard cell:
  layout->AddRectangleAsPort(
      Rectangle(Point(1990, 1105 - 680), Point(2160, 1275 - 680)), "A0");

  // licon.drawing [DRAWING] 66/44
  layout->SetActiveLayerByName("licon.drawing");
  layout->AddRectangle(Rectangle(Point(175, 2255), Point(345, 2425)));
  layout->AddRectangle(Rectangle(Point(3295, 1460), Point(3465, 1630)));
  layout->AddRectangle(Rectangle(Point(1775, 1955), Point(1945, 2125)));
  layout->AddRectangle(Rectangle(Point(175, 1915), Point(345, 2085)));
  layout->AddRectangle(Rectangle(Point(1630, 370), Point(1800, 540)));
  layout->AddRectangle(Rectangle(Point(3135, 1955), Point(3305, 2125)));
  layout->AddRectangle(Rectangle(Point(1615, 895), Point(1785, 1065)));
  layout->AddRectangle(Rectangle(Point(3565, 1955), Point(3735, 2125)));
  layout->AddRectangle(Rectangle(Point(2905, 895), Point(3075, 1065)));
  layout->AddRectangle(Rectangle(Point(3145, 370), Point(3315, 540)));
  layout->AddRectangle(Rectangle(Point(2805, 370), Point(2975, 540)));
  layout->AddRectangle(Rectangle(Point(2455, 1460), Point(2625, 1630)));
  layout->AddRectangle(Rectangle(Point(910, 1075), Point(1080, 1245)));
  layout->AddRectangle(Rectangle(Point(595, 295), Point(765, 465)));
  layout->AddRectangle(Rectangle(Point(2150, 1955), Point(2320, 2125)));
  layout->AddRectangle(Rectangle(Point(3565, 370), Point(3735, 540)));
  layout->AddRectangle(Rectangle(Point(2095, 895), Point(2265, 1065)));
  layout->AddRectangle(Rectangle(Point(595, 1575), Point(765, 1745)));
  layout->AddRectangle(Rectangle(Point(595, 1915), Point(765, 2085)));
  layout->AddRectangle(Rectangle(Point(595, 2255), Point(765, 2425)));
  layout->AddRectangle(Rectangle(Point(430, 1075), Point(600, 1245)));
  layout->AddRectangle(Rectangle(Point(175, 390), Point(345, 560)));
  layout->AddRectangle(Rectangle(Point(175, 1575), Point(345, 1745)));

  // psdm.drawing [DRAWING] 94/20
  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddPolygon(Polygon({Point(0, 1355),
                              Point(2345, 1355),
                              Point(2345, 1740),
                              Point(2735, 1740),
                              Point(2735, 1355),
                              Point(3185, 1355),
                              Point(3185, 1740),
                              Point(3575, 1740),
                              Point(3575, 1355),
                              Point(4140, 1355),
                              Point(4140, 2910),
                              Point(0, 2910)}));

  // poly.drawing [DRAWING] 66/20
  layout->SetActiveLayerByName("poly.drawing");
  layout->AddPolygon(Polygon({Point(380, 995),
                              Point(395, 995),
                              Point(395, 105),
                              Point(545, 105),
                              Point(545, 995),
                              Point(650, 995),
                              Point(650, 1325),
                              Point(545, 1325),
                              Point(545, 2615),
                              Point(395, 2615),
                              Point(395, 1325),
                              Point(380, 1325)}));
  layout->AddPolygon(Polygon({Point(860, 995),
                              Point(940, 995),
                              Point(940, 105),
                              Point(1090, 105),
                              Point(1090, 995),
                              Point(1130, 995),
                              Point(1130, 1325),
                              Point(1090, 1325),
                              Point(1090, 2420),
                              Point(940, 2420),
                              Point(940, 1325),
                              Point(860, 1325)}));
  layout->AddPolygon(Polygon({Point(3245, 1380),
                              Point(3365, 1380),
                              Point(3365, 105),
                              Point(3515, 105),
                              Point(3515, 2420),
                              Point(3365, 2420),
                              Point(3365, 1710),
                              Point(3245, 1710)}));
  layout->AddPolygon(Polygon({Point(2525, 105),
                              Point(2675, 105),
                              Point(2675, 845),
                              Point(3155, 845),
                              Point(3155, 1115),
                              Point(3035, 1115),
                              Point(3035, 2420),
                              Point(2885, 2420),
                              Point(2885, 1115),
                              Point(2825, 1115),
                              Point(2825, 995),
                              Point(2525, 995)}));
  layout->AddPolygon(Polygon({Point(2405, 1380),
                              Point(2675, 1380),
                              Point(2675, 2420),
                              Point(2525, 2420),
                              Point(2525, 1710),
                              Point(2405, 1710)}));
  layout->AddPolygon(Polygon({Point(1420, 105),
                              Point(1570, 105),
                              Point(1570, 815),
                              Point(1835, 815),
                              Point(1835, 1145),
                              Point(1420, 1145)}));
  layout->AddPolygon(Polygon({Point(2045, 105),
                              Point(2195, 105),
                              Point(2195, 815),
                              Point(2315, 815),
                              Point(2315, 1145),
                              Point(2195, 1145),
                              Point(2195, 1795),
                              Point(1605, 1795),
                              Point(1605, 2420),
                              Point(1455, 2420),
                              Point(1455, 1645),
                              Point(2045, 1645)}));

  // diff.drawing [DRAWING] 65/20
  layout->SetActiveLayerByName("diff.drawing");
  layout->AddPolygon(Polygon({Point(135, 235),
                              Point(3775, 235),
                              Point(3775, 655),
                              Point(805, 655),
                              Point(805, 885),
                              Point(135, 885)}));
  layout->AddPolygon(Polygon({Point(135, 1485),
                              Point(805, 1485),
                              Point(805, 1870),
                              Point(3795, 1870),
                              Point(3795, 2290),
                              Point(805, 2290),
                              Point(805, 2485),
                              Point(135, 2485)}));

  // mcon.drawing [DRAWING] 67/44
  layout->SetActiveLayerByName("mcon.drawing");
  layout->AddRectangle(Rectangle(Point(3365, -85), Point(3535, 85)));
  layout->AddRectangle(Rectangle(Point(3825, 2635), Point(3995, 2805)));
  layout->AddRectangle(Rectangle(Point(3825, -85), Point(3995, 85)));
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));
  layout->AddRectangle(Rectangle(Point(605, 2635), Point(775, 2805)));
  layout->AddRectangle(Rectangle(Point(605, -85), Point(775, 85)));
  layout->AddRectangle(Rectangle(Point(1065, -85), Point(1235, 85)));
  layout->AddRectangle(Rectangle(Point(1525, 2635), Point(1695, 2805)));
  layout->AddRectangle(Rectangle(Point(1525, -85), Point(1695, 85)));
  layout->AddRectangle(Rectangle(Point(1985, -85), Point(2155, 85)));
  layout->AddRectangle(Rectangle(Point(2445, 2635), Point(2615, 2805)));
  layout->AddRectangle(Rectangle(Point(2445, -85), Point(2615, 85)));
  layout->AddRectangle(Rectangle(Point(2905, -85), Point(3075, 85)));
  layout->AddRectangle(Rectangle(Point(3365, 2635), Point(3535, 2805)));
  layout->AddRectangle(Rectangle(Point(2905, 2635), Point(3075, 2805)));
  layout->AddRectangle(Rectangle(Point(1985, 2635), Point(2155, 2805)));
  layout->AddRectangle(Rectangle(Point(1065, 2635), Point(1235, 2805)));
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  Rectangle *tiling_bounds = layout->AddRectangle(
      Rectangle(Point(0, 0), Point(4140, 2720)));
  layout->SetTilingBounds(*tiling_bounds);

  // li.drawing [DRAWING] 67/20
  layout->SetActiveLayerByName("li.drawing");
  layout->AddRectangle(Rectangle(Point(1990, 255), Point(2265, 1415)));
  layout->AddPolygon(Polygon({Point(0, -85),
                              Point(4140, -85),
                              Point(4140, 85),
                              Point(3315, 85),
                              Point(3315, 620),
                              Point(2805, 620),
                              Point(2805, 85),
                              Point(845, 85),
                              Point(845, 485),
                              Point(515, 485),
                              Point(515, 85),
                              Point(0, 85)}));
  layout->AddPolygon(Polygon({Point(0, 2635),
                              Point(595, 2635),
                              Point(595, 1495),
                              Point(765, 1495),
                              Point(765, 2635),
                              Point(3135, 2635),
                              Point(3135, 1875),
                              Point(3305, 1875),
                              Point(3305, 2635),
                              Point(4140, 2635),
                              Point(4140, 2805),
                              Point(0, 2805)}));
  layout->AddPolygon(Polygon({Point(90, 255),
                              Point(345, 255),
                              Point(345, 825),
                              Point(260, 825),
                              Point(260, 1495),
                              Point(425, 1495),
                              Point(425, 2465),
                              Point(90, 2465)}));
  layout->AddPolygon(Polygon({Point(515, 655),
                              Point(1270, 655),
                              Point(1270, 255),
                              Point(1800, 255),
                              Point(1800, 620),
                              Point(1445, 620),
                              Point(1445, 1955),
                              Point(2400, 1955),
                              Point(2400, 2125),
                              Point(1275, 2125),
                              Point(1275, 825),
                              Point(685, 825),
                              Point(685, 1325),
                              Point(430, 1325),
                              Point(430, 995),
                              Point(515, 995)}));
  layout->AddPolygon(Polygon({Point(1615, 815),
                              Point(1785, 815),
                              Point(1785, 1615),
                              Point(2435, 1615),
                              Point(2435, 255),
                              Point(2625, 255),
                              Point(2625, 1785),
                              Point(1615, 1785)}));
  layout->AddPolygon(Polygon({Point(910, 995),
                              Point(1105, 995),
                              Point(1105, 2295),
                              Point(2795, 2295),
                              Point(2795, 1440),
                              Point(3545, 1440),
                              Point(3545, 1630),
                              Point(2965, 1630),
                              Point(2965, 2465),
                              Point(935, 2465),
                              Point(935, 1325),
                              Point(910, 1325)}));
  layout->AddPolygon(Polygon({Point(2825, 895),
                              Point(3535, 895),
                              Point(3535, 290),
                              Point(3780, 290),
                              Point(3780, 895),
                              Point(4055, 895),
                              Point(4055, 2285),
                              Point(3540, 2285),
                              Point(3540, 1875),
                              Point(3715, 1875),
                              Point(3715, 1065),
                              Point(2825, 1065)}));

  // nwell.drawing [DRAWING] 64/20
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle(Rectangle(Point(-190, 1305), Point(4330, 2910)));

  // nsdm.drawing [DRAWING] 93/44
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle(Rectangle(Point(0, -190), Point(4140, 1015)));

  // hvtp.drawing [DRAWING] 78/44
  layout->SetActiveLayerByName("hvtp.drawing");
  layout->AddRectangle(Rectangle(Point(0, 1250), Point(4140, 2720)));

  // npc.drawing [DRAWING] 95/20
  layout->SetActiveLayerByName("npc.drawing");
  layout->AddPolygon(Polygon({Point(0, 975),
                              Point(1515, 975),
                              Point(1515, 795),
                              Point(2365, 795),
                              Point(2365, 975),
                              Point(2805, 975),
                              Point(2805, 795),
                              Point(3175, 795),
                              Point(3175, 975),
                              Point(4140, 975),
                              Point(4140, 1345),
                              Point(3565, 1345),
                              Point(3565, 1730),
                              Point(3195, 1730),
                              Point(3195, 1345),
                              Point(2725, 1345),
                              Point(2725, 1730),
                              Point(2355, 1730),
                              Point(2355, 1345),
                              Point(0, 1345)}));

  return layout.release();
}

}  // namespace atoms
}  // namespace atoms
