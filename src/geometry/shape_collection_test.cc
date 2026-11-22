#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "point.h"
#include "polygon.h"
#include "poly_line.h"
#include "port.h"
#include "rectangle.h"
#include "shape_collection.h"
#include "../equivalent_nets.h"

namespace bfg {
namespace geometry {
namespace {

constexpr Layer kLayerA = 1;
constexpr Layer kLayerB = 2;

// Builds a collection with one shape of each kind on each of nets "a" and "b",
// plus an unconnectable rectangle with no net. Shapes on net "a" are on
// kLayerA, those on net "b" on kLayerB.
//
// Rectangles and polygons on net "a" are connectable; those on net "b" are
// not. Ports are always connectable.
void PopulateCollection(ShapeCollection *collection) {
  Rectangle *rect_a = new Rectangle({0, 0}, {10, 10}, kLayerA, "a");
  rect_a->set_is_connectable(true);
  collection->rectangles().emplace_back(rect_a);
  collection->rectangles().emplace_back(
      new Rectangle({20, 0}, {30, 10}, kLayerB, "b"));
  collection->rectangles().emplace_back(
      new Rectangle({40, 0}, {50, 10}, kLayerA, ""));

  Polygon *poly_a = new Polygon({{0, 20}, {0, 30}, {10, 30}, {10, 20}});
  poly_a->set_layer(kLayerA);
  poly_a->set_net("a");
  poly_a->set_is_connectable(true);
  collection->polygons().emplace_back(poly_a);
  Polygon *poly_b = new Polygon({{20, 20}, {20, 30}, {30, 30}, {30, 20}});
  poly_b->set_layer(kLayerB);
  poly_b->set_net("b");
  collection->polygons().emplace_back(poly_b);

  collection->ports().emplace_back(new Port({0, 40}, {5, 45}, kLayerA, "a"));
  collection->ports().emplace_back(new Port({20, 40}, {25, 45}, kLayerB, "b"));

  PolyLine *line_a = new PolyLine(std::vector<Point>{{0, 60}, {10, 60}});
  line_a->set_layer(kLayerA);
  line_a->set_net("a");
  line_a->set_is_connectable(true);
  collection->poly_lines().emplace_back(line_a);
  PolyLine *line_b = new PolyLine(std::vector<Point>{{20, 60}, {30, 60}});
  line_b->set_layer(kLayerB);
  line_b->set_net("b");
  collection->poly_lines().emplace_back(line_b);
}

// Collects the net names of every shape in the collection, in order.
//
// PolyLine's copy constructor currently drops the net (see
// DISABLED_CopiedPolyLinesKeepNet below), so tests of the methods that copy
// shapes leave poly lines out with include_poly_lines = false.
std::vector<std::string> AllNets(const ShapeCollection &collection,
                                 bool include_poly_lines = true) {
  std::vector<std::string> nets;
  for (const auto &shape : collection.rectangles()) {
    nets.push_back(shape->net());
  }
  for (const auto &shape : collection.polygons()) {
    nets.push_back(shape->net());
  }
  for (const auto &shape : collection.ports()) {
    nets.push_back(shape->net());
  }
  if (!include_poly_lines) {
    return nets;
  }
  for (const auto &shape : collection.poly_lines()) {
    nets.push_back(shape->net());
  }
  return nets;
}

TEST(ShapeCollectionTest, Empty) {
  ShapeCollection collection;
  EXPECT_TRUE(collection.Empty());

  collection.poly_lines().emplace_back(
      new PolyLine(std::vector<Point>{{0, 0}, {1, 0}}));
  EXPECT_FALSE(collection.Empty());
}

TEST(ShapeCollectionTest, AddCopiesEverything) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection copy;
  copy.Add(source);
  EXPECT_EQ(3, copy.rectangles().size());
  EXPECT_EQ(2, copy.polygons().size());
  EXPECT_EQ(2, copy.ports().size());
  EXPECT_EQ(2, copy.poly_lines().size());

  // The copy is deep: changing it does not change the source.
  copy.rectangles().front()->set_net("changed");
  EXPECT_EQ("a", source.rectangles().front()->net());
}

TEST(ShapeCollectionTest, CopyConstructor) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection copy(source);
  EXPECT_EQ(AllNets(source, false), AllNets(copy, false));
  EXPECT_EQ(source.poly_lines().size(), copy.poly_lines().size());
}

TEST(ShapeCollectionTest, AddConnectableShapes) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection connectable;
  connectable.AddConnectableShapes(source);
  std::vector<std::string> expected = {"a", "a", "a", "b"};
  EXPECT_EQ(expected, AllNets(connectable, false));
  EXPECT_EQ(1, connectable.poly_lines().size());
}

TEST(ShapeCollectionTest, AddNonConnectableShapes) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection non_connectable;
  non_connectable.AddNonConnectableShapes(source);
  std::vector<std::string> expected = {"b", "", "b"};
  EXPECT_EQ(expected, AllNets(non_connectable, false));
  EXPECT_EQ(1, non_connectable.poly_lines().size());
}

