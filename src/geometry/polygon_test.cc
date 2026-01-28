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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  intersections = polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

  std::vector<std::pair<Point, Point>> expected = {
      {{2, 2}, {4, 2}}
  };

  EXPECT_EQ(expected, intersections);

  line = Line({0, 0}, {2, 0});

  intersections = polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

  std::vector<std::pair<Point, Point>> expected = {
      {{28950, 3695}, {28950, 3925}}
  };
  EXPECT_EQ(expected, intersections);
}

// A similar test, but horizontally:
//
//        +----+          +----+
//        |    +----------+    |
//        |                    |
//  >----(|)   +----------+   (|)-------->
//        +----+          +----+
//
// Again since the boundary of the polygon is considered to be interior,
// this line should 'enter' the polygon on the left and 'exit' on the right,
// yielding only one pair of intersecting points.
TEST(Polygon, IntersectingPoints_HorizontalLineThroughBoundary) {
  std::vector<Point> points = {
    {8090, 25785},
    {7800, 25785},
    {7800, 25740},
    {6520, 25740},
    {6520, 25785},
    {6230, 25785},
    {6230, 25555},
    {6520, 25555},
    {6520, 25600},
    {7800, 25600},
    {7800, 25555},
    {8090, 25555}
  };
  Polygon polygon = Polygon(points);

  Line line = Line({0, 25600}, {1, 25600});

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

  std::vector<std::pair<Point, Point>> expected = {
      {{6230, 25600}, {8090, 25600}}
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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);
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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);
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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);

  std::vector<std::pair<Point, Point>> expected = {
      {{13990, 15}, {13990, 5135}}
  };

  EXPECT_EQ(expected, intersections);

  line = Line({14130, 0}, {14130, 1});

  intersections = polygon.IntersectingPoints(line);

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

  std::vector<std::pair<Point, Point>> intersections =
      polygon.IntersectingPoints(line);
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

TEST(Polygon, Equal) {
  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });
  Polygon other = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  EXPECT_TRUE(big_l_shape == other);

  big_l_shape.set_layer(0);
  other.set_layer(2);
  EXPECT_FALSE(big_l_shape == other);

  Polygon broken_l = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4}
  });
  EXPECT_FALSE(broken_l == big_l_shape);
}

TEST(Polygon, EqualUnderRotation) {
  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });
  Polygon big_l_rotated = Polygon({
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4},
      {0, 0},
      {10, 0}
  });
  EXPECT_TRUE(big_l_shape == big_l_rotated);

  Polygon another_big_l_rotated = Polygon({
      {6, 4},
      {0, 4},
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10}
  });
  EXPECT_TRUE(big_l_shape == another_big_l_rotated);

  big_l_shape.set_layer(0);
  big_l_rotated.set_layer(1);
  EXPECT_FALSE(big_l_shape == big_l_rotated);
}

TEST(Polygon, Edges) {
  Polygon square = Polygon({
      {0, 0},
      {0, 2},
      {2, 2},
      {2, 0}
  });

  std::vector<Line> edges = square.Edges();

  EXPECT_EQ(edges[0].start(), Point(0, 0));
  EXPECT_EQ(edges[0].end(), Point(0, 2));
  EXPECT_EQ(edges[1].start(), Point(0, 2));
  EXPECT_EQ(edges[1].end(), Point(2, 2));
  EXPECT_EQ(edges[2].start(), Point(2, 2));
  EXPECT_EQ(edges[2].end(), Point(2, 0));
  EXPECT_EQ(edges[3].start(), Point(2, 0));
  EXPECT_EQ(edges[3].end(), Point(0, 0));
}

TEST(Polygon, Fattened) {
  Polygon square = Polygon({
      {0, 0},
      {0, 2},
      {2, 2},
      {2, 0}
  });

  square.Fatten(3);

  std::vector<Point> expected = {
      {-3, -3},
      {-3, 5},
      {5, 5},
      {5, -3}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(Polygon, PolygonOverlapsPolygon_NoOverlap_NotEvenClose) {
  Polygon square = Polygon({
      {20, 20},
      {20, 22},
      {22, 22},
      {22, 20}
  });

  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  EXPECT_FALSE(big_l_shape.Overlaps(square));
  EXPECT_FALSE(square.Overlaps(big_l_shape));
}

TEST(Polygon, PolygonOverlapsPolygon_NoOverlap_WithinBoundingBox) {
  Polygon square = Polygon({
      {0, 5},
      {0, 7},
      {2, 5},
      {2, 7}
  });

  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  EXPECT_FALSE(big_l_shape.Overlaps(square));
  EXPECT_FALSE(square.Overlaps(big_l_shape));
}

TEST(Polygon, PolygonOverlapsPolygon_Overlap_EntirelyContained) {
  Polygon square = Polygon({
      {1, 1},
      {1, 3},
      {3, 3},
      {3, 1}
  });

  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });

  // TODO(aryap): Implement these and uncomment.
  //EXPECT_TRUE(big_l_shape.Overlaps(square));
  //EXPECT_TRUE(square.Overlaps(big_l_shape));
}

