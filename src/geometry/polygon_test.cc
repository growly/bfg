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
  //      +---(+)   |
  //      |    2, 0 |
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
      {{0, -2}, {2, 0}},
      {{2, 0}, {4, 2}}
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

  std::vector<std::pair<Point, Point>> expected = {
      {{32010, 6185}, {32010, 6415}},
      {{32010, 6415}, {32010, 6585}}
  };
}

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

}  // namespace
}  // namespace geometry
}  // namespace bfg
