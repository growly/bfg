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
  LOG(INFO) << line.Describe();
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
//   length=    |                       |
//   width=     |     ->                |
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
    0, 2, 6, 0
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
    0, 2, 6, 0
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

//                                  +------+
//  +-----------o           +-------+---+  |
//              |                   +---+--+
//              |                       |
//   length=    |                       |
//   width=     |     ->                |
//              |                       |
//              |                       |
//              |                       |
//              |                       |
//              +                       +
//
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

TEST(PolyLineTest, BulgeAfterCornerTurn) {
  Point p_0 = {2, 10};
  Point p_1 = {2, 2};
  Point p_2 = {10, 2};
  PolyLine line = PolyLine({p_0, p_1, p_2});
  line.SetWidth(4);
  line.InsertBulge({4, 2}, 6, 6);
  LOG(INFO) << line.Describe();
  //line.InsertBulge(p_2, 240, 330);

  std::vector<Point> expected = {
      {2, 10},
      {2, 5},
      {2, 2},
      {7, 2},
      {10, 2}
  };
  EXPECT_EQ(expected, line.Vertices());

  std::vector<uint64_t> expected_widths = {
    4,
    std::max(4, 1), // (6 / 2) - 2
    6,
    4
  };
  std::vector<uint64_t> widths;
  for (const auto &segment : line.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
} 

TEST(PolyLineTest, What) {
  PolyLine line = PolyLine({24530, 4810}, {
      LineSegment {{24530, 4885}, 140},
      LineSegment {{24665, 4885}, 140},
  });

  line.InsertBulge({24530, 4810}, 230, 290);
  line.InsertBulge({24665, 4885}, 230, 290);
}

TEST(PolyLineTest, What2) {
// <0| (1220, 0) |140| (1220, 2975) |240| (1220, 3305) |140| (1220, 6280) |0>
// point = (1220, 135) w x l 240 x 330
// <0| (1220, 105) |240| (1220, 300) |140| (1220, 2975) |240| (1220, 3305) |140| (1220, 6280) |0>
  PolyLine line = PolyLine({1220, 0}, {
      LineSegment {{1220, 2975}, 140},
      LineSegment {{1220, 3305}, 240},
      LineSegment {{1220, 6280}, 140}
  });

  line.InsertBulge({1220, 135}, 240, 330);
}

TEST(PolyLineTest, What3) {
  PolyLine line = PolyLine({585, 1465}, {
      LineSegment {{585, 1135}, 240},
      LineSegment {{585, 1075}, 170},
      LineSegment {{1615, 1075}, 170},
      LineSegment {{1615, 1105}, 170},
  });

  line.InsertBulge({1615, 1105}, 240, 330);

//<0| (585, 1465) |240| (585, 1135) |170| (585, 1075) |170| (1615, 1075) |170| (1615, 1105) |0>
//point = (1615, 1105) w x l 240 x 330
//<0| (585, 1465) |240| (585, 1135) |170| (585, 1075) |270| (1495, 1075) |170| (1615, 1075) |240| (1615, 1270) |0>

//  std::vector<Point> expected = {
//      {-3, 2},
//      {17, 2}
//  };
//  EXPECT_EQ(expected, line.Vertices());
//
//  std::vector<uint64_t> expected_widths = { 30 };
//  std::vector<uint64_t> widths;
//  for (const auto &segment : line.segments()) {
//    widths.push_back(segment.width);
//  }
//  EXPECT_EQ(expected_widths, widths);
//
}

TEST(PolyLineTest, What4) {
// <0| (540, 6135) |170| (540, 5965) |170| (700, 5965) |0>
// point = (540, 6135) w x l 230 x 290
// <0| (540, 6280) |230| (540, 5990) |170| (540, 5965) |170| (700, 5965) |0>
// <0| (540, 6280) |230| (540, 5990) |170| (540, 5965) |170| (700, 5965) |0>
// point = (700, 5965) w x l 170 x 330
// <0| (540, 6280) |230| (540, 5990) |170| (540, 6050) |170| (540, 5965) |170| (700, 5965) |170| (865, 5965) |0>
  PolyLine line = PolyLine({540, 6280}, {
      LineSegment {{540, 5990}, 230},
      LineSegment {{540, 5965}, 170},
      LineSegment {{700, 5965}, 170}
  });

  line.InsertBulge({700, 5965}, 170, 330);
}

// |0| (17830, 9690) |140| (17730, 9690) |140| (17730, 9570) |0|
// turns into
// |0| (17685, 9690) |230| (17845, 9690) |260| (17730, 9690) |230| (17730, 9425) |0|
 
//                       +------------+
//  +--o--+              |+---+       |
//  |                    ++---+-------+
//  o                 ->  |   |
//  |                     |   |
//  |                     +-+-+
//  |                       |
//  +                       +
//TEST(PolyLineTest, OverlappingBulgeAtStart) {
//  Point p_0 = {10, 10};
//  Point p_1 = {2, 10};
//  Point p_2 = {2, 2};
//  PolyLine line = PolyLine({p_0, p_1, p_2});
//  line.SetWidth(2);
//  LOG(INFO) << line.Describe();
//  line.InsertBulge({6, 10}, 6, 12);
// LOG(INFO) << line.Describe();
//  line.InsertBulge({2, 6}, 8, 12);
//  LOG(INFO) << line.Describe();
//  //line.InsertBulge(p_2, 240, 330);
//
//  std::vector<Point> expected = {
//      {12, 10},
//      {2, 10},
//      {2, 7},
//      {2, 2}
//  };
//  EXPECT_EQ(expected, line.Vertices());
//
//  std::vector<uint64_t> expected_widths = {
//    6,
//    4,
//    2
//  };
//  std::vector<uint64_t> widths;
//  for (const auto &segment : line.segments()) {
//    widths.push_back(segment.width);
//  }
//  EXPECT_EQ(expected_widths, widths);
//} 

// What happens when bulges at the end of a line are bigger than the connecting
// width?
//
//                                +------------+
//           +--o--+              |+---+       |
//           |                    ++---+-------+
//           o                     |   |
//           |                     |   |
//           |                     |   |
//           o                     |   |
//           |                    ++---+-------+
//           +--o--+              |+---+       |
//                                +------------+
//

//             (170)
// +---------------------------+
//                             |  (170)
//                             |
//                             + <- bulge added here
//
//
//                        +----------+
// +----------------------+----+     |
// |                      |    |     |
// +----------------------+----+     |
//                        |          |
//                        +----------+
TEST(PolyLineTest, BulgeAtEndLongerThanConnectingWidth) {
  Point p_0 = Point(-665, 1475);
  Point p_3 = Point(675, 1405);
  PolyLine line = PolyLine(p_0, {
      LineSegment {{675, 1475}, 170},
      LineSegment {p_3, 170},
  });

  LOG(INFO) << line.Describe();

  line.InsertBulge(p_3, 170, 330);

  LOG(INFO) << line.Describe();
}

// w =  0     4     8     0     4     8
//               +-----+           +-----+
//         +-----+     |     +-----+     |
//   +-----+     |     |+-+--+     |     |
//         +-----+     |     +-----+     |
//               +-----+           +-----+
// x = 2   4     6     8    10    12    14
//
TEST(PolyLineTest, BulgeOverlapsSameLineDifferentWidthSegments) {
  PolyLine line = PolyLine({2, 2}, {
      LineSegment {{4, 2}, 0},
      LineSegment {{6, 2}, 4},
      LineSegment {{8, 2}, 8},
      LineSegment {{10, 2}, 0},
      LineSegment {{12, 2}, 4},
      LineSegment {{14, 2}, 8},
  });

  PolyLine one = PolyLine(line);
  one.InsertBulge({7, 2}, 30, 20);

  std::vector<Point> expected = {
      {-3, 2},
      {17, 2}
  };
  EXPECT_EQ(expected, one.Vertices());

  std::vector<uint64_t> expected_widths = { 30 };
  std::vector<uint64_t> widths;
  for (const auto &segment : one.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  PolyLine two = PolyLine(line);
  LOG(INFO) << two.Describe();
  two.InsertBulge({7, 2}, 30, 10);
  LOG(INFO) << two.Describe();

  expected = {
    {2, 2},
    {12, 2},
    {14, 2}
  };
  EXPECT_EQ(expected, two.Vertices());

  expected_widths = { 30, 8 };
  widths.clear();
  for (const auto &segment : two.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  PolyLine three = PolyLine(line);
  LOG(INFO) << three.Describe();
  three.InsertBulge({7, 2}, 30, 6);
  LOG(INFO) << three.Describe();

  expected = {
    {2, 2},
    {4, 2},
    {10, 2},
    {12, 2},
    {14, 2}
  };
  EXPECT_EQ(expected, three.Vertices());

  expected_widths = { 0, 30, 4, 8 };
  widths.clear();
  for (const auto &segment : three.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  PolyLine four = PolyLine(line);
  LOG(INFO) << four.Describe();
  four.InsertBulge({11, 2}, 30, 6);
  LOG(INFO) << four.Describe();

  expected = {
    {2, 2},
    {4, 2},
    {6, 2},
    {8, 2},
    {14, 2}
  };
  EXPECT_EQ(expected, four.Vertices());

  expected_widths = { 0, 4, 8, 30 };
  widths.clear();
  for (const auto &segment : four.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);

  PolyLine five = PolyLine(line);
  LOG(INFO) << five.Describe();
  five.InsertBulge({8, 2}, 6, 20);
  LOG(INFO) << five.Describe();

  expected = {
    {-2, 2},
    {6, 2},
    {8, 2},
    {12, 2},
    {14, 2},
    {18, 2}
  };
  EXPECT_EQ(expected, five.Vertices());

  expected_widths = { 6, 8, 6, 8, 6 };
  widths.clear();
  for (const auto &segment : five.segments()) {
    widths.push_back(segment.width);
  }
  EXPECT_EQ(expected_widths, widths);
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