TEST(ShapeCollectionTest, AddConnectableShapesOnNets) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection on_nets;
  on_nets.AddConnectableShapesOnNets(source, EquivalentNets("b"));
  // Only the port on "b" is connectable.
  std::vector<std::string> expected = {"b"};
  EXPECT_EQ(expected, AllNets(on_nets));
}

TEST(ShapeCollectionTest, AddConnectableShapesNotOnNets) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection not_on_nets;
  not_on_nets.AddConnectableShapesNotOnNets(source, EquivalentNets("b"));
  std::vector<std::string> expected = {"a", "a", "a"};
  EXPECT_EQ(expected, AllNets(not_on_nets, false));
  EXPECT_EQ(1, not_on_nets.poly_lines().size());
}

// TODO(aryap): PolyLine's copy constructor does not copy the Shape base, so
// copies lose their layer, net and connectability. Enable once fixed.
TEST(ShapeCollectionTest, DISABLED_CopiedPolyLinesKeepNet) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection copy(source);
  ASSERT_EQ(2, copy.poly_lines().size());
  const PolyLine &original = *source.poly_lines().front();
  const PolyLine &copied = *copy.poly_lines().front();
  EXPECT_EQ(original.net(), copied.net());
  EXPECT_EQ(original.layer(), copied.layer());
  EXPECT_EQ(original.is_connectable(), copied.is_connectable());
}

TEST(ShapeCollectionTest, Consume) {
  ShapeCollection source;
  PopulateCollection(&source);

  ShapeCollection target;
  target.rectangles().emplace_back(
      new Rectangle({100, 100}, {110, 110}, kLayerA, "c"));
  target.Consume(&source);

  // Consumed shapes are appended after the existing ones.
  EXPECT_EQ(4, target.rectangles().size());
  EXPECT_EQ(2, target.polygons().size());
  EXPECT_EQ(2, target.ports().size());
  EXPECT_EQ(2, target.poly_lines().size());
  EXPECT_EQ("c", target.rectangles().front()->net());
  EXPECT_EQ("a", target.rectangles().at(1)->net());
}

TEST(ShapeCollectionTest, KeepOnlyNets) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.KeepOnlyNets(EquivalentNets("a"));
  std::vector<std::string> expected = {"a", "a", "a", "a"};
  EXPECT_EQ(expected, AllNets(collection));
}

TEST(ShapeCollectionTest, KeepOnlyNets_Aliases) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.KeepOnlyNets(EquivalentNets(std::set<std::string>{"a", "b"}));
  std::vector<std::string> expected = {"a", "b", "a", "b", "a", "b", "a", "b"};
  EXPECT_EQ(expected, AllNets(collection));
}

TEST(ShapeCollectionTest, RemoveNets) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.RemoveNets(EquivalentNets("a"));
  std::vector<std::string> expected = {"b", "", "b", "b", "b"};
  EXPECT_EQ(expected, AllNets(collection));
}

TEST(ShapeCollectionTest, KeepOnlyLayers) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.KeepOnlyLayers({kLayerB});
  std::vector<std::string> expected = {"b", "b", "b", "b"};
  EXPECT_EQ(expected, AllNets(collection));

  collection.KeepOnlyLayers({});
  EXPECT_TRUE(collection.Empty());
}

TEST(ShapeCollectionTest, PrefixNetNames) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.PrefixNetNames("inst");
  // Shapes without a net are left alone.
  std::vector<std::string> expected = {
      "inst.a", "inst.b", "", "inst.a", "inst.b", "inst.a", "inst.b",
      "inst.a", "inst.b"};
  EXPECT_EQ(expected, AllNets(collection));
}

TEST(ShapeCollectionTest, PrefixNetNames_SeparatorAndExceptions) {
  ShapeCollection collection;
  PopulateCollection(&collection);

  collection.PrefixNetNames("inst", "/", {"b"});
  std::vector<std::string> expected = {
      "inst/a", "b", "", "inst/a", "b", "inst/a", "b", "inst/a", "b"};
  EXPECT_EQ(expected, AllNets(collection));
}

TEST(ShapeCollectionTest, GetBoundingBox) {
  ShapeCollection collection;
  collection.rectangles().emplace_back(
      new Rectangle({-5, 0}, {10, 10}, kLayerA, ""));
  Polygon *polygon = new Polygon({{0, 0}, {0, 30}, {20, 30}});
  collection.polygons().emplace_back(polygon);
  collection.ports().emplace_back(new Port({0, -7}, {1, 1}, kLayerA, ""));

  EXPECT_EQ(Rectangle({-5, -7}, {20, 30}), collection.GetBoundingBox());
}

TEST(ShapeCollectionTest, Describe) {
  ShapeCollection collection;
  EXPECT_EQ("", collection.Describe());

  PopulateCollection(&collection);
  std::string description = collection.Describe();
  EXPECT_NE(std::string::npos, description.find("rect 0 0 10 10 connectable"));
  EXPECT_NE(std::string::npos, description.find("net: b"));
  EXPECT_NE(std::string::npos, description.find("polygon "));
  EXPECT_NE(std::string::npos, description.find("port 0 40 5 45"));
  EXPECT_NE(std::string::npos, description.find("poly line "));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
