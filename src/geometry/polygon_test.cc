#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "polygon.h"

namespace bfg {
namespace geometry {
namespace {

using testing::ElementsAre;

TEST(Polygon, IntersectingPoints_IntersectionAtStartCorner) {
  //
  //          (+)---+ 4, 2
  //     0, 0  |    |
  //     (+)---+    |
  //      |         |
  // x    +---------+ 4, -2
  // -2, -2
  //
  std::vector<Point> points = {
      {0, 0},
      {2, 0},
      {2, 2},
      {4, 2},
      {4, 0},
      {4, -2},
      {0, -2}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({-2, -2}, {0, 0});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  ASSERT_EQ(2, intersections.size());
  EXPECT_EQ(intersections[0].first, Point(0, 0));
  EXPECT_EQ(intersections[0].second, Point(0, 0));
  EXPECT_EQ(intersections[1].first, Point(2, 2));
  EXPECT_EQ(intersections[1].second, Point(2, 2));
}

TEST(Polygon, IntersectingPoints_SplitVertical_Intersection) {
  //
  //           +---(+)4, 4
  //           |    |
  //          (|)   |
  //     0, 0  |    |
  //     (+)---+    |
  //      |         |
  // x    +---------+ 4, -2
  // -2, -2
  //
  std::vector<Point> points = {
      {0, 0},
      {2, 0},
      {2, 6},
      {4, 6},
      {4, 4},
      {4, -2},
      {0, -2}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({-2, -2}, {0, 0});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{0, 0}, {0, 0}},
      {{2, 2}, {4, 4}}
  };

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_IntersectionAtInnerCorner) {
  //
  //           +---(+)4, 2
  //     0, 0  |    |
  //      +----+ <------ this corner is not included since the boundary is in
  //      |    2, 0 |    the polygon
  //     (+)--------+ 4, -2
  //       0, -2
  // x
  // -2, -4
  //
  std::vector<Point> points = {
      {0, 0},
      {2, 0},
      {2, 2},
      {4, 2},
      {4, 0},
      {4, -2},
      {0, -2}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({-2, -4}, {0, -2});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{0, -2}, {4, 2}}
  };

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_InterfaceAtAcuteCorner) {
  //
  // +--------(+) 2, 2
  // | 0, 0    |
  // +---(+)   |
  //      |    |
  // x    +----+
  // -2, -2
  //
  std::vector<Point> points = {
      {0, 0},
      {-2, 0},
      {-2, 2},
      {2, 2},
      {2, -2},
      {0, -2}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({-2, -2}, {0, 0});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{0, 0}, {2, 2}}
  };

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_VerticalLine) {
  std::vector<Point> points = {
      {12000, 1800},
      {13310, 1800},
      {13310, 1755},
      {13600, 1755},
      {13600, 1800},
      {14880, 1800},
      {14880, 1755},
      {15170, 1755},
      {15170, 1985},
      {14880, 1985},
      {14880, 1940},
      {13600, 1940},
      {13600, 1985},
      {13310, 1985},
      {13310, 1940},
      {12000, 1940}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({15170, 0}, {15170, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{15170, 1755}, {15170, 1985}}
  };

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_SimpleVerticalLine) {
  //
  //         2, 2
  //        (+)---+
  //         |    |
  //        (+)---+
  //
  //         x (2, -2)
  std::vector<Point> points = {
      {2, 2},
      {4, 2},
      {4, 0},
      {2, 0}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({2, -2}, {2, 0});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{2, 0}, {2, 2}}
  };

  EXPECT_EQ(expected, intersections);

  //
  //              4, 2
  //         +---(+)
  //         |    | 
  //         +---(+)
  //
  //              x (4, -2)
  line = Line({4, -2}, {4, 0});

  intersections.clear();
  polygon.IntersectingPoints(line, &intersections);

  expected = {
      {{4, 0}, {4, 2}}
  };

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_SimpleHorizontalLine) {
  //
  // 0, 2    2, 2
  // x      (+)--(+)
  //         |    |
  //         +----+
  std::vector<Point> points = {
      {2, 2},
      {4, 2},
      {4, 0},
      {2, 0}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({0, 2}, {2, 2});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{2, 2}, {4, 2}}
  };

  EXPECT_EQ(expected, intersections);

  line = Line({0, 0}, {2, 0});

