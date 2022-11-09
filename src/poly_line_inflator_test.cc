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

}  // namespace
}  // namespace bfg
