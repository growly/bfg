#include "sky130_dfxtp.h"

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

bfg::Cell *Sky130Dfxtp::Generate() {
  // A flip-flop is two back-to-back latches.

  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_dfxtp"));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  return cell.release();
}

bfg::Circuit *Sky130Dfxtp::GenerateCircuit() {
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

  // 18 transistors in flip-flop, 2 in output buffer.

  // These are all sky130_fd_pr__nfet_01v8:
  Circuit *nfet_01v8 = design_db_->FindCellOrDie(
      "sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  circuit::Instance *nfet_0 = circuit->AddInstance("nfet_0", nfet_01v8);
  circuit::Instance *nfet_1 = circuit->AddInstance("nfet_1", nfet_01v8);
  circuit::Instance *nfet_2 = circuit->AddInstance("nfet_2", nfet_01v8);
  circuit::Instance *nfet_3 = circuit->AddInstance("nfet_3", nfet_01v8);
  circuit::Instance *nfet_4 = circuit->AddInstance("nfet_4", nfet_01v8);
  circuit::Instance *nfet_5 = circuit->AddInstance("nfet_5", nfet_01v8);
  circuit::Instance *nfet_6 = circuit->AddInstance("nfet_6", nfet_01v8);
  circuit::Instance *nfet_7 = circuit->AddInstance("nfet_7", nfet_01v8);
  circuit::Instance *nfet_8 = circuit->AddInstance("nfet_8", nfet_01v8);
  circuit::Instance *nfet_9 = circuit->AddInstance("nfet_9", nfet_01v8);

  // These are all sky130_fd_pr__pfet_01v8_hvt:
  Circuit *pfet_01v8_hvt = design_db_->FindCellOrDie(
      "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();
  circuit::Instance *pfet_0 = circuit->AddInstance("pfet_0", pfet_01v8_hvt);
  circuit::Instance *pfet_1 = circuit->AddInstance("pfet_1", pfet_01v8_hvt);
  circuit::Instance *pfet_2 = circuit->AddInstance("pfet_2", pfet_01v8_hvt);
  circuit::Instance *pfet_3 = circuit->AddInstance("pfet_3", pfet_01v8_hvt);
  circuit::Instance *pfet_4 = circuit->AddInstance("pfet_4", pfet_01v8_hvt);
  circuit::Instance *pfet_5 = circuit->AddInstance("pfet_5", pfet_01v8_hvt);
  circuit::Instance *pfet_6 = circuit->AddInstance("pfet_6", pfet_01v8_hvt);
  circuit::Instance *pfet_7 = circuit->AddInstance("pfet_7", pfet_01v8_hvt);
  circuit::Instance *pfet_8 = circuit->AddInstance("pfet_8", pfet_01v8_hvt);
  circuit::Instance *pfet_9 = circuit->AddInstance("pfet_9", pfet_01v8_hvt);

  //                    /                    /
  //                   _|                   _|
  //      +----------o|_ pfet0   +-------+o|_  pfet2
  //      |             |        |           |
  //      |  +----------+        |           d
  //      |  |         _|        c          _|
  //      |  |  CLK -o|_ pfet1   |  CLKI -o|_  pfet3
  //      |  |          |        |           |
  //      |  a          |        +--------------------- c
  //      |  |          |        |           |
  //   D -+  |          +--- b --|-----------+--------- b
  //      |  |          |        |           |
  //      |  |         _|        |          _|
  //      |  |  CLKI -|_ nfet1   |    CLK -|_  nfet3
  //      |  |          |        |           |
  //      |  +----------+        |           e
  //      |            _|        |          _|
  //      +-----------|_ nfet0   +---------|_  nfet2
  //                    |                    |
  //                    V                    V


  circuit::Wire a = circuit->AddSignal("a");
  circuit::Wire b = circuit->AddSignal("b");

  pfet_0->Connect({{"d", a}, {"g", D}, {"s", VPWR}, {"b", VPB}});
  nfet_0->Connect({{"d", a}, {"g", D}, {"s", VGND}, {"b", VNB}});
  pfet_1->Connect({{"d", b}, {"g", CLKI}, {"s", a}, {"b", VPB}});
  nfet_1->Connect({{"d", b}, {"g", CLK}, {"s", a}, {"b", VNB}});

  circuit::Wire c = circuit->AddSignal("c");
  circuit::Wire d = circuit->AddSignal("d");
  circuit::Wire e = circuit->AddSignal("e");

  pfet_2->Connect({{"d", d}, {"g", c}, {"s", VPWR}, {"b", VPB}});
  nfet_2->Connect({{"d", e}, {"g", c}, {"s", VGND}, {"b", VNB}});
  pfet_3->Connect({{"d", b}, {"g", CLKI}, {"s", d}, {"b", VPB}});
  nfet_3->Connect({{"d", b}, {"g", CLK}, {"s", e}, {"b", VNB}});

  //                    /                    /
  //                   _|                   _|
  //      +----------o|_ pfet4   +-------+o|_  pfet6
  //      |             |        |           |
  //      |  +----------+        |           h
  //      |  |         _|        g          _|
  //      |  | CLKI -o|_ pfet5   |   CLK -o|_  pfet7
  //      |  |          |        |           |
  //   c ----+          |        +--------------------- Q_N
  //      |  |          |        |           |
  //   b -+  |          +--- f --------------+--------- f
  //      |  |          |        |           |
  //      |  |         _|        |          _|
  //      |  |   CLK -|_ nfet5   |   CLKI -|_  nfet7
  //      |  |          |        |           |
  //      |  +-----c----+        |          (i) // TODO: Don't use i
  //      |            _|        |          _|
  //      +-----------|_ nfet4   +---------|_  nfet6
  //                    |                    |
  //                    V                    V

  circuit::Wire f = circuit->AddSignal("f");

  pfet_4->Connect({{"d", c}, {"g", b}, {"s", VPWR}, {"b", VPB}});
  nfet_4->Connect({{"d", c}, {"g", b}, {"s", VGND}, {"b", VNB}});
  pfet_5->Connect({{"d", f}, {"g", CLKI}, {"s", c}, {"b", VPB}});
  nfet_5->Connect({{"d", f}, {"g", CLK}, {"s", c}, {"b", VNB}});

  circuit::Wire Q_N = circuit->AddSignal("Q_N");
  circuit::Wire h = circuit->AddSignal("h");
  circuit::Wire i = circuit->AddSignal("i");

  pfet_6->Connect({{"d", h}, {"g", Q_N}, {"s", VPWR}, {"b", VPB}});
  nfet_6->Connect({{"d", i}, {"g", Q_N}, {"s", VGND}, {"b", VNB}});
  pfet_7->Connect({{"d", f}, {"g", CLK}, {"s", h}, {"b", VPB}});
  nfet_7->Connect({{"d", f}, {"g", CLKI}, {"s", i}, {"b", VNB}});

  //               /                    /
  //              _|                   _|
  //      +-----o|_ pfet4   +-------+o|_  pfet6
  //      |        |        |           |
  //      |        |        |           |
  //   f -+        |        |           |
  //      |        |        |           +-- Q
  // Q_N ----------+-- Q_N -+           |
  //      |        |        |           |
  //      |       _|        |          _|
  //      +------|_ nfet4   +---------|_  nfet6
  //               |                    |
  //               V                    V

  pfet_8->Connect({{"d", Q_N}, {"g", f}, {"s", VPWR}, {"b", VPB}});
  nfet_8->Connect({{"d", Q_N}, {"g", f}, {"s", VGND}, {"b", VNB}});

  pfet_9->Connect({{"d", Q}, {"g", f}, {"s", VPWR}, {"b", VPB}});
  nfet_9->Connect({{"d", Q}, {"g", f}, {"s", VGND}, {"b", VNB}});

  return circuit.release();
}

bfg::Layout *Sky130Dfxtp::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  // TODO(aryap): The layout below must be parameterised according to the
  // parameters in our Parameters struct. At the very least, the pertinent ones
  // we want to mess with (output buffer?)

  // TODO(aryap): A fundamental problem with how these standard cells are
  // constructed is that the mcon vias from met1 down to li need to overlap
  // completely, otherwise they cause a DRC error.
  //
  // The cells are normally constructed in multiples of a site width (460 nm) so
  // that all mcon vias always line up. But by truncating our flip flop we have
  // to either squeeze into 1 unit of this width less, or sacrifice the wasted
  // space of going 1 unit of width more.
  //
  // It might be possible to cut the flip-flop short and squish the li pour on
  // the left side to accommodate, but this would cramp the access pin. Consider
  // that the adjacent cell would have an output close by.
  //
  // I'm leaving this TODO for when we have the luxury of precision-manipulating
  // the layouts for performance.
  //
  // An alternative idea, that breaks the standard-cell rules, is to simply omit
  // mcon drawings on either the VSS or VDD rails when the cell is flipped for
  // placement. This could be an option. I don't think that would violate
  // spacing rules.
  // int64_t x_min = 80;
  int64_t x_min = 20 - 460;

  layout->AddRectangle(Rectangle(Point(x_min, 0), Point(6000, 2720)));

  // mcon.drawing [DRAWING] 67/44
  layout->SetActiveLayerByName("mcon.drawing");
  layout->AddRectangle(Rectangle(Point(1545, -85), Point(1715, 85)));
  layout->AddRectangle(Rectangle(Point(2465, -85), Point(2635, 85)));
  layout->AddRectangle(Rectangle(Point(2005, -85), Point(2175, 85)));
  layout->AddRectangle(Rectangle(Point(2925, 2635), Point(3095, 2805)));
  layout->AddRectangle(Rectangle(Point(2925, -85), Point(3095, 85)));
  layout->AddRectangle(Rectangle(Point(2940, 1785), Point(3110, 1955)));
  layout->AddRectangle(Rectangle(Point(3375, 1445), Point(3545, 1615)));
  layout->AddRectangle(Rectangle(Point(3385, 2635), Point(3555, 2805)));
  layout->AddRectangle(Rectangle(Point(3385, -85), Point(3555, 85)));
  layout->AddRectangle(Rectangle(Point(3845, 2635), Point(4015, 2805)));
  layout->AddRectangle(Rectangle(Point(3845, -85), Point(4015, 85)));
  layout->AddRectangle(Rectangle(Point(4305, 2635), Point(4475, 2805)));
  layout->AddRectangle(Rectangle(Point(4305, -85), Point(4475, 85)));
  layout->AddRectangle(Rectangle(Point(4765, 2635), Point(4935, 2805)));
  layout->AddRectangle(Rectangle(Point(4765, -85), Point(4935, 85)));
  layout->AddRectangle(Rectangle(Point(5225, 2635), Point(5395, 2805)));
  layout->AddRectangle(Rectangle(Point(5225, -85), Point(5395, 85)));
  layout->AddRectangle(Rectangle(Point(5685, 2635), Point(5855, 2805)));
  layout->AddRectangle(Rectangle(Point(5685, -85), Point(5855, 85)));
  layout->AddRectangle(Rectangle(Point(165, 2635), Point(335, 2805)));
  layout->AddRectangle(Rectangle(Point(625, 2635), Point(795, 2805)));
  layout->AddRectangle(Rectangle(Point(625, -85), Point(795, 85)));
  layout->AddRectangle(Rectangle(Point(1085, 2635), Point(1255, 2805)));
  layout->AddRectangle(Rectangle(Point(1370, 1785), Point(1540, 1955)));
  layout->AddRectangle(Rectangle(Point(165, -85), Point(335, 85)));
  layout->AddRectangle(Rectangle(Point(855, 1445), Point(1025, 1615)));
  layout->AddRectangle(Rectangle(Point(1085, -85), Point(1255, 85)));
  layout->AddRectangle(Rectangle(Point(1545, 2635), Point(1715, 2805)));
  layout->AddRectangle(Rectangle(Point(2465, 2635), Point(2635, 2805)));
  layout->AddRectangle(Rectangle(Point(2005, 2635), Point(2175, 2805)));

  // Additional mcon added to correct for cut in non-unit-widths:
  layout->AddRectangle(Rectangle(
        Point(x_min + 145, 2635),
        Point(x_min + 145 + 170, 2805)));
  layout->AddRectangle(Rectangle(
        Point(x_min + 145, -85),
        Point(x_min + 145 + 170, 85)));

  // met1.drawing [DRAWING] 68/20
  layout->SetActiveLayerByName("met1.drawing");
  layout->AddRectangle(Rectangle(Point(x_min, 2480), Point(6000, 2960)));
  layout->AddRectangle(Rectangle(Point(x_min, -240), Point(6000, 240)));
  layout->AddPolygon(Polygon({//Point(0, 1800),
                              //Point(1310, 1800),
                              Point(1310, 1755),
                              Point(1600, 1755),
                              Point(1600, 1800),
                              Point(2880, 1800),
                              Point(2880, 1755),
                              Point(3170, 1755),
                              Point(3170, 1985),
                              Point(2880, 1985),
                              Point(2880, 1940),
                              Point(1600, 1940),
                              Point(1600, 1985),
                              Point(1310, 1985)//,
                              //Point(1310, 1940),
                              //Point(0, 1940)
                              }));
  layout->AddPolygon(Polygon({//Point(0, 1460),
                              //Point(795, 1460),
                              Point(795, 1415),
                              Point(1085, 1415),
                              Point(1085, 1460),
                              Point(3315, 1460),
                              Point(3315, 1415),
                              Point(3605, 1415),
                              Point(3605, 1645),
                              Point(3315, 1645),
                              Point(3315, 1600),
                              Point(1085, 1600),
                              Point(1085, 1645),
                              Point(795, 1645),
                              //Point(795, 1600),
                              //Point(0, 1600)
                              }));

  // diff.drawing [DRAWING] 65/20
  layout->SetActiveLayerByName("diff.drawing");
  layout->AddRectangle(Rectangle(Point(4715, 1485), Point(5815, 2485)));
  layout->AddRectangle(Rectangle(Point(4735, 235), Point(5825, 885)));
  layout->AddPolygon(Polygon({Point(135, 2065),
                              Point(2035, 2065),
                              Point(2035, 1735),
                              Point(2870, 1735),
                              Point(2870, 2065),
                              Point(4350, 2065),
                              Point(4350, 2485),
                              Point(135, 2485)}));
  layout->AddPolygon(Polygon({Point(135, 235),
                              Point(4460, 235),
                              Point(4460, 655),
                              Point(3800, 655),
                              Point(3800, 595),
                              Point(2940, 595),
                              Point(2940, 875),
                              Point(2290, 875),
                              Point(2290, 655),
                              Point(1695, 655),
                              Point(1695, 595),
                              Point(795, 595),
                              Point(795, 655),
                              Point(135, 655)}));

  // licon.drawing [DRAWING] 66/44
  layout->SetActiveLayerByName("licon.drawing");
  layout->AddRectangle(Rectangle(Point(600, 365), Point(770, 535)));
  layout->AddRectangle(Rectangle(Point(835, 1265), Point(1005, 1435)));
  layout->AddRectangle(Rectangle(Point(935, 785), Point(1105, 955)));
  layout->AddRectangle(Rectangle(Point(1055, 2190), Point(1225, 2360)));
  layout->AddRectangle(Rectangle(Point(1155, 365), Point(1325, 535)));
  layout->AddRectangle(Rectangle(Point(1380, 1655), Point(1550, 1825)));
  layout->AddRectangle(Rectangle(Point(1930, 845), Point(2100, 1015)));
  layout->AddRectangle(Rectangle(Point(2150, 335), Point(2320, 505)));
  layout->AddRectangle(Rectangle(Point(2250, 2255), Point(2420, 2425)));
  layout->AddRectangle(Rectangle(Point(2250, 1915), Point(2420, 2085)));
  layout->AddRectangle(Rectangle(Point(2250, 1325), Point(2420, 1495)));
  layout->AddRectangle(Rectangle(Point(2670, 2215), Point(2840, 2385)));
  layout->AddRectangle(Rectangle(Point(5195, 445), Point(5365, 615)));
  layout->AddRectangle(Rectangle(Point(5405, 1075), Point(5575, 1245)));
  layout->AddRectangle(Rectangle(Point(4755, 1545), Point(4925, 1715)));
  layout->AddRectangle(Rectangle(Point(5605, 2225), Point(5775, 2395)));
  layout->AddRectangle(Rectangle(Point(4775, 640), Point(4945, 810)));
  layout->AddRectangle(Rectangle(Point(4775, 300), Point(4945, 470)));
  layout->AddRectangle(Rectangle(Point(5185, 2105), Point(5355, 2275)));
  layout->AddRectangle(Rectangle(Point(5605, 1885), Point(5775, 2055)));
  layout->AddRectangle(Rectangle(Point(5605, 1545), Point(5775, 1715)));
  layout->AddRectangle(Rectangle(Point(5615, 650), Point(5785, 820)));
  layout->AddRectangle(Rectangle(Point(2745, 365), Point(2915, 535)));
  layout->AddRectangle(Rectangle(Point(2930, 1325), Point(3100, 1495)));
  layout->AddRectangle(Rectangle(Point(3145, 2165), Point(3315, 2335)));
  layout->AddRectangle(Rectangle(Point(3145, 785), Point(3315, 955)));
  layout->AddRectangle(Rectangle(Point(3260, 365), Point(3430, 535)));
  layout->AddRectangle(Rectangle(Point(3440, 1655), Point(3610, 1825)));
  layout->AddRectangle(Rectangle(Point(4120, 1655), Point(4290, 1825)));
  layout->AddRectangle(Rectangle(Point(5615, 310), Point(5785, 480)));
  layout->AddRectangle(Rectangle(Point(4140, 2215), Point(4310, 2385)));
  layout->AddRectangle(Rectangle(Point(4755, 1900), Point(4925, 2070)));
  layout->AddRectangle(Rectangle(Point(5185, 1705), Point(5355, 1875)));
  layout->AddRectangle(Rectangle(Point(175, 295), Point(345, 465)));
  layout->AddRectangle(Rectangle(Point(4250, 365), Point(4420, 535)));
  layout->AddRectangle(Rectangle(Point(120, 1415), Point(290, 1585)));
  layout->AddRectangle(Rectangle(Point(4485, 1075), Point(4655, 1245)));
  layout->AddRectangle(Rectangle(Point(595, 2215), Point(765, 2385)));
  layout->AddRectangle(Rectangle(Point(4755, 2255), Point(4925, 2425)));
  layout->AddRectangle(Rectangle(Point(175, 2255), Point(345, 2425)));

  // li.drawing [DRAWING] 67/20
  layout->SetActiveLayerByName("li.drawing");
  layout->AddRectangle(Rectangle(Point(10, 715), Point(290, 1665)));
  layout->AddRectangle(Rectangle(Point(800, 1125), Point(1040, 1720)));
  layout->AddRectangle(Rectangle(Point(2930, 1245), Point(3120, 1965)));
  layout->AddPolygon(Polygon({Point(3165, 365),
                              Point(3950, 365),
                              Point(3950, 995),
                              Point(4655, 995),
                              Point(4655, 1325),
                              Point(3950, 1325),
                              Point(3950, 2335),
                              Point(3065, 2335),
                              Point(3065, 2165),
                              Point(3780, 2165),
                              Point(3780, 535),
                              Point(3165, 535)}));
  layout->AddPolygon(Polygon({Point(4695, 300),
                              Point(5025, 300),
                              Point(5025, 995),
                              Point(5575, 995),
                              Point(5575, 1325),
                              Point(5015, 1325),
                              Point(5015, 2465),
                              Point(4675, 2465),
                              Point(4675, 1905),
                              Point(4120, 1905),
                              Point(4120, 1530),
                              Point(4825, 1530),
                              Point(4825, 825),
                              Point(4695, 825)}));
  layout->AddPolygon(Polygon({Point(5525, 1495),
                              Point(5700, 1495),
                              Point(5700, 1445),
                              Point(5745, 1445),
                              Point(5745, 865),
                              Point(5690, 865),
                              Point(5690, 825),
                              Point(5535, 825),
                              Point(5535, 305),
                              Point(5865, 305),
                              Point(5865, 740),
                              Point(5915, 740),
                              Point(5915, 1575),
                              Point(5855, 1575),
                              Point(5855, 2420),
                              Point(5525, 2420)}));
  layout->AddPolygon(Polygon({Point(3145, 705),
                              Point(3610, 705),
                              Point(3610, 1995),
                              Point(3290, 1995),
                              Point(3290, 1035),
                              Point(3145, 1035)}));
  layout->AddPolygon(Polygon({Point(x_min, 2635),
                              Point(80, 2635),
                              Point(80, 2175),
                              Point(345, 2175),
                              Point(345, 2635),
                              Point(2250, 2635),
                              Point(2250, 1835),
                              Point(2420, 1835),
                              Point(2420, 2635),
                              Point(4130, 2635),
                              Point(4130, 2135),
                              Point(4445, 2135),
                              Point(4445, 2635),
                              Point(5185, 2635),
                              Point(5185, 1625),
                              Point(5355, 1625),
                              Point(5355, 2635),
                              Point(6000, 2635),
                              Point(6000, 2805),
                              Point(x_min, 2805)}));
  layout->AddPolygon(Polygon({Point(x_min, -85),
                              Point(6000, -85),
                              Point(6000, 85),
                              Point(5365, 85),
                              Point(5365, 695),
                              Point(5195, 695),
                              Point(5195, 85),
                              Point(4435, 85),
                              Point(4435, 615),
                              Point(4225, 615),
                              Point(4225, 85),
                              Point(2410, 85),
                              Point(2410, 585),
                              Point(2040, 585),
                              Point(2040, 85),
                              Point(345, 85),
                              Point(345, 545),
                              Point(95, 545),
                              Point(95, 85),
                              Point(x_min, 85)}));
  layout->AddPolygon(Polygon({Point(515, 365),
                              Point(850, 365),
                              Point(850, 535),
                              Point(685, 535),
                              Point(685, 805),
                              Point(630, 805),
                              Point(630, 1910),
                              Point(765, 1910),
                              Point(765, 2465),
                              Point(515, 2465),
                              Point(515, 2040),
                              Point(460, 2040),
                              Point(460, 675),
                              Point(515, 675)}));
  layout->AddPolygon(Polygon({Point(855, 735),
                              Point(1380, 735),
                              Point(1380, 1655),
                              Point(1740, 1655),
                              Point(1740, 2020),
                              Point(1210, 2020),
                              Point(1210, 955),
                              Point(855, 955)}));
  layout->AddPolygon(Polygon({Point(1045, 365),
                              Point(1720, 365),
                              Point(1720, 1315),
                              Point(2250, 1315),
                              Point(2250, 1245),
                              Point(2420, 1245),
                              Point(2420, 1575),
                              Point(2080, 1575),
                              Point(2080, 2360),
                              Point(975, 2360),
                              Point(975, 2190),
                              Point(1910, 2190),
                              Point(1910, 1485),
                              Point(1550, 1485),
                              Point(1550, 535),
                              Point(1045, 535)}));
  layout->AddPolygon(Polygon({Point(2590, 365),
                              Point(2995, 365),
                              Point(2995, 535),
                              Point(2760, 535),
                              Point(2760, 2135),
                              Point(2840, 2135),
                              Point(2840, 2465),
                              Point(2590, 2465),
                              Point(2590, 1065),
                              Point(2130, 1065),
                              Point(2130, 1095),
                              Point(1930, 1095),
                              Point(1930, 765),
                              Point(2590, 765)}));

  // poly.drawing [DRAWING] 66/20
  layout->SetActiveLayerByName("poly.drawing");
  layout->AddPolygon(Polygon({Point(1810, 795),
                              Point(1945, 795),
                              Point(1945, 105),
                              Point(2095, 105),
                              Point(2095, 795),
                              Point(2180, 795),
                              Point(2180, 1065),
                              Point(1960, 1065),
                              Point(1960, 2615),
                              Point(1810, 2615)}));
  layout->AddPolygon(Polygon({Point(855, 735),
                              Point(955, 735),
                              Point(955, 105),
                              Point(1105, 105),
                              Point(1105, 735),
                              Point(1185, 735),
                              Point(1185, 1005),
                              Point(855, 1005)}));
  layout->AddPolygon(Polygon({Point(70, 1335),
                              Point(395, 1335),
                              Point(395, 105),
                              Point(545, 105),
                              Point(545, 2615),
                              Point(395, 2615),
                              Point(395, 1665),
                              Point(70, 1665)}));
  layout->AddPolygon(Polygon({Point(755, 1215),
                              Point(1450, 1215),
                              Point(1450, 105),
                              Point(1600, 105),
                              Point(1600, 1365),
                              Point(1085, 1365),
                              Point(1085, 1485),
                              Point(970, 1485),
                              Point(970, 2615),
                              Point(820, 2615),
                              Point(820, 1485),
                              Point(755, 1485)}));
  layout->AddPolygon(Polygon({Point(1295, 1575),
                              Point(1600, 1575),
                              Point(1600, 1905),
                              Point(1445, 1905),
                              Point(1445, 2615),
                              Point(1295, 2615)}));
  layout->AddPolygon(Polygon({Point(3935, 1575),
                              Point(4050, 1575),
                              Point(4050, 105),
                              Point(4200, 105),
                              Point(4200, 1575),
                              Point(4340, 1575),
                              Point(4340, 1905),
                              Point(4085, 1905),
                              Point(4085, 2615),
                              Point(3935, 2615)}));
  layout->AddPolygon(Polygon({Point(3360, 1605),
                              Point(3690, 1605),
                              Point(3690, 1875),
                              Point(3515, 1875),
                              Point(3515, 2615),
                              Point(3365, 2615),
                              Point(3365, 1875),
                              Point(3360, 1875)}));
  layout->AddPolygon(Polygon({Point(2880, 1245),
                              Point(3575, 1245),
                              Point(3575, 105),
                              Point(3725, 105),
                              Point(3725, 1395),
                              Point(3150, 1395),
                              Point(3150, 1575),
                              Point(3095, 1575),
                              Point(3095, 2615),
                              Point(2945, 2615),
                              Point(2945, 1575),
                              Point(2880, 1575)}));
  layout->AddPolygon(Polygon({Point(5355, 995),
                              Point(5415, 995),
                              Point(5415, 105),
                              Point(5565, 105),
                              Point(5565, 995),
                              Point(5625, 995),
                              Point(5625, 1325),
                              Point(5555, 1325),
                              Point(5555, 2615),
                              Point(5405, 2615),
                              Point(5405, 1325),
                              Point(5355, 1325)}));
  layout->AddPolygon(Polygon({Point(4435, 995),
                              Point(4995, 995),
                              Point(4995, 105),
                              Point(5145, 105),
                              Point(5145, 1325),
                              Point(5135, 1325),
                              Point(5135, 2615),
                              Point(4985, 2615),
                              Point(4985, 1325),
                              Point(4435, 1325)}));
  layout->AddPolygon(Polygon({Point(3045, 105),
                              Point(3195, 105),
                              Point(3195, 705),
                              Point(3365, 705),
                              Point(3365, 1035),
                              Point(3045, 1035)}));
  layout->AddPolygon(Polygon({Point(2470, 950),
                              Point(2540, 950),
                              Point(2540, 105),
                              Point(2690, 105),
                              Point(2690, 1100),
                              Point(2620, 1100),
                              Point(2620, 2615),
                              Point(2470, 2615),
                              Point(2470, 1545),
                              Point(2170, 1545),
                              Point(2170, 1275),
                              Point(2470, 1275)}));

  // nwell.drawing [DRAWING] 64/20
  layout->SetActiveLayerByName("nwell.drawing");
  //layout->AddRectangle(Rectangle(Point(0, 1305), Point(6190, 2910)));
  layout->AddRectangle(Rectangle(Point(x_min, 1305), Point(6000, 2910)));

  // npc.drawing [DRAWING] 95/20
  layout->SetActiveLayerByName("npc.drawing");
  layout->AddPolygon(Polygon({Point(x_min, 975),
                              Point(835, 975),
                              Point(835, 685),
                              Point(1545, 685),
                              Point(1545, 745),
                              Point(2230, 745),
                              Point(2230, 975),
                              Point(2960, 975),
                              Point(2960, 685),
                              Point(3415, 685),
                              Point(3415, 795),
                              Point(4315, 795),
                              Point(4315, 975),
                              Point(6000, 975),
                              Point(6000, 1345),
                              Point(4550, 1345),
                              Point(4550, 1925),
                              Point(3315, 1925),
                              Point(3315, 1595),
                              Point(1725, 1595),
                              Point(1725, 1925),
                              Point(735, 1925),
                              Point(735, 1685),
                              Point(20, 1685),
                              Point(20, 1420),
                              Point(x_min, 1420)}));

  // nsdm.drawing [DRAWING] 93/44
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle(Rectangle(Point(x_min, -190), Point(6000, 1015)));

  // hvtp.drawing [DRAWING] 78/44
  layout->SetActiveLayerByName("hvtp.drawing");
  layout->AddRectangle(Rectangle(Point(x_min, 1250), Point(6000, 2720)));

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  Rectangle *tiling_bounds = layout->AddRectangle(Rectangle(Point(x_min, 0), Point(6000, 2720)));
  layout->SetTilingBounds(*tiling_bounds);

  // psdm.drawing [DRAWING] 94/20
  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddPolygon(Polygon({Point(x_min, 1935),
                              Point(1880, 1935),
                              Point(1880, 1605),
                              Point(3305, 1605),
                              Point(3305, 1935),
                              Point(4570, 1935),
                              Point(4570, 1355),
                              Point(6000, 1355),
                              Point(6000, 2910),
                              Point(x_min, 2910)}));

  // li.pin [PIN] 67/16
  layout->SetActiveLayerByName("li.pin");
  Rectangle *d_port = layout->AddRectangle(
      Rectangle(Point(85, 1105), Point(255, 1275)));
  Rectangle *q_port = layout->AddRectangle(
      Rectangle(Point(5590, 425), Point(5760, 595)));

  layout->SetActiveLayerByName("li.drawing");
  layout->AddPort({*d_port, "D"});
  layout->AddPort({*q_port, "Q"});

  // nwell.pin [PIN] 64/16
  layout->SetActiveLayerByName("nwell.pin");

  // pwell.pin [PIN] 122/16
  layout->SetActiveLayerByName("pwell.pin");

  // met1.pin [PIN] 68/16
  layout->SetActiveLayerByName("met1.pin");

  layout->AddPort({
      Rectangle(Point(3375, 1445), Point(3545, 1615)), "CLKB"});
  layout->AddPort({
      Rectangle(Point(1370, 1785), Point(1540, 1955)), "CLK"});

  layout->Translate(Point(-x_min, 0));
  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
