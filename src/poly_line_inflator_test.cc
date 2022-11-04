#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glog/logging.h>

#include "geometry/line.h"
#include "geometry/line_segment.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "poly_line_inflator.h"

namespace bfg {
namespace {

using geometry::LineSegment;
using geometry::PolyLine;

TEST(PolyLineInflatorTest, SharpCorner) {
  PolyLine line = PolyLine({125, 155}, {
      LineSegment {{125, 485}, 240},
      LineSegment {{125, 480}, 170},
      LineSegment {{-35, 480}, 170},
      LineSegment {{-365, 480}, 249}
  });
  // TODO(aryap): Actually put a test here?
  EXPECT_TRUE(true);
}

}  // namespace
}  // namespace bfg
