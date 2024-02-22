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

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  EXPECT_TRUE(inflated.has_value());

  for (const auto &point : inflated->vertices()) {
    LOG(INFO) << point;
  }
}

TEST(PolyLineInflatorTest, OverhangCutOff) {
  PolyLine line = PolyLine({-665, 1713}, {
      LineSegment {{-665, 1475}, 170},
      LineSegment {{-580, 1475}, 184},
      LineSegment {{590, 1475}, 170},
      LineSegment {{675, 1475}, 170},
      LineSegment {{675, 1405}, 170},
      LineSegment {{675, 1240}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  EXPECT_TRUE(inflated.has_value());
}

TEST(PolyLineInflatorTest, AnotherSharpCorner1) {
  PolyLine line = PolyLine(
      {60, 1135}, {
      LineSegment {{335, 1135}, 170},
      LineSegment {{335, 1050}, 170},
      LineSegment {{335, 905}, 170},
      LineSegment {{1015, 905}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  std::vector<geometry::Point> expected_vertices = {
    {60, 1220},
    {420, 1220},
    {420, 990},
    {1015, 990},
    {1015, 820},
    {250, 820},
    {250, 1050},
    {60, 1050}
  };

  EXPECT_TRUE(inflated.has_value());
  EXPECT_EQ(expected_vertices, inflated->vertices());
}

TEST(PolyLineInflatorTest, AnotherSharpCorner2) {
  PolyLine line = PolyLine(
      {335, 905}, {
      LineSegment {{1015, 905}, 170},
      LineSegment {{1015, 1135}, 170},
      LineSegment {{1155, 1135}, 170},
      LineSegment {{1485, 1135}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  std::vector<geometry::Point> expected_vertices = {
    {335, 990},
    {930, 990},
    {930, 1220},
    {1485, 1220},
    {1485, 1050},
    {1100, 1050},
    {1100, 820},
    {335, 820}
  };

  EXPECT_TRUE(inflated.has_value());
  EXPECT_EQ(expected_vertices, inflated->vertices());
}

TEST(PolyLineInflatorTest, TwoSharpCornersOnTheSameLine) {
  PolyLine line = PolyLine(
      {60, 1135}, {
      LineSegment {{335, 1135}, 170},
      LineSegment {{335, 1050}, 170},
      LineSegment {{335, 905}, 170},
      LineSegment {{1015, 905}, 170},
      LineSegment {{1015, 1135}, 170},
      LineSegment {{1155, 1135}, 170},
      LineSegment {{1485, 1135}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  std::vector<geometry::Point> expected_vertices = {
    {60, 1220},
    {420, 1220},
    {420, 990},
    {930, 990},
    {930, 1220},
    {1485, 1220},
    {1485, 1050},
    {1100, 1050},
    {1100, 820},
    {250, 820},
    {250, 1050},
    {60, 1050}
  };

  EXPECT_TRUE(inflated.has_value());
  for (const auto &point : inflated->vertices()) {
    LOG(INFO) << point;
  }
  EXPECT_EQ(expected_vertices, inflated->vertices());
}

TEST(PolyLineInflatorTest, TODO_ThisShouldNotBreak) {
  // before bulges:
  //  <0| (225, 1055) |170| (335, 1055) |170| (335, 985) |170| (1015, 985) |170| (1015, 1055) |170| (1320, 1055) |0>
  // after bulges:
  //  <0| (60, 1055) |170| (335, 1055) |170| (335, 970) |170| (335, 985) |170| (1015, 985) |170| (1015, 1055) |170| (1155, 1055) |170| (1485, 1055) |0>
  PolyLine line = PolyLine(
      {60, 1055}, {
      LineSegment {{335, 1055}, 170},
      LineSegment {{335, 970}, 170},
      LineSegment {{335, 985}, 170},
      LineSegment {{1015, 985}, 170},
      LineSegment {{1015, 1055}, 170},
      LineSegment {{1155, 1055}, 170},
      LineSegment {{1485, 1055}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  std::vector<geometry::Point> expected_vertices = {
    {60, 1220},
    {420, 1220},
    {420, 990},
    {930, 990},
    {930, 1220},
    {1485, 1220},
    {1485, 1050},
    {1100, 1050},
    {1100, 820},
    {250, 820},
    {250, 1050},
    {60, 1050}
  };

  EXPECT_TRUE(inflated.has_value());
  for (const auto &point : inflated->vertices()) {
    LOG(INFO) << point;
  }
  EXPECT_EQ(expected_vertices, inflated->vertices());
}

TEST(PolyLineInflatorTest, TODO_ThisShouldNotBreak2) {
  PolyLine line = PolyLine(
      {60, 495}, {
      LineSegment {{335, 495}, 170},
      LineSegment {{335, 580}, 170},
      LineSegment {{335, 565}, 170},
      LineSegment {{2220, 565}, 170},
      LineSegment {{2220, 890}, 170},
      LineSegment {{2220, 1220}, 170}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);

  std::vector<geometry::Point> expected_vertices = {
    {60, 1220},
    {420, 1220},
    {420, 990},
    {930, 990},
    {930, 1220},
    {1485, 1220},
    {1485, 1050},
    {1100, 1050},
    {1100, 820},
    {250, 820},
    {250, 1050},
    {60, 1050}
  };

  EXPECT_TRUE(inflated.has_value());
  for (const auto &point : inflated->vertices()) {
    LOG(INFO) << point;
  }
  EXPECT_EQ(expected_vertices, inflated->vertices());
}

TEST(PolyLineInflatorTest, TODO_ThisShouldNotBreak3) {
  PolyLine line = PolyLine(
      {3065, 3355}, {
      LineSegment {{3820, 3355}, 210},
      LineSegment {{3935, 3355}, 250},
      LineSegment {{3935, 3315}, 290},
      LineSegment {{4805, 3315}, 290}
  });

  PhysicalPropertiesDatabase db;
  PolyLineInflator inflator(db);

  std::optional<geometry::Polygon> inflated =
      inflator.InflatePolyLine(line);
  EXPECT_TRUE(inflated.has_value());
  for (const auto &point : inflated->vertices()) {
    LOG(INFO) << point;
  }
}

}  // namespace
}  // namespace bfg
