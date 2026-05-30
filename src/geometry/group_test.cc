#include <gtest/gtest.h>

#include "group.h"
#include "point.h"
#include "poly_line.h"
#include "polygon.h"
#include "port.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {
namespace {

TEST(Group, DefaultConstruct) {
  Group group;
  EXPECT_TRUE(group.rectangles().empty());
  EXPECT_TRUE(group.polygons().empty());
  EXPECT_TRUE(group.ports().empty());
  EXPECT_TRUE(group.poly_lines().empty());
}

TEST(Group, ConstructFromRectangles) {
  Rectangle r1(Point(0, 0), Point(10, 10));
  Rectangle r2(Point(20, 20), Point(30, 30));
  Group group({&r1, &r2});
  EXPECT_EQ(group.rectangles().size(), 2u);
}

TEST(Group, AddRectangle) {
  Rectangle r(Point(0, 0), Point(10, 10));
  Group group;
  group.Add(&r);
  ASSERT_EQ(group.rectangles().size(), 1u);
  EXPECT_EQ(group.rectangles()[0], &r);
}

TEST(Group, AddPolygon) {
  Polygon p({Point(0, 0), Point(10, 0), Point(5, 10)});
  Group group;
  group.Add(&p);
  ASSERT_EQ(group.polygons().size(), 1u);
  EXPECT_EQ(group.polygons()[0], &p);
}

TEST(Group, AddPort) {
  Port port(Point(0, 0), Point(10, 10), 0, "net");
  Group group;
  group.Add(&port);
  ASSERT_EQ(group.ports().size(), 1u);
  EXPECT_EQ(group.ports()[0], &port);
}

TEST(Group, AddPolyLine) {
  PolyLine pl;
  Group group;
  group.Add(&pl);
  ASSERT_EQ(group.poly_lines().size(), 1u);
  EXPECT_EQ(group.poly_lines()[0], &pl);
}

TEST(Group, GetBoundingBox_Empty) {
  Group group;
  EXPECT_FALSE(group.GetBoundingBox().has_value());
}

TEST(Group, GetBoundingBox_SingleRectangle) {
  Rectangle r(Point(10, 20), Point(30, 40));
  Group group;
  group.Add(&r);
  auto bb = group.GetBoundingBox();
  ASSERT_TRUE(bb.has_value());
  EXPECT_EQ(bb->lower_left(), Point(10, 20));
  EXPECT_EQ(bb->upper_right(), Point(30, 40));
}

TEST(Group, GetBoundingBox_MultipleRectangles) {
  Rectangle r1(Point(0, 0), Point(10, 10));
  Rectangle r2(Point(5, 5), Point(20, 25));
  Group group;
  group.Add(&r1);
  group.Add(&r2);
  auto bb = group.GetBoundingBox();
  ASSERT_TRUE(bb.has_value());
  EXPECT_EQ(bb->lower_left(), Point(0, 0));
  EXPECT_EQ(bb->upper_right(), Point(20, 25));
}

TEST(Group, GetBoundingBox_SinglePolygon) {
  Polygon p({Point(5, 5), Point(15, 5), Point(15, 15), Point(5, 15)});
  Group group;
  group.Add(&p);
  auto bb = group.GetBoundingBox();
  ASSERT_TRUE(bb.has_value());
  EXPECT_EQ(bb->lower_left(), Point(5, 5));
  EXPECT_EQ(bb->upper_right(), Point(15, 15));
}

TEST(Group, GetBoundingBox_SinglePort) {
  Port port(Point(0, 0), Point(50, 50), 0, "vdd");
  Group group;
  group.Add(&port);
  auto bb = group.GetBoundingBox();
  ASSERT_TRUE(bb.has_value());
  EXPECT_EQ(bb->lower_left(), Point(0, 0));
  EXPECT_EQ(bb->upper_right(), Point(50, 50));
}

TEST(Group, GetBoundingBox_MixedRectangleAndPolygon) {
  Rectangle r(Point(0, 0), Point(10, 10));
  Polygon p({Point(8, 8), Point(20, 8), Point(20, 20)});
  Group group;
  group.Add(&r);
  group.Add(&p);
  auto bb = group.GetBoundingBox();
  ASSERT_TRUE(bb.has_value());
  EXPECT_EQ(bb->lower_left(), Point(0, 0));
  EXPECT_EQ(bb->upper_right(), Point(20, 20));
}

TEST(Group, GetBoundingBox_PolyLinesIgnored) {
  PolyLine pl;
  Group group;
  group.Add(&pl);
  EXPECT_FALSE(group.GetBoundingBox().has_value());
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