TEST(Polygon, PolygonOverlapsPolygon_Overlap_EdgeIntersections) {
  Polygon square = Polygon({
      {-1, -1},
      {-1, 5},
      {5, 5},
      {5, -1}
  });

  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });
  EXPECT_TRUE(big_l_shape.Overlaps(square));
  EXPECT_TRUE(square.Overlaps(big_l_shape));
}

TEST(Polygon, PolygonOverlapsPolygon_Overlap_EdgeIncidence) {
  Polygon square = Polygon({
      {6, 6},
      {6, 10},
      {10, 10},
      {10, 6}
  });

  Polygon big_l_shape = Polygon({
      {0, 0},
      {10, 0},
      {10, 10},
      {6, 10},
      {6, 4},
      {0, 4}
  });
  EXPECT_TRUE(big_l_shape.Overlaps(square));
  EXPECT_TRUE(square.Overlaps(big_l_shape));
}

TEST(PolygonTest, MirrorY_Square) {
  Polygon square({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  square.MirrorY();
  // MirrorY negates x coordinates of all vertices
  std::vector<Point> expected = {
      {0, 0},
      {-10, 0},
      {-10, 10},
      {0, 10}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, MirrorY_OffsetPolygon) {
  Polygon poly({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  poly.MirrorY();
  std::vector<Point> expected = {
      {-5, 10},
      {-15, 10},
      {-15, 30},
      {-5, 30}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, MirrorY_TwiceReturnsToOriginal) {
  Polygon original({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Polygon poly = original;
  poly.MirrorY();
  poly.MirrorY();
  EXPECT_TRUE(original == poly);
}

TEST(PolygonTest, MirrorX_Square) {
  Polygon square({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  square.MirrorX();
  // MirrorX negates y coordinates of all vertices
  std::vector<Point> expected = {
      {0, 0},
      {10, 0},
      {10, -10},
      {0, -10}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, MirrorX_OffsetPolygon) {
  Polygon poly({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  poly.MirrorX();
  std::vector<Point> expected = {
      {5, -10},
      {15, -10},
      {15, -30},
      {5, -30}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, MirrorX_TwiceReturnsToOriginal) {
  Polygon original({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Polygon poly = original;
  poly.MirrorX();
  poly.MirrorX();
  EXPECT_TRUE(original == poly);
}

TEST(PolygonTest, Translate_ZeroOffset) {
  Polygon poly({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  poly.Translate(Point(0, 0));
  std::vector<Point> expected = {
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Translate_PositiveOffset) {
  Polygon poly({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  poly.Translate(Point(100, 200));
  std::vector<Point> expected = {
      {100, 200},
      {110, 200},
      {110, 210},
      {100, 210}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Translate_NegativeOffset) {
  Polygon poly({
      {50, 60},
      {70, 60},
      {70, 80},
      {50, 80}
  });
  poly.Translate(Point(-30, -40));
  std::vector<Point> expected = {
      {20, 20},
      {40, 20},
      {40, 40},
      {20, 40}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Translate_PreservesBoundingBoxDimensions) {
  Polygon poly({
      {0, 0},
      {100, 0},
      {100, 200},
      {0, 200}
  });
  Rectangle original_bb = poly.GetBoundingBox();
  poly.Translate(Point(1000, 2000));
  Rectangle new_bb = poly.GetBoundingBox();
  EXPECT_EQ(original_bb.Width(), new_bb.Width());
  EXPECT_EQ(original_bb.Height(), new_bb.Height());
}

TEST(PolygonTest, FlipHorizontal_Square) {
  // FlipHorizontal mirrors Y then moves back to original lower_left
  Polygon poly({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Rectangle original_bb = poly.GetBoundingBox();
  poly.FlipHorizontal();
  Rectangle new_bb = poly.GetBoundingBox();
  // The bounding box lower_left should be preserved
  EXPECT_EQ(original_bb.lower_left(), new_bb.lower_left());
  EXPECT_EQ(original_bb.Width(), new_bb.Width());
  EXPECT_EQ(original_bb.Height(), new_bb.Height());
}

TEST(PolygonTest, FlipHorizontal_LShape) {
  // L-shape should flip horizontally
  Polygon l_shape({
      {0, 0},
      {10, 0},
      {10, 5},
      {5, 5},
      {5, 10},
      {0, 10}
  });
  l_shape.FlipHorizontal();
  // After FlipHorizontal, the L should be mirrored
  std::vector<Point> expected = {
      {0, 0},
      {-10, 0},
      {-10, 5},
      {-5, 5},
      {-5, 10},
      {0, 10}
  };
  // Translate expected to match new lower_left
  // Original lower_left was (0,0), after MirrorY it would be (-10,0)
  // So it gets translated by (10,0) to put lower_left back at (0,0)
  expected = {
      {10, 0},
      {0, 0},
      {0, 5},
      {5, 5},
      {5, 10},
      {10, 10}
  };
  EXPECT_THAT(l_shape.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, FlipVertical_Square) {
  // FlipVertical mirrors X then moves back to original lower_left
  Polygon poly({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Rectangle original_bb = poly.GetBoundingBox();
  poly.FlipVertical();
  Rectangle new_bb = poly.GetBoundingBox();
  // The bounding box lower_left should be preserved
  EXPECT_EQ(original_bb.lower_left(), new_bb.lower_left());
  EXPECT_EQ(original_bb.Width(), new_bb.Width());
  EXPECT_EQ(original_bb.Height(), new_bb.Height());
}

TEST(PolygonTest, FlipVertical_LShape) {
  Polygon l_shape({
      {0, 0},
      {10, 0},
      {10, 5},
      {5, 5},
      {5, 10},
      {0, 10}
  });
  l_shape.FlipVertical();
  // After FlipVertical (MirrorX then restore lower_left)
  std::vector<Point> expected = {
      {0, 10},
      {10, 10},
      {10, 5},
      {5, 5},
      {5, 0},
      {0, 0}
  };
  EXPECT_THAT(l_shape.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Rotate_Square90Degrees) {
  // Square at origin rotated 90 degrees CCW
  Polygon square({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  square.Rotate(90);
  std::vector<Point> expected = {
      {0, 0},
      {0, 10},
      {-10, 10},
      {-10, 0}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Rotate_Square180Degrees) {
  Polygon square({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  square.Rotate(180);
  std::vector<Point> expected = {
      {0, 0},
      {-10, 0},
      {-10, -10},
      {0, -10}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Rotate_Square270Degrees) {
  Polygon square({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  square.Rotate(270);
  std::vector<Point> expected = {
      {0, 0},
      {0, -10},
      {10, -10},
      {10, 0}
  };
  EXPECT_THAT(square.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Rotate_360DegreesReturnsToOriginal) {
  Polygon original({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Polygon poly = original;
  poly.Rotate(360);
  EXPECT_TRUE(original == poly);
}

TEST(PolygonTest, Rotate_FourRotationsOf90ReturnsToOriginal) {
  Polygon original({
      {1, 2},
      {5, 2},
      {5, 10},
      {1, 10}
  });
  Polygon poly = original;
  poly.Rotate(90);
  poly.Rotate(90);
  poly.Rotate(90);
  poly.Rotate(90);
  EXPECT_TRUE(original == poly);
}

TEST(PolygonTest, Rotate_LShape90Degrees) {
  Polygon l_shape({
      {0, 0},
      {10, 0},
      {10, 5},
      {5, 5},
      {5, 10},
      {0, 10}
  });
  l_shape.Rotate(90);
  std::vector<Point> expected = {
      {0, 0},
      {0, 10},
      {-5, 10},
      {-5, 5},
      {-10, 5},
      {-10, 0}
  };
  EXPECT_THAT(l_shape.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, Rotate_NegativeDegrees) {
  // -90 degrees should be equivalent to 270 degrees CCW
  Polygon poly1({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  Polygon poly2({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  poly1.Rotate(-90);
  poly2.Rotate(270);
  EXPECT_TRUE(poly1 == poly2);
}

TEST(PolygonTest, CombinedOperations_MirrorYThenTranslate) {
  Polygon poly({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  poly.MirrorY();
  poly.Translate(Point(20, 0));
  // After MirrorY: (-5,10), (-15,10), (-15,30), (-5,30)
  // After Translate: (15,10), (5,10), (5,30), (15,30)
  std::vector<Point> expected = {
      {15, 10},
      {5, 10},
      {5, 30},
      {15, 30}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

TEST(PolygonTest, CombinedOperations_MirrorXThenMirrorY) {
  // MirrorX then MirrorY is equivalent to 180 degree rotation
  Polygon poly1({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  Polygon poly2({
      {5, 10},
      {15, 10},
      {15, 30},
      {5, 30}
  });
  poly1.MirrorX();
  poly1.MirrorY();
  poly2.Rotate(180);
  EXPECT_TRUE(poly1 == poly2);
}

TEST(PolygonTest, CombinedOperations_RotateThenTranslate) {
  Polygon poly({
      {0, 0},
      {10, 0},
      {10, 10},
      {0, 10}
  });
  poly.Rotate(90);
  poly.Translate(Point(20, 0));
  std::vector<Point> expected = {
      {20, 0},
      {20, 10},
      {10, 10},
      {10, 0}
  };
  EXPECT_THAT(poly.vertices(), testing::ContainerEq(expected));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
