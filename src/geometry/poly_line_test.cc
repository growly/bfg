#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glog/logging.h>

#include "line.h"
#include "point.h"
#include "poly_line.h"

namespace bfg {
namespace geometry {
namespace {

//         x
//  +--o--o
//        |x
//   x    o
//        +
TEST(PolyLineTest, IntersectsPoint) {
  PolyLine line = PolyLine({{0, 3}, {6, 3}, {6, 0}});
  size_t index = 0;
  EXPECT_TRUE(line.Intersects(Point(3, 3), &index));
  EXPECT_EQ(0, index);
  EXPECT_FALSE(line.Intersects(Point(1, 1), &index));
  EXPECT_EQ(-1, index);
  EXPECT_FALSE(line.Intersects(Point(7, 2), &index));
  EXPECT_TRUE(line.Intersects(Point(6, 1), &index));
  EXPECT_EQ(1, index);
  EXPECT_TRUE(line.Intersects(Point(6, 3), &index));
  EXPECT_EQ(0, index);
  EXPECT_FALSE(line.Intersects(Point(7, 4), &index));
}

//  +-----------+           +-----------+
//              |                       |
//              |                       |
//   length=l   |                       +
//   width=2    o     ->                |
//              |                       +
//              |                       |
//              |                       |
//              |                       |
//              +                       +
TEST(PolyLineTest, InsertBulge_MidSegment) {
  PolyLine line = PolyLine({{0, 9}, {12, 9}, {12, 0}});
  line.InsertBulge(Point(12, 5), 2, 2);
  std::vector<Point> expected = {
      {0, 9},
      {12, 9},
      {12, 6},
      {12, 4},
      {12, 0}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    0, 0, 2, 0
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  line.InsertBulge(Point(4, 9), 2, 2);
  expected = {
      {0, 9},
      {3, 9},
      {5, 9},
      {12, 9},
      {12, 6},
      {12, 4},
      {12, 0}
  };
  EXPECT_EQ(expected, line.Vertices());

  expected_widths = {
    0, 2, 0, 0, 2, 0
  };
  widths.clear();
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

// This test is not symmetric, the length of the bulge continue in the
// direction start -> finish when turning a corner:
//                                   +------+
//  +-----------o           +--------+--+   |
//              |                    +--|---+
//              |                       |
//   length=l   |                       |
//   width=2    |     ->                |
//              |                       |
//              |                       |
//              |                       |
//              |                       |
//              +                       +
//
TEST(PolyLineTest, InsertBulge_EndSegmentIsMidLine) {
  PolyLine line = PolyLine({{0, 9}, {12, 9}, {12, 0}});
  line.InsertBulge(Point(12, 9), 2, 6);
  std::vector<Point> expected = {
      {0, 9},
      {9, 9},
      {12, 9},
      {12, 8},
      {12, 0}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    0, 2, 1, 0
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  line = PolyLine({{12, 0}, {12, 9}, {0, 9}});
  line.InsertBulge(Point(12, 9), 2, 6);
  expected = {
      {12, 0},
      {12, 6},
      {12, 9},
      {11, 9},
      {0, 9}
  };
  EXPECT_EQ(expected, line.Vertices());

  expected_widths = {
    0, 2, 1, 0
  };
  widths.clear();
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

//  +-----------+           +-----------+
//              |                       |
//              |                       |
//   length=l   |                       |
//   width=2    |     ->                |
//              |                       |
//              |                       |
//              |                       |
//              |                       +
//              o                       |
//                                      +
//
TEST(PolyLineTest, InsertBulge_EndOfPolyLine) {
  PolyLine line = PolyLine({{0, 9}, {12, 9}, {12, 0}});
  line.InsertBulge(Point(12, 0), 2, 2);
  std::vector<Point> expected = {
      {0, 9},
      {12, 9},
      {12, 1},
      {12, -1}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    0, 0, 2
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  line = PolyLine({{12, 0}, {12, 9}, {0, 9}});
  line.InsertBulge(Point(0, 9), 2, 2);
  expected = {
      {12, 0},
      {12, 9},
      {1, 9},
      {-1, 9}
  };
  EXPECT_EQ(expected, line.Vertices());

  expected_widths = {
    0, 0, 2
  };
  widths.clear();
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

//  o-----------+          +-+----------+
//              |                       |
//              |                       |
//   length=l   |                       |
//   width=2    |     ->                |
//              |                       |
//              |                       |
//              |                       |
//              |                       |
//              +                       +
//
TEST(PolyLineTest, InsertBulge_StartOfPolyLine) {
  PolyLine line = PolyLine({{0, 9}, {12, 9}, {12, 0}});
  line.InsertBulge(Point(0, 9), 2, 2);
  LOG(INFO) << line.Describe();
  std::vector<Point> expected = {
      {-1, 9},
      {1, 9},
      {12, 9},
      {12, 0}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    2, 0, 0
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  line = PolyLine({{12, 0}, {12, 9}, {0, 9}});
  line.InsertBulge(Point(12, 0), 2, 2);
  LOG(INFO) << line.Describe();
  expected = {
      {12, -1},
      {12, 1},
      {12, 9},
      {0, 9}
  };
  EXPECT_EQ(expected, line.Vertices());

  expected_widths = {
    2, 0, 0
  };
  widths.clear();
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

TEST(PolyLineTest, BulgeDoesNotFitBeforeCorner) {
  Point p_0 = {125, 320};
  Point p_1 = {125, 480};
  Point p_2 = {-200, 480};
  PolyLine line = PolyLine({p_0, p_1, p_2});
  line.SetWidth(170);
  line.InsertBulge(p_0, 240, 330);
  LOG(INFO) << line.Describe();
  //line.InsertBulge(p_2, 240, 330);

  std::vector<Point> expected = {
      {125, 320 - (330 / 2)},
      {125, 480},
      {125 - (240 / 2), 480},
      {-200, 480}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    240, std::max(5, 170), 170
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
