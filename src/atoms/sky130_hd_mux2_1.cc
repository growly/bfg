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
  // A buffer is two back-to-back inverters:
  //
  //                       /            /
  //      +-----------+   _|        _  _|
  //      |           +--|_  p1   --S-|_  p3
  //      |    /           |            |             /
  //      |   _|          _|           _|            _|
  //      +-o|_ p0   A0 o|_  p2   A1 o|_  p4     +-o|_  p5
  //      |    | _         |            |        |    |
  // S ---+    +-S-+       +------------+--------+    +-- X
  //      |   _|   |      _|           _|        |   _|
  //      +--|_ n0 | A1 -|_  n2   A0 -|_  n4     +--|_  n5
  //           |   |       |            |             |
  //           V   |   _  _|           _|             V
  //               +---S-|_  n1   --S-|_  n3
  //                       |            |
  //                       V            V

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_hd_mux2_1": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Circuit *Sky130HdMux21::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  // TODO(aryap): Fill out.

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
  layout->AddRectangle(Rectangle(Point(150, 2125), Point(320, 2295)));
  layout->AddRectangle(Rectangle(Point(150, 1785), Point(320, 1955)));
  layout->AddRectangle(Rectangle(Point(150, 425), Point(320, 595)));
  layout->AddRectangle(Rectangle(Point(2450, 1105), Point(2620, 1275)));
  layout->AddRectangle(Rectangle(Point(2450, 765), Point(2620, 935)));
  layout->AddRectangle(Rectangle(Point(2910, 1445), Point(3080, 1615)));
  layout->AddRectangle(Rectangle(Point(1990, 1105), Point(2160, 1275)));
  layout->AddRectangle(Rectangle(Point(3370, 1445), Point(3540, 1615)));

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
