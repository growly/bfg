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

}  // namespace
}  // namespace geometry
}  // namespace bfg
