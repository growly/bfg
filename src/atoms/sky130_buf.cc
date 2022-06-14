#include "sky130_buf.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../cell.h"

namespace bfg {

namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;

bfg::Cell *Sky130Buf::Generate() {
  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_buf"));

  // areaid.standardc 81/4
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  cell->AddRectangle(Rectangle(Point(0, 0),
                     parameters_.width,
                     parameters_.height));

  // met1.drawing 68/20
  // The second "metal" layer.
  cell->AddRectangle(Rectangle(Point(0, -240),
                     parameters_.width,
                     480));

  cell->AddRectangle(Rectangle(Point(0, parameters_.height - 240),
                     parameters_.width,
                     480));

  // li1.drawing 67/20
  // The first "metal" layer.
  cell->AddPolygon(Polygon({ Point(0, -85),
                             Point(0, 85),
                             Point(525, 85),
                             Point(525, 465),
                             Point(855, 465),
                             Point(855, 85),
                             Point(parameters_.width, 85),
                             Point(parameters_.width, -85) }));

  cell->AddPolygon(Polygon({ Point(175, 255),     // metal width 170 or 180
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

  cell->AddRectangle(Rectangle(Point(105, 985), Point(445, 1355)));

  cell->AddPolygon(Polygon({ Point(1035, 255),
                             Point(1035, 760),
                             Point(1115, 760),
                             Point(1115, 1560),
                             Point(1025, 1560),
                             Point(1025, 2465),
                             Point(1295, 2465),
                             Point(1295, 255)}));

  cell->AddPolygon(Polygon({ Point(525, 1875),
                             Point(525, 2635),
                             Point(0, 2635),
                             Point(0, 2805),
                             Point(1380, 2805),
                             Point(1380, 2635),
                             Point(855, 2635),
                             Point(855, 1875)}));

  return cell.release();
}

}  // namespace atoms
}  // namespace bfg
