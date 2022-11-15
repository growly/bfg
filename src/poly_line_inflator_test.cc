#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glog/logging.h>

#include "geometry/line.h"
#include "geometry/line_segment.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/poly_line.h"
#include "poly_line_inflator.h"
#include "physical_properties_database.h"

namespace bfg {
namespace {

using geometry::LineSegment;
using geometry::PolyLine;

TEST(PolyLineInflatorTest, SharpCorner) {
  //
  //          x (1)
  //
  //          x (2)
  //
  // x (4)    x (3)  x (5)
  //
  PolyLine line = PolyLine({24530, 4615}, {
      LineSegment {{24530, 4325}, 230},
      LineSegment {{24530, 4304}, 140},
      LineSegment {{24520, 4304}, 140},
      LineSegment {{24810, 4304}, 230}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  geometry::Polygon inflated;
  inflator.InflatePolyLine(line, &inflated);

  for (const auto &point : inflated.vertices()) {
    LOG(INFO) << point;
  }

  EXPECT_TRUE(true);
}

// inflating: <0| (31510, 10785) |230| (31510, 10810) |230| (31510, 11075) |230| (31532, 11086) |0>
//  into: (31395, 10785) (31395, 10870) (31395, 11145) (31481, 11188) (31532, 11086) (31583, 10984) (31625, 11005) (31625, 10870) (31625, 10785) 
//
//
// <0| (18170, 6630) |140| (18070, 6630) |140| (18070, 6705) |230| (18070, 6995) |0>
// point = (18170, 6630) w x l 230 x 290
//            <0| (18315, 6630) |230| (18070, 6630) |140| (18070, 6745) |140| (18070, 6705) |230| (18070, 6995) |0>
// inflating: <0| (18315, 6630) |230| (18070, 6630) |140| (18070, 6745) |140| (18070, 6705) |230| (18070, 6995) |0>
//  into: (18315, 6515) (18000, 6515) (18000, 6745) (18140, 6745) (18140, 6705) (17955, 6705) (17955, 6995) (18070, 6995) (18185, 6995) (18185, 6705) (18000, 6705) (18000, 6745) (18140, 6745) (18140, 6745) (18315, 6745)


}  // namespace
}  // namespace bfg
