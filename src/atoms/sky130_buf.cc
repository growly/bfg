#include "sky130_buf.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../cell.h"
#include "../layout.h"

namespace bfg {

namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;


bfg::Cell *Sky130Buf::Generate() {
  // A buffer is two back-to-back inverters:
  //
  //          /         /
  //         _|        _|
  //      +o|_ X1   +o|_  X3
  //      |   |     |   |
  //   ---+   +-----+   +---
  //      |  _|     |  _|
  //      +-|_ X0   +-|_  X2
  //          |         |
  //          V         V

  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_buf"));
  cell->set_layout(GenerateLayout());

  // TODO(growly): std::move?
  return cell.release();
}

bfg::Circuit *Sky130Buf::GenerateCircuit() {
  return nullptr;
}

bfg::Layout *Sky130Buf::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout());

  // areaid.standardc 81/4
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  layout->AddRectangle(Rectangle(Point(0, 0),
                                 parameters_.width,
                                 parameters_.height));

  // met1.drawing 68/20
  // The second "metal" layer.
  layout->AddRectangle(Rectangle(Point(0, -240),
                                 parameters_.width,
                                 480));

  layout->AddRectangle(Rectangle(Point(0, parameters_.height - 240),
                                 parameters_.width,
                                 480));

  // li1.drawing 67/20
  // The first "metal" layer.
  layout->AddPolygon(Polygon({Point(0, -85),
                              Point(0, 85),
                              Point(525, 85),
                              Point(525, 465),
                              Point(855, 465),
                              Point(855, 85),
                              Point(parameters_.width, 85),
                              Point(parameters_.width, -85) }));

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

  // npc.drawing 95/20
  // What is this?

  // licon1.drawing 66/44
  // Contacts from li1 layer to diffusion.

  // hvtp.drawing 78/44

  // poly.drawing 66/20
  // Polysilicon, more generally gate material.
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
  // psdm.drawing 94/20

  // diff.drawing 65/20
  // Diffusion. Intersection with gate material layer defines gate size.
  
  

  // nwell.pin 64/16
  // nwell.drawing 64/20
  // pwell.pin 122/16

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