  intersections.clear();
  polygon.IntersectingPoints(line, &intersections);

  expected = {
      {{2, 0}, {4, 0}}
  };

  EXPECT_EQ(expected, intersections);
}


// In this example the polygon is an 'H', and the intersecting line is incident
// on two vertical lines on top of each other in one of the arms.
//
//     ^
//     |
//     |
// +--(+)  +---+
// |   |   |   |
// |   +---+   |
// |           |
// |   +---+   |
// |   |   |   |
// +--(+)  +---+
//     |
//     |
//
TEST(Polygon, IntersectingPoints_YetAnotherVerticalLine) {
  std::vector<Point> points = {
      {30520, 3925},
      {30230, 3925},
      {30230, 3880},
      {28950, 3880},
      {28950, 3925},
      {28660, 3925},
      {28660, 3695},
      {28950, 3695},
      {28950, 3740},
      {30230, 3740},
      {30230, 3695},
      {30520, 3695}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({28950, 0}, {28950, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{28950, 3695}, {28950, 3925}}
  };
  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, IntersectingPoints_AnotherVerticalLine) {
  std::vector<Point> points = {
      {12000, 7240},
      {13310, 7240},
      {13310, 7195},
      {13600, 7195},
      {13600, 7240},
      {14880, 7240},
      {14880, 7195},
      {15170, 7195},
      {15170, 7425},
      {14880, 7425},
      {14880, 7380},
      {13600, 7380},
      {13600, 7425},
      {13310, 7425},
      {13310, 7380},
      {12000, 7380}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({15170, 0}, {15170, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{15170, 7195}, {15170, 7425}}
  };

  EXPECT_EQ(expected, intersections);
}

//
//
//
//
//  +--+
//     |
//     
TEST(Polygon, IntersectingPoints_TODO) {
  std::vector<Point> points = {
    {13990, 15},
    {13990, 215},
    {13945, 215},
    {13945, 505},
    {13990, 505},
    {13990, 4645},
    {13945, 4645},
    {13945, 4935},
    {13990, 4935},
    {13990, 5135},
    {14060, 5135},
    {14130, 5135},
    {14130, 4935},
    {14175, 4935},
    {14175, 4645},
    {14130, 4645},
    {14130, 505},
    {14175, 505},
    {14175, 215},
    {14130, 215},
    {14130, 15},
  };
  Polygon polygon(points);

  Line line = Line({32010, 0}, {32010, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{32010, 6185}, {32010, 6415}},
      {{32010, 6415}, {32010, 6585}}
  };
}


TEST(Polygon, IntersectingPoints_BogusPolygonStillWorks) {
  std::vector<Point> points = {
      {31970, 6185},
      {31970, 6408},
      {31970, 6415},
      {32055, 6415},
      {32040, 6415},
      {31750, 6415},
      {31750, 6500},
      {31750, 6585},
      {31750, 6585},
      {31895, 6585},
      {32140, 6585},
      {32140, 6408},
      {32140, 6185},
  };
  Polygon polygon(points);

  Line line = Line({32010, 0}, {32010, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);
  for (const auto &p : intersections)
    LOG(INFO) <<  p.first << " " << p.second;

  std::vector<std::pair<Point, Point>> expected = {
      {{32010, 6185}, {32010, 6415}},
      {{32010, 6415}, {32010, 6585}}
  };
}

// This polygon breaks a lot of assumptions. It should not crash the program,
// but it will have ill-defined behaviour.
//
//   8,9  10  |        11
//    +---+--(-)-------+
//    |                |
//   7+                |
//    |       3        |
//   6+---+---+---+4   |
//        5   |        |
//           2+        +12
//            |        |
//          1(+)-------+13
//            |
//            | x = 28270
//
// I think the answer we want is the that the line x = 28270 intersects the
// polygon between (28270, 5085) and (28270, 5485).
//
// Perhaps instead of further complicating the intersection routine, we should
// clean up polygons like this first.
TEST(Polygon, IntersectingPoints_BogusPolygonStillWorks2) {
  std::vector<Point> points = {
      {28270, 5085},
      {28270, 5308},
      {28270, 5315},
      {28355, 5315},
      {28245, 5315},
      {27955, 5315},
      {27955, 5400},
      {27955, 5485},
      {27955, 5485},
      {28100, 5485},
      {28440, 5485},
      {28440, 5308},
      {28440, 5085},
  };
  Polygon polygon(points);

  Line line = Line({28270, 0}, {28270, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);
  for (const auto &p : intersections)
    LOG(INFO) <<  p.first << " " << p.second;

  //std::vector<std::pair<Point, Point>> expected = {
  //    {{28270, 5085}, {28270, 5315}},
  //    {{28270, 5315}, {, 6585}}
  //};
}

//      x = 13990 
//      |   x = 14130
//      | 
//     (+)+-+      y = 5135
//      |   |
//  +---+   +---+  y = 4935
//  |           |
//  +---+   +---+  y = 4645
//      |   |
//      |   |
//  +---+   +---+  y = 505
//  |           |
//  +---+   +---+  y = 215
//      |   |
//     (s)--+      y = 15
//      |
//      |          s = start point
//      |
TEST(Polygon, IntersectingPoints_VerticalTwoPlus) {
  std::vector<Point> points = {
      {13990, 15},
      {13990, 215},
      {13945, 215},
      {13945, 505},
      {13990, 505},
      {13990, 4645},
      {13945, 4645},
      {13945, 4935},
      {13990, 4935},
      {13990, 5135},
      {14060, 5135},
      {14130, 5135},
      {14130, 4935},
      {14175, 4935},
      {14175, 4645},
      {14130, 4645},
      {14130, 505},
      {14175, 505},
      {14175, 215},
      {14130, 215},
      {14130, 15}
  };
  Polygon polygon(points);

  Line line = Line({13990, 0}, {13990, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);

  std::vector<std::pair<Point, Point>> expected = {
      {{13990, 15}, {13990, 5135}}
  };

  EXPECT_EQ(expected, intersections);

  line = Line({14130, 0}, {14130, 1});

  intersections.clear();
  polygon.IntersectingPoints(line, &intersections);

  expected = {
      {{14130, 15}, {14130, 5135}}
  };
  for (const auto &p : intersections)
    LOG(INFO) <<  p.first << " " << p.second;

  EXPECT_EQ(expected, intersections);
}

TEST(Polygon, OutsideVerticalEdgeSpanningEndAndStartPoints) {
  // This tickled two bugs:
  //  - double erasure of an iterator to the choices_copy vector; and
  //  - not setting 'check_for_dupes' correctly when considering a duplicate
  //  point (i.e. the intersection with the start point) and the same point in a
  //  span.
  //
  //            |
  //  y = 2225  +-----+     +-----+
  //            |     |     |     |
  //  y = 2180  |     +-----+     |
  //            |                 |
  //  y = 2040  |     +-----+     |
  //            |     |     |     |
  //  y = 1995 (s)----+     +-----+
  //            ^
  //            |
  //      x = 29150 29440 30720 31010
  //
  // Expect intersections at (29150, 1995), (29150, 2225).
 
  std::vector<Point> points = {
    {29150, 1995},
    {29440, 1995},
    {29440, 2040},
    {30720, 2040},
    {30720, 1995},
    {31010, 1995},
    {31010, 2225},
    {30720, 2225},
    {30720, 2180},
    {29440, 2180},
    {29440, 2225},
    {29150, 2225}
  };
  Polygon polygon(points);

  Line line = Line({29150, 0}, {29150, 1});

  std::vector<std::pair<Point, Point>> intersections;
  polygon.IntersectingPoints(line, &intersections);
  for (const auto &p : intersections)
    LOG(INFO) <<  p.first << " " << p.second;

  std::vector<std::pair<Point, Point>> expected = {
      {{29150, 1995}, {29150, 2225}}
      //{{29150, 2225}, {29150, 1995}}
  };

  EXPECT_EQ(expected, intersections);
}

//         +--+       +---+
//         |  |       |   |  <- polygon
//         +--+       |   |
//                    |   |
//  (0, 4) +----------+   |
//         |              |
//  (0, 0) +--------------+
TEST(Polygon, OverlapsRectangle_EntirelyInside1) {
  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  Rectangle bottom_right = Rectangle({7, 1}, {9, 3});

  EXPECT_TRUE(big_l_shape.Overlaps(bottom_right));
}

TEST(Polygon, OverlapsRectangle_InBoundingBoxButOutside) {
  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  Rectangle top_left = Rectangle({0, 6}, {4, 10});
  EXPECT_FALSE(big_l_shape.Overlaps(top_left));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
