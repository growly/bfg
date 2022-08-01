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
  cell->set_layout(GenerateLayout());
  cell->set_circuit(GenerateCircuit());

  return cell.release();
}

bfg::Circuit *Sky130Dfxtp::GenerateCircuit() {
  return nullptr;
}

bfg::Layout *Sky130Dfxtp::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(physical_db_));

  layout->AddRectangle(Rectangle(Point(0, 0), Point(7360, 2720)));

  // mcon.drawing [DRAWING] 67/44
  layout->SetActiveLayerByName("mcon.drawing");
  layout->AddRectangle(Rectangle(Point(2905, -85), Point(3075, 85)));
  layout->AddRectangle(Rectangle(Point(3825, -85), Point(3995, 85)));
  layout->AddRectangle(Rectangle(Point(3365, -85), Point(3535, 85)));
  layout->AddRectangle(Rectangle(Point(4285, 2635), Point(4455, 2805)));
  layout->AddRectangle(Rectangle(Point(4285, -85), Point(4455, 85)));
  layout->AddRectangle(Rectangle(Point(4300, 1785), Point(4470, 1955)));
  layout->AddRectangle(Rectangle(Point(4735, 1445), Point(4905, 1615)));
  layout->AddRectangle(Rectangle(Point(4745, 2635), Point(4915, 2805)));
  layout->AddRectangle(Rectangle(Point(4745, -85), Point(4915, 85)));
  layout->AddRectangle(Rectangle(Point(5205, 2635), Point(5375, 2805)));
  layout->AddRectangle(Rectangle(Point(5205, -85), Point(5375, 85)));
  layout->AddRectangle(Rectangle(Point(5665, 2635), Point(5835, 2805)));
  layout->AddRectangle(Rectangle(Point(5665, -85), Point(5835, 85)));
  layout->AddRectangle(Rectangle(Point(6125, 2635), Point(6295, 2805)));
  layout->AddRectangle(Rectangle(Point(6125, -85), Point(6295, 85)));
  layout->AddRectangle(Rectangle(Point(6585, 2635), Point(6755, 2805)));
  layout->AddRectangle(Rectangle(Point(6585, -85), Point(6755, 85)));
  layout->AddRectangle(Rectangle(Point(7045, 2635), Point(7215, 2805)));
  layout->AddRectangle(Rectangle(Point(7045, -85), Point(7215, 85)));
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));
  layout->AddRectangle(Rectangle(Point(605, 2635), Point(775, 2805)));
  layout->AddRectangle(Rectangle(Point(605, -85), Point(775, 85)));
  layout->AddRectangle(Rectangle(Point(630, 1785), Point(800, 1955)));
  layout->AddRectangle(Rectangle(Point(1025, 1445), Point(1195, 1615)));
  layout->AddRectangle(Rectangle(Point(1065, -85), Point(1235, 85)));
  layout->AddRectangle(Rectangle(Point(1525, 2635), Point(1695, 2805)));
  layout->AddRectangle(Rectangle(Point(1985, 2635), Point(2155, 2805)));
  layout->AddRectangle(Rectangle(Point(1985, -85), Point(2155, 85)));
  layout->AddRectangle(Rectangle(Point(2445, 2635), Point(2615, 2805)));
  layout->AddRectangle(Rectangle(Point(2730, 1785), Point(2900, 1955)));
  layout->AddRectangle(Rectangle(Point(1065, 2635), Point(1235, 2805)));
  layout->AddRectangle(Rectangle(Point(1525, -85), Point(1695, 85)));
  layout->AddRectangle(Rectangle(Point(2215, 1445), Point(2385, 1615)));
  layout->AddRectangle(Rectangle(Point(2445, -85), Point(2615, 85)));
  layout->AddRectangle(Rectangle(Point(2905, 2635), Point(3075, 2805)));
  layout->AddRectangle(Rectangle(Point(3825, 2635), Point(3995, 2805)));
  layout->AddRectangle(Rectangle(Point(3365, 2635), Point(3535, 2805)));

  // met1.drawing [DRAWING] 68/20
  layout->SetActiveLayerByName("met1.drawing");
  layout->AddRectangle(Rectangle(Point(0, 2480), Point(7360, 2960)));
  layout->AddRectangle(Rectangle(Point(0, -240), Point(7360, 240)));
  layout->AddPolygon(Polygon({Point(4530, 1985),
                              Point(4240, 1985),
                              Point(4240, 1940),
                              Point(2960, 1940),
                              Point(2960, 1985),
                              Point(2670, 1985),
                              Point(2670, 1940),
                              Point(860, 1940),
                              Point(860, 1985),
                              Point(570, 1985),
                              Point(570, 1755),
                              Point(860, 1755),
                              Point(860, 1800),
                              Point(2670, 1800),
                              Point(2670, 1755),
                              Point(2960, 1755),
                              Point(2960, 1800),
                              Point(4240, 1800),
                              Point(4240, 1755),
                              Point(4530, 1755)}));
  layout->AddPolygon(Polygon({Point(4965, 1645),
                              Point(4675, 1645),
                              Point(4675, 1600),
                              Point(2445, 1600),
                              Point(2445, 1645),
                              Point(2155, 1645),
                              Point(2155, 1600),
                              Point(1255, 1600),
                              Point(1255, 1645),
                              Point(965, 1645),
                              Point(965, 1415),
                              Point(1255, 1415),
                              Point(1255, 1460),
                              Point(2155, 1460),
                              Point(2155, 1415),
                              Point(2445, 1415),
                              Point(2445, 1460),
                              Point(4675, 1460),
                              Point(4675, 1415),
                              Point(4965, 1415)}));

  // diff.drawing [DRAWING] 65/20
  layout->SetActiveLayerByName("diff.drawing");
  layout->AddRectangle(Rectangle(Point(6075, 1485), Point(7175, 2485)));
  layout->AddRectangle(Rectangle(Point(135, 1815), Point(1225, 2455)));
  layout->AddRectangle(Rectangle(Point(135, 235), Point(1225, 655)));
  layout->AddRectangle(Rectangle(Point(6095, 235), Point(7185, 885)));
  layout->AddPolygon(Polygon({Point(5710, 2485),
                              Point(1495, 2485),
                              Point(1495, 2065),
                              Point(3395, 2065),
                              Point(3395, 1735),
                              Point(4230, 1735),
                              Point(4230, 2065),
                              Point(5710, 2065)}));
  layout->AddPolygon(Polygon({Point(5820, 655),
                              Point(5160, 655),
                              Point(5160, 595),
                              Point(4300, 595),
                              Point(4300, 875),
                              Point(3650, 875),
                              Point(3650, 655),
                              Point(3055, 655),
                              Point(3055, 595),
                              Point(2155, 595),
                              Point(2155, 655),
                              Point(1495, 655),
                              Point(1495, 235),
                              Point(5820, 235)}));

  // licon.drawing [DRAWING] 66/44
  layout->SetActiveLayerByName("licon.drawing");
  layout->AddRectangle(Rectangle(Point(160, 1075), Point(330, 1245)));
  layout->AddRectangle(Rectangle(Point(1960, 365), Point(2130, 535)));
  layout->AddRectangle(Rectangle(Point(2195, 1265), Point(2365, 1435)));
  layout->AddRectangle(Rectangle(Point(2295, 785), Point(2465, 955)));
  layout->AddRectangle(Rectangle(Point(2415, 2190), Point(2585, 2360)));
  layout->AddRectangle(Rectangle(Point(2515, 365), Point(2685, 535)));
  layout->AddRectangle(Rectangle(Point(2740, 1655), Point(2910, 1825)));
  layout->AddRectangle(Rectangle(Point(3290, 845), Point(3460, 1015)));
  layout->AddRectangle(Rectangle(Point(3510, 335), Point(3680, 505)));
  layout->AddRectangle(Rectangle(Point(3610, 2255), Point(3780, 2425)));
  layout->AddRectangle(Rectangle(Point(3610, 1915), Point(3780, 2085)));
  layout->AddRectangle(Rectangle(Point(3610, 1325), Point(3780, 1495)));
  layout->AddRectangle(Rectangle(Point(4030, 2215), Point(4200, 2385)));
  layout->AddRectangle(Rectangle(Point(6555, 445), Point(6725, 615)));
  layout->AddRectangle(Rectangle(Point(6765, 1075), Point(6935, 1245)));
  layout->AddRectangle(Rectangle(Point(6115, 1545), Point(6285, 1715)));
  layout->AddRectangle(Rectangle(Point(6965, 2225), Point(7135, 2395)));
  layout->AddRectangle(Rectangle(Point(6135, 640), Point(6305, 810)));
  layout->AddRectangle(Rectangle(Point(6135, 300), Point(6305, 470)));
  layout->AddRectangle(Rectangle(Point(6545, 2105), Point(6715, 2275)));
  layout->AddRectangle(Rectangle(Point(6965, 1885), Point(7135, 2055)));
  layout->AddRectangle(Rectangle(Point(6965, 1545), Point(7135, 1715)));
  layout->AddRectangle(Rectangle(Point(6975, 650), Point(7145, 820)));
  layout->AddRectangle(Rectangle(Point(4105, 365), Point(4275, 535)));
  layout->AddRectangle(Rectangle(Point(4290, 1325), Point(4460, 1495)));
  layout->AddRectangle(Rectangle(Point(4505, 2165), Point(4675, 2335)));
  layout->AddRectangle(Rectangle(Point(4505, 785), Point(4675, 955)));
  layout->AddRectangle(Rectangle(Point(4620, 365), Point(4790, 535)));
  layout->AddRectangle(Rectangle(Point(4800, 1655), Point(4970, 1825)));
  layout->AddRectangle(Rectangle(Point(5480, 1655), Point(5650, 1825)));
  layout->AddRectangle(Rectangle(Point(6975, 310), Point(7145, 480)));
  layout->AddRectangle(Rectangle(Point(5500, 2215), Point(5670, 2385)));
  layout->AddRectangle(Rectangle(Point(6115, 1900), Point(6285, 2070)));
  layout->AddRectangle(Rectangle(Point(175, 2215), Point(345, 2385)));
  layout->AddRectangle(Rectangle(Point(6545, 1705), Point(6715, 1875)));
  layout->AddRectangle(Rectangle(Point(1015, 425), Point(1185, 595)));
  layout->AddRectangle(Rectangle(Point(1535, 295), Point(1705, 465)));
  layout->AddRectangle(Rectangle(Point(595, 2135), Point(765, 2305)));
  layout->AddRectangle(Rectangle(Point(175, 425), Point(345, 595)));
  layout->AddRectangle(Rectangle(Point(5610, 365), Point(5780, 535)));
  layout->AddRectangle(Rectangle(Point(1480, 1415), Point(1650, 1585)));
  layout->AddRectangle(Rectangle(Point(670, 1150), Point(840, 1320)));
  layout->AddRectangle(Rectangle(Point(5845, 1075), Point(6015, 1245)));
  layout->AddRectangle(Rectangle(Point(1015, 2215), Point(1185, 2385)));
  layout->AddRectangle(Rectangle(Point(595, 295), Point(765, 465)));
  layout->AddRectangle(Rectangle(Point(1955, 2215), Point(2125, 2385)));
  layout->AddRectangle(Rectangle(Point(6115, 2255), Point(6285, 2425)));
  layout->AddRectangle(Rectangle(Point(1015, 1875), Point(1185, 2045)));
  layout->AddRectangle(Rectangle(Point(1535, 2255), Point(1705, 2425)));
  layout->AddRectangle(Rectangle(Point(175, 1875), Point(345, 2045)));

  // li.drawing [DRAWING] 67/20
  layout->SetActiveLayerByName("li.drawing");
  layout->AddRectangle(Rectangle(Point(1015, 345), Point(1200, 2465)));
  layout->AddRectangle(Rectangle(Point(1370, 715), Point(1650, 1665)));
  layout->AddRectangle(Rectangle(Point(90, 975), Point(440, 1625)));
  layout->AddRectangle(Rectangle(Point(2160, 1125), Point(2400, 1720)));
  layout->AddRectangle(Rectangle(Point(4290, 1245), Point(4480, 1965)));
  layout->AddPolygon(Polygon({Point(6015, 1325),
                              Point(5310, 1325),
                              Point(5310, 2335),
                              Point(4425, 2335),
                              Point(4425, 2165),
                              Point(5140, 2165),
                              Point(5140, 535),
                              Point(4525, 535),
                              Point(4525, 365),
                              Point(5310, 365),
                              Point(5310, 995),
                              Point(6015, 995)}));
  layout->AddPolygon(Polygon({Point(6935, 1325),
                              Point(6375, 1325),
                              Point(6375, 2465),
                              Point(6035, 2465),
                              Point(6035, 1905),
                              Point(5480, 1905),
                              Point(5480, 1530),
                              Point(6185, 1530),
                              Point(6185, 825),
                              Point(6055, 825),
                              Point(6055, 300),
                              Point(6385, 300),
                              Point(6385, 995),
                              Point(6935, 995)}));
  layout->AddPolygon(Polygon({Point(7275, 1575),
                              Point(7215, 1575),
                              Point(7215, 2420),
                              Point(6885, 2420),
                              Point(6885, 1495),
                              Point(7060, 1495),
                              Point(7060, 1445),
                              Point(7105, 1445),
                              Point(7105, 865),
                              Point(7050, 865),
                              Point(7050, 825),
                              Point(6895, 825),
                              Point(6895, 305),
                              Point(7225, 305),
                              Point(7225, 740),
                              Point(7275, 740)}));
  layout->AddPolygon(Polygon({Point(4970, 1995),
                              Point(4650, 1995),
                              Point(4650, 1035),
                              Point(4505, 1035),
                              Point(4505, 705),
                              Point(4970, 705)}));
  layout->AddPolygon(Polygon({Point(840, 1965),
                              Point(345, 1965),
                              Point(345, 2465),
                              Point(175, 2465),
                              Point(175, 1795),
                              Point(610, 1795),
                              Point(610, 805),
                              Point(175, 805),
                              Point(175, 345),
                              Point(345, 345),
                              Point(345, 635),
                              Point(840, 635)}));
  layout->AddPolygon(Polygon({Point(7360, 2805),
                              Point(0, 2805),
                              Point(0, 2635),
                              Point(515, 2635),
                              Point(515, 2135),
                              Point(845, 2135),
                              Point(845, 2635),
                              Point(1440, 2635),
                              Point(1440, 2175),
                              Point(1705, 2175),
                              Point(1705, 2635),
                              Point(3610, 2635),
                              Point(3610, 1835),
                              Point(3780, 1835),
                              Point(3780, 2635),
                              Point(5490, 2635),
                              Point(5490, 2135),
                              Point(5805, 2135),
                              Point(5805, 2635),
                              Point(6545, 2635),
                              Point(6545, 1625),
                              Point(6715, 1625),
                              Point(6715, 2635),
                              Point(7360, 2635)}));
  layout->AddPolygon(Polygon({Point(7360, 85),
                              Point(6725, 85),
                              Point(6725, 695),
                              Point(6555, 695),
                              Point(6555, 85),
                              Point(5795, 85),
                              Point(5795, 615),
                              Point(5585, 615),
                              Point(5585, 85),
                              Point(3770, 85),
                              Point(3770, 585),
                              Point(3400, 585),
                              Point(3400, 85),
                              Point(1705, 85),
                              Point(1705, 545),
                              Point(1455, 545),
                              Point(1455, 85),
                              Point(845, 85),
                              Point(845, 465),
                              Point(515, 465),
                              Point(515, 85),
                              Point(0, 85),
                              Point(0, -85),
                              Point(7360, -85)}));
  layout->AddPolygon(Polygon({Point(2210, 535),
                              Point(2045, 535),
                              Point(2045, 805),
                              Point(1990, 805),
                              Point(1990, 1910),
                              Point(2125, 1910),
                              Point(2125, 2465),
                              Point(1875, 2465),
                              Point(1875, 2040),
                              Point(1820, 2040),
                              Point(1820, 675),
                              Point(1875, 675),
                              Point(1875, 365),
                              Point(2210, 365)}));
  layout->AddPolygon(Polygon({Point(3100, 2020),
                              Point(2570, 2020),
                              Point(2570, 955),
                              Point(2215, 955),
                              Point(2215, 735),
                              Point(2740, 735),
                              Point(2740, 1655),
                              Point(3100, 1655)}));
  layout->AddPolygon(Polygon({Point(3780, 1575),
                              Point(3440, 1575),
                              Point(3440, 2360),
                              Point(2335, 2360),
                              Point(2335, 2190),
                              Point(3270, 2190),
                              Point(3270, 1485),
                              Point(2910, 1485),
                              Point(2910, 535),
                              Point(2405, 535),
                              Point(2405, 365),
                              Point(3080, 365),
                              Point(3080, 1315),
                              Point(3610, 1315),
                              Point(3610, 1245),
                              Point(3780, 1245)}));
  layout->AddPolygon(Polygon({Point(4355, 535),
                              Point(4120, 535),
                              Point(4120, 2135),
                              Point(4200, 2135),
                              Point(4200, 2465),
                              Point(3950, 2465),
                              Point(3950, 1065),
                              Point(3490, 1065),
                              Point(3490, 1095),
                              Point(3290, 1095),
                              Point(3290, 765),
                              Point(3950, 765),
                              Point(3950, 365),
                              Point(4355, 365)}));

  // poly.drawing [DRAWING] 66/20
  layout->SetActiveLayerByName("poly.drawing");
  layout->AddPolygon(Polygon({Point(3540, 1065),
                              Point(3320, 1065),
                              Point(3320, 2615),
                              Point(3170, 2615),
                              Point(3170, 795),
                              Point(3305, 795),
                              Point(3305, 105),
                              Point(3455, 105),
                              Point(3455, 795),
                              Point(3540, 795)}));
  layout->AddPolygon(Polygon({Point(2545, 1005),
                              Point(2215, 1005),
                              Point(2215, 735),
                              Point(2315, 735),
                              Point(2315, 105),
                              Point(2465, 105),
                              Point(2465, 735),
                              Point(2545, 735)}));
  layout->AddPolygon(Polygon({Point(1905, 2615),
                              Point(1755, 2615),
                              Point(1755, 1665),
                              Point(1430, 1665),
                              Point(1430, 1335),
                              Point(1755, 1335),
                              Point(1755, 105),
                              Point(1905, 105)}));
  layout->AddPolygon(Polygon({Point(2960, 1365),
                              Point(2445, 1365),
                              Point(2445, 1485),
                              Point(2330, 1485),
                              Point(2330, 2615),
                              Point(2180, 2615),
                              Point(2180, 1485),
                              Point(2115, 1485),
                              Point(2115, 1215),
                              Point(2810, 1215),
                              Point(2810, 105),
                              Point(2960, 105)}));
  layout->AddPolygon(Polygon({Point(2960, 1905),
                              Point(2805, 1905),
                              Point(2805, 2615),
                              Point(2655, 2615),
                              Point(2655, 1575),
                              Point(2960, 1575)}));
  layout->AddPolygon(Polygon({Point(5700, 1905),
                              Point(5445, 1905),
                              Point(5445, 2615),
                              Point(5295, 2615),
                              Point(5295, 1575),
                              Point(5410, 1575),
                              Point(5410, 105),
                              Point(5560, 105),
                              Point(5560, 1575),
                              Point(5700, 1575)}));
  layout->AddPolygon(Polygon({Point(5050, 1875),
                              Point(4875, 1875),
                              Point(4875, 2615),
                              Point(4725, 2615),
                              Point(4725, 1875),
                              Point(4720, 1875),
                              Point(4720, 1605),
                              Point(5050, 1605)}));
  layout->AddPolygon(Polygon({Point(965, 2585),
                              Point(815, 2585),
                              Point(815, 1370),
                              Point(590, 1370),
                              Point(590, 1100),
                              Point(815, 1100),
                              Point(815, 105),
                              Point(965, 105)}));
  layout->AddPolygon(Polygon({Point(5085, 1395),
                              Point(4510, 1395),
                              Point(4510, 1575),
                              Point(4455, 1575),
                              Point(4455, 2615),
                              Point(4305, 2615),
                              Point(4305, 1575),
                              Point(4240, 1575),
                              Point(4240, 1245),
                              Point(4935, 1245),
                              Point(4935, 105),
                              Point(5085, 105)}));
  layout->AddPolygon(Polygon({Point(545, 880),
                              Point(380, 880),
                              Point(380, 1590),
                              Point(545, 1590),
                              Point(545, 2585),
                              Point(395, 2585),
                              Point(395, 1740),
                              Point(230, 1740),
                              Point(230, 1325),
                              Point(110, 1325),
                              Point(110, 995),
                              Point(230, 995),
                              Point(230, 730),
                              Point(395, 730),
                              Point(395, 105),
                              Point(545, 105)}));
  layout->AddPolygon(Polygon({Point(6985, 1325),
                              Point(6915, 1325),
                              Point(6915, 2615),
                              Point(6765, 2615),
                              Point(6765, 1325),
                              Point(6715, 1325),
                              Point(6715, 995),
                              Point(6775, 995),
                              Point(6775, 105),
                              Point(6925, 105),
                              Point(6925, 995),
                              Point(6985, 995)}));
  layout->AddPolygon(Polygon({Point(6505, 1325),
                              Point(6495, 1325),
                              Point(6495, 2615),
                              Point(6345, 2615),
                              Point(6345, 1325),
                              Point(5795, 1325),
                              Point(5795, 995),
                              Point(6355, 995),
                              Point(6355, 105),
                              Point(6505, 105)}));
  layout->AddPolygon(Polygon({Point(4725, 1035),
                              Point(4405, 1035),
                              Point(4405, 105),
                              Point(4555, 105),
                              Point(4555, 705),
                              Point(4725, 705)}));
  layout->AddPolygon(Polygon({Point(4050, 1100),
                              Point(3980, 1100),
                              Point(3980, 2615),
                              Point(3830, 2615),
                              Point(3830, 1545),
                              Point(3530, 1545),
                              Point(3530, 1275),
                              Point(3830, 1275),
                              Point(3830, 950),
                              Point(3900, 950),
                              Point(3900, 105),
                              Point(4050, 105)}));

  // nwell.drawing [DRAWING] 64/20
  layout->SetActiveLayerByName("nwell.drawing");
  layout->AddRectangle(Rectangle(Point(-190, 1305), Point(7550, 2910)));

  // npc.drawing [DRAWING] 95/20
  layout->SetActiveLayerByName("npc.drawing");
  layout->AddPolygon(Polygon({Point(7360, 1345),
                              Point(5910, 1345),
                              Point(5910, 1925),
                              Point(4675, 1925),
                              Point(4675, 1595),
                              Point(3085, 1595),
                              Point(3085, 1925),
                              Point(2095, 1925),
                              Point(2095, 1685),
                              Point(1380, 1685),
                              Point(1380, 1420),
                              Point(565, 1420),
                              Point(565, 1345),
                              Point(0, 1345),
                              Point(0, 975),
                              Point(2195, 975),
                              Point(2195, 685),
                              Point(2905, 685),
                              Point(2905, 745),
                              Point(3590, 745),
                              Point(3590, 975),
                              Point(4320, 975),
                              Point(4320, 685),
                              Point(4775, 685),
                              Point(4775, 795),
                              Point(5675, 795),
                              Point(5675, 975),
                              Point(7360, 975)}));

  // nsdm.drawing [DRAWING] 93/44
  layout->SetActiveLayerByName("nsdm.drawing");
  layout->AddRectangle(Rectangle(Point(0, -190), Point(7360, 1015)));

  // hvtp.drawing [DRAWING] 78/44
  layout->SetActiveLayerByName("hvtp.drawing");
  layout->AddRectangle(Rectangle(Point(0, 1250), Point(7360, 2720)));

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  layout->AddRectangle(Rectangle(Point(0, 0), Point(7360, 2720)));

  // psdm.drawing [DRAWING] 94/20
  layout->SetActiveLayerByName("psdm.drawing");
  layout->AddPolygon(Polygon({Point(7360, 2910),
                              Point(0, 2910),
                              Point(0, 1685),
                              Point(1350, 1685),
                              Point(1350, 1935),
                              Point(3240, 1935),
                              Point(3240, 1605),
                              Point(4665, 1605),
                              Point(4665, 1935),
                              Point(5930, 1935),
                              Point(5930, 1355),
                              Point(7360, 1355)}));

  // li.pin [PIN] 67/16
  layout->SetActiveLayerByName("li.pin");
  layout->AddRectangle(Rectangle(Point(145, 1105), Point(315, 1275)));
  layout->AddRectangle(Rectangle(Point(1445, 1105), Point(1615, 1275)));
  layout->AddRectangle(Rectangle(Point(6950, 425), Point(7120, 595)));

  // nwell.pin [PIN] 64/16
  layout->SetActiveLayerByName("nwell.pin");
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));

  // pwell.pin [PIN] 122/16
  layout->SetActiveLayerByName("pwell.pin");
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));

  // met1.pin [PIN] 68/16
  layout->SetActiveLayerByName("met1.pin");
  layout->AddRectangle(Rectangle(Point(145, 2635), Point(315, 2805)));
  layout->AddRectangle(Rectangle(Point(145, -85), Point(315, 85)));

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
