#include "layout.h"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "dev_pdk_setup.h"
#include "edge_list.h"
#include "equivalent_nets.h"
#include "geometry/group.h"
#include "geometry/instance.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"
#include "geometry/shape_collection.h"
#include "physical_properties_database.h"
#include "scoped_layer.h"

namespace bfg {
namespace {

using geometry::Instance;
using geometry::Point;
using geometry::Port;
using geometry::PortSet;
using geometry::Rectangle;

// Layouts keep a reference to their PhysicalPropertiesDatabase, so it has to
// outlive them.
class LayoutTest : public testing::Test {
 protected:
  std::unique_ptr<Layout> MakeLayout() {
    return std::make_unique<Layout>(physical_db_);
  }

  PhysicalPropertiesDatabase physical_db_;
};

// Like LayoutTest, but with real Sky130 rules for functions that need them.
class LayoutSky130Test : public testing::Test {
 protected:
  void SetUp() override {
    physical_db_.LoadTechnologyFromFile("test_data/sky130.technology.pb");
    bfg::SetUpSky130(&physical_db_);
  }

  std::vector<Rectangle*> RectanglesOn(
      const Layout &layout, const std::string &layer_name) {
    std::vector<Rectangle*> rectangles;
    ShapeCollection *shapes = layout.GetShapeCollection(
        physical_db_.GetLayer(layer_name));
    if (!shapes) {
      return rectangles;
    }
    for (const auto &rectangle : shapes->rectangles()) {
      rectangles.push_back(rectangle.get());
    }
    return rectangles;
  }

  size_t PolygonCountOn(const Layout &layout, const std::string &layer_name) {
    ShapeCollection *shapes = layout.GetShapeCollection(
        physical_db_.GetLayer(layer_name));
    return shapes ? shapes->polygons().size() : 0;
  }

  PhysicalPropertiesDatabase physical_db_;
};

TEST_F(LayoutTest, GetBoundingBox_Empty) {
  std::unique_ptr<Layout> layout = MakeLayout();
  EXPECT_EQ(Rectangle({0, 0}, {0, 0}), layout->GetBoundingBox());
}

TEST_F(LayoutTest, GetBoundingBox_IgnoresEmptyLayers) {
  std::unique_ptr<Layout> layout = MakeLayout();

  layout->set_active_layer(1);
  layout->AddRectangle(Rectangle({10, 10}, {20, 30}));

  // Leave an empty shape collection behind on layer 2.
  layout->set_active_layer(2);
  layout->AddRectangle(Rectangle({-100, -100}, {-90, -90}));
  layout->GetShapeCollection(2)->rectangles().clear();

  EXPECT_EQ(Rectangle({10, 10}, {20, 30}), layout->GetBoundingBox());
}

TEST_F(LayoutTest, GetBoundingBox_IncludesInstances) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {5, 5}));

  std::unique_ptr<Layout> parent = MakeLayout();
  parent->AddRectangle(Rectangle({0, 0}, {1, 1}));
  parent->AddInstance(Instance(child.get(), {20, 30}));

  EXPECT_EQ(Rectangle({0, 0}, {25, 35}), parent->GetBoundingBox());
}

TEST_F(LayoutTest, Translate) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {1, 1}));

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddRectangle(Rectangle({1, 2}, {3, 4}));
  layout->AddPort(Port({5, 5}, {6, 6}, 0, "p"));
  layout->SavePoint("pt", {7, 8});
  layout->SetTilingBounds(Rectangle({0, 0}, {10, 10}));
  Instance *instance = layout->AddInstance(Instance(child.get(), {2, 2}));

  layout->Translate({100, 200});

  ShapeCollection *shapes = layout->GetShapeCollection(0);
  EXPECT_EQ(Rectangle({101, 202}, {103, 204}), *shapes->rectangles().front());
  EXPECT_EQ(Point(105, 205), shapes->ports().front()->lower_left());
  EXPECT_EQ(Point(107, 208), layout->GetPointOrDie("pt"));
  EXPECT_EQ(Rectangle({100, 200}, {110, 210}), layout->GetTilingBounds());
  EXPECT_EQ(Point(102, 202), instance->lower_left());
}

TEST_F(LayoutTest, ResetOrigin) {
  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddRectangle(Rectangle({5, 7}, {10, 10}));
  layout->AddRectangle(Rectangle({8, 9}, {12, 11}));

  layout->ResetOrigin();
  EXPECT_EQ(Rectangle({0, 0}, {7, 4}), layout->GetBoundingBox());
}

TEST_F(LayoutTest, Rotate) {
  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddRectangle(Rectangle({1, 0}, {2, 1}));
  layout->SavePoint("pt", {5, 0});

  // Rotations are counter-clockwise about the origin.
  layout->Rotate(90);
  EXPECT_EQ(Rectangle({-1, 1}, {0, 2}),
            *layout->GetShapeCollection(0)->rectangles().front());
  EXPECT_EQ(Point(0, 5), layout->GetPointOrDie("pt"));
}

TEST_F(LayoutTest, MirrorY) {
  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddRectangle(Rectangle({1, 0}, {3, 1}));
  layout->SavePoint("pt", {5, 6});

  layout->MirrorY();
  EXPECT_EQ(Rectangle({-3, 0}, {-1, 1}),
            *layout->GetShapeCollection(0)->rectangles().front());
  EXPECT_EQ(Point(-5, 6), layout->GetPointOrDie("pt"));
}

TEST_F(LayoutTest, MirrorX) {
  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddRectangle(Rectangle({1, 2}, {3, 5}));
  layout->SavePoint("pt", {5, 6});

  layout->MirrorX();
  EXPECT_EQ(Rectangle({1, -5}, {3, -2}),
            *layout->GetShapeCollection(0)->rectangles().front());
  EXPECT_EQ(Point(5, -6), layout->GetPointOrDie("pt"));
}

TEST_F(LayoutTest, MirrorY_MovesInstances) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {2, 2}));

  std::unique_ptr<Layout> parent = MakeLayout();
  Instance *instance = parent->AddInstance(Instance(child.get(), {10, 0}));
  ASSERT_EQ(Rectangle({10, 0}, {12, 2}), instance->GetBoundingBox());

  parent->MirrorY();
  EXPECT_EQ(Rectangle({-12, 0}, {-10, 2}), instance->GetBoundingBox());
}

// TODO(aryap): MirrorX reflects instances about their own origin but, unlike
// MirrorY, does not mirror their position.
TEST_F(LayoutTest, DISABLED_MirrorX_MovesInstances) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {2, 2}));

  std::unique_ptr<Layout> parent = MakeLayout();
  Instance *instance = parent->AddInstance(Instance(child.get(), {0, 10}));
  ASSERT_EQ(Rectangle({0, 10}, {2, 12}), instance->GetBoundingBox());

  parent->MirrorX();
  EXPECT_EQ(Rectangle({0, -12}, {2, -10}), instance->GetBoundingBox());
}

TEST_F(LayoutTest, AddLayout) {
  std::unique_ptr<Layout> other = MakeLayout();
  other->set_active_layer(3);
  other->AddRectangle(Rectangle({0, 0}, {1, 1}, 3, "a"));
  other->AddRectangle(Rectangle({2, 2}, {3, 3}, 3, "vdd"));
  other->AddPort(Port({0, 0}, {1, 1}, 3, "a"));
  other->AddGlobalNet("vdd");
  other->SavePoint("pt", {1, 1});

  std::unique_ptr<Layout> child = MakeLayout();
  Instance child_instance(child.get());
  child_instance.set_name("c");
  other->AddInstance(child_instance);

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddLayout(*other, "x");

  ShapeCollection *shapes = layout->GetShapeCollection(3);
  ASSERT_NE(nullptr, shapes);
  ASSERT_EQ(2, shapes->rectangles().size());
  EXPECT_EQ("x.a", shapes->rectangles().at(0)->net());
  // Global nets are not prefixed.
  EXPECT_EQ("vdd", shapes->rectangles().at(1)->net());
  EXPECT_TRUE(layout->HasPort("x.a"));
  EXPECT_EQ(Point(1, 1), layout->GetPointOrDie("x.pt"));
  ASSERT_EQ(1, layout->instances().size());
  EXPECT_EQ("x.c", layout->instances().front()->name());

  // The source is unchanged.
  EXPECT_EQ("a", other->GetShapeCollection(3)->rectangles().front()->net());
}

TEST_F(LayoutTest, AddLayout_WithoutPorts) {
  std::unique_ptr<Layout> other = MakeLayout();
  other->AddPort(Port({0, 0}, {1, 1}, 0, "a"));

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddLayout(*other, "", false);
  EXPECT_FALSE(layout->HasPort("a"));
  EXPECT_TRUE(layout->Ports().empty());
}

TEST_F(LayoutTest, ConsumeLayout) {
  std::unique_ptr<Layout> other = MakeLayout();
  other->set_active_layer(3);
  other->AddRectangle(Rectangle({0, 0}, {1, 1}, 3, "a"));
  other->SavePoint("pt", {1, 1});
  std::unique_ptr<Layout> child = MakeLayout();
  other->AddInstance(Instance(child.get()));

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->ConsumeLayout(other.get(), "x");

  ShapeCollection *shapes = layout->GetShapeCollection(3);
  ASSERT_NE(nullptr, shapes);
  ASSERT_EQ(1, shapes->rectangles().size());
  EXPECT_EQ("x.a", shapes->rectangles().front()->net());
  EXPECT_EQ(Point(1, 1), layout->GetPointOrDie("x.pt"));
  EXPECT_EQ(1, layout->instances().size());
}

// TODO(aryap): ShapeCollection::Consume moves the unique_ptrs out but leaves
// the (now null) entries behind in the source collection.
TEST_F(LayoutTest, DISABLED_ConsumeLayout_EmptiesSource) {
  std::unique_ptr<Layout> other = MakeLayout();
  other->AddRectangle(Rectangle({0, 0}, {1, 1}));

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->ConsumeLayout(other.get());
  EXPECT_TRUE(other->GetShapeCollection(0)->Empty());
}

// TODO(aryap): ConsumeLayout moves ports but not ports_by_net_, so they can't
// be found by net in the new layout.
TEST_F(LayoutTest, DISABLED_ConsumeLayout_KeepsPortsByNet) {
  std::unique_ptr<Layout> other = MakeLayout();
  other->AddPort(Port({0, 0}, {1, 1}, 0, "a"));

  std::unique_ptr<Layout> layout = MakeLayout();
  layout->ConsumeLayout(other.get(), "x");
  EXPECT_EQ(1, layout->Ports().size());
  EXPECT_TRUE(layout->HasPort("x.a"));
}

TEST_F(LayoutTest, Flatten) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {2, 2}, 0, "a"));
  child->SetTilingBounds(Rectangle({0, 0}, {4, 4}));

  std::unique_ptr<Layout> parent = MakeLayout();
  Instance instance(child.get(), {10, 0});
  instance.set_name("i0");
  parent->AddInstance(instance);

  parent->Flatten(true);
  EXPECT_TRUE(parent->instances().empty());
  ShapeCollection *shapes = parent->GetShapeCollection(0);
  ASSERT_NE(nullptr, shapes);
  ASSERT_EQ(1, shapes->rectangles().size());
  EXPECT_EQ(Rectangle({10, 0}, {12, 2}), *shapes->rectangles().front());
  EXPECT_EQ("i0.a", shapes->rectangles().front()->net());
  // Tiling bounds grow to cover the flattened instance's.
  EXPECT_EQ(Rectangle({10, 0}, {14, 4}), parent->GetTilingBounds());
}

TEST_F(LayoutTest, Flatten_NoPrefixes) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {2, 2}, 0, "a"));

  std::unique_ptr<Layout> parent = MakeLayout();
  Instance instance(child.get(), {10, 0});
  instance.set_name("i0");
  parent->AddInstance(instance);

  parent->Flatten(false);
  EXPECT_EQ("a", parent->GetShapeCollection(0)->rectangles().front()->net());
}

TEST_F(LayoutTest, Flatten_RotatedMatchesBoundingBox) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({1, 0}, {2, 1}));

  std::unique_ptr<Layout> parent = MakeLayout();
  Instance *instance = parent->AddInstance(Instance(child.get(), {5, 5}));
  instance->set_rotation_degrees_ccw(90);
  Rectangle expected = instance->GetBoundingBox();

  parent->Flatten();
  EXPECT_EQ(expected, *parent->GetShapeCollection(0)->rectangles().front());
}

TEST_F(LayoutTest, CopyAllShapes_Depth) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "b"));

  std::unique_ptr<Layout> parent = MakeLayout();
  parent->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "a"));
  Instance instance(child.get(), {10, 10});
  instance.set_name("i");
  parent->AddInstance(instance);

  ShapeCollection shallow;
  parent->CopyAllShapes(&shallow, 0);
  ASSERT_EQ(1, shallow.rectangles().size());
  EXPECT_EQ("a", shallow.rectangles().front()->net());

  ShapeCollection deep;
  parent->CopyAllShapes(&deep, std::nullopt);
  ASSERT_EQ(2, deep.rectangles().size());
  EXPECT_EQ("i.b", deep.rectangles().at(1)->net());
  EXPECT_EQ(Point(10, 10), deep.rectangles().at(1)->lower_left());

  ShapeCollection no_prefix;
  parent->CopyAllShapes(&no_prefix, std::nullopt, std::set<std::string>{"b"});
  EXPECT_EQ("b", no_prefix.rectangles().at(1)->net());
}

TEST_F(LayoutTest, CopyConnectableShapesOnNets) {
  std::unique_ptr<Layout> child = MakeLayout();
  Rectangle *child_rect =
      child->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "b"));
  child_rect->set_is_connectable(true);

  std::unique_ptr<Layout> parent = MakeLayout();
  Rectangle *on_net = parent->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "a"));
  on_net->set_is_connectable(true);
  parent->AddRectangle(Rectangle({2, 2}, {3, 3}, 0, "a"));  // Unconnectable.
  Rectangle *off_net =
      parent->AddRectangle(Rectangle({4, 4}, {5, 5}, 0, "c"));
  off_net->set_is_connectable(true);
  Instance instance(child.get(), {10, 10});
  instance.set_name("i");
  parent->AddInstance(instance);

  ShapeCollection on_nets;
  parent->CopyConnectableShapesOnNets(
      EquivalentNets(std::set<std::string>{"a", "i.b"}), &on_nets);
  ASSERT_EQ(2, on_nets.rectangles().size());
  EXPECT_EQ("a", on_nets.rectangles().at(0)->net());
  EXPECT_EQ("i.b", on_nets.rectangles().at(1)->net());

  ShapeCollection not_on_nets;
  parent->CopyConnectableShapesNotOnNets(EquivalentNets("a"), &not_on_nets);
  ASSERT_EQ(2, not_on_nets.rectangles().size());
  EXPECT_EQ("c", not_on_nets.rectangles().at(0)->net());
  EXPECT_EQ("i.b", not_on_nets.rectangles().at(1)->net());

  ShapeCollection shallow;
  parent->CopyConnectableShapesNotOnNets(EquivalentNets("a"), &shallow, 0);
  ASSERT_EQ(1, shallow.rectangles().size());
  EXPECT_EQ("c", shallow.rectangles().front()->net());
}

TEST_F(LayoutTest, CopyShapesOnLayer) {
  std::unique_ptr<Layout> child = MakeLayout();
  child->set_active_layer(1);
  child->AddRectangle(Rectangle({0, 0}, {1, 1}, 1, "b"));
  child->set_active_layer(2);
  child->AddRectangle(Rectangle({0, 0}, {1, 1}, 2, "c"));

  std::unique_ptr<Layout> parent = MakeLayout();
  parent->set_active_layer(1);
  parent->AddRectangle(Rectangle({0, 0}, {1, 1}, 1, "a"));
  Instance instance(child.get(), {10, 10});
  instance.set_name("i");
  parent->AddInstance(instance);

  ShapeCollection shapes;
  parent->CopyShapesOnLayer(1, &shapes);
  ASSERT_EQ(2, shapes.rectangles().size());
  EXPECT_EQ("a", shapes.rectangles().at(0)->net());
  EXPECT_EQ("i.b", shapes.rectangles().at(1)->net());
}

TEST_F(LayoutTest, PortsAndDeletePorts) {
  std::unique_ptr<Layout> layout = MakeLayout();
  layout->AddPort(Port({0, 0}, {1, 1}, 0, "a"));
  layout->AddPort(Port({2, 0}, {3, 1}, 0, "a"));
  // PortSets order ports by position alone, so keep these apart.
  layout->AddPort(Port({4, 0}, {5, 1}, 0, "b"), "pre");

  EXPECT_EQ(3, layout->Ports().size());
  EXPECT_TRUE(layout->HasPort("a"));
  EXPECT_TRUE(layout->HasPort("pre.b"));
  EXPECT_FALSE(layout->HasPort("b"));

  PortSet ports = Port::MakePortSet();
  layout->GetPorts("a", &ports);
  EXPECT_EQ(2, ports.size());

  layout->DeletePorts("a");
  EXPECT_FALSE(layout->HasPort("a"));
  EXPECT_EQ(1, layout->Ports().size());
}

TEST_F(LayoutTest, AddRectangleAsPort) {
  std::unique_ptr<Layout> layout = MakeLayout();
  Rectangle *rectangle =
      layout->AddRectangleAsPort(Rectangle({0, 0}, {4, 4}), "a");
  EXPECT_EQ("a", rectangle->net());
  EXPECT_TRUE(layout->HasPort("a"));
}

TEST_F(LayoutTest, GetInstancesByName) {
  std::unique_ptr<Layout> child = MakeLayout();
  std::unique_ptr<Layout> parent = MakeLayout();
  Instance named(child.get());
  named.set_name("n");
  parent->AddInstance(named);
  parent->AddInstance(Instance(child.get()));

  std::unordered_map<std::string, Instance *const> mapping;
  parent->GetInstancesByName(&mapping);
  ASSERT_EQ(1, mapping.size());
  EXPECT_EQ("n", mapping.begin()->first);
}

TEST_F(LayoutTest, EraseLayerAndRemoveLayerFromChildTemplates) {
  std::unique_ptr<Layout> grandchild = MakeLayout();
  grandchild->set_active_layer(1);
  grandchild->AddRectangle(Rectangle({0, 0}, {1, 1}));

  std::unique_ptr<Layout> child = MakeLayout();
  child->set_active_layer(1);
  child->AddRectangle(Rectangle({0, 0}, {1, 1}));
  child->set_active_layer(2);
  child->AddRectangle(Rectangle({0, 0}, {1, 1}));
  child->AddInstance(Instance(grandchild.get()));

  std::unique_ptr<Layout> parent = MakeLayout();
  parent->AddInstance(Instance(child.get()));

  parent->RemoveLayerFromChildTemplates(1);
  EXPECT_EQ(nullptr, child->GetShapeCollection(1));
  EXPECT_NE(nullptr, child->GetShapeCollection(2));
  EXPECT_EQ(nullptr, grandchild->GetShapeCollection(1));

  child->EraseLayer(2);
  EXPECT_EQ(nullptr, child->GetShapeCollection(2));
}

class LayoutEdgeSpecTest : public LayoutTest {
 protected:
  void SetUp() override {
    child_ = MakeLayout();
    child_->AddPort(Port({0, 0}, {1, 1}, 0, "x"));
    parent_ = MakeLayout();
    Instance a(child_.get(), {0, 0});
    a.set_name("a");
    a_ = parent_->AddInstance(a);
    Instance b(child_.get(), {10, 0});
    b.set_name("b");
    b_ = parent_->AddInstance(b);
  }

  std::unique_ptr<Layout> child_;
  std::unique_ptr<Layout> parent_;
  Instance *a_;
  Instance *b_;
};

// TODO(aryap): ResolveEdgeSpec always writes the matched instance to *from,
// and only succeeds if one instance matches both endpoints.
TEST_F(LayoutEdgeSpecTest, DISABLED_ResolvesDifferentInstances) {
  EdgeSpec spec;
  spec.set_from("a", {"x"});
  spec.set_to("b", {"x"});

  Instance *from = nullptr;
  Instance *to = nullptr;
  std::vector<Port*> from_ports;
  std::vector<Port*> to_ports;
  ASSERT_TRUE(
      parent_->ResolveEdgeSpec(spec, &from, &from_ports, &to, &to_ports));
  EXPECT_EQ(a_, from);
  EXPECT_EQ(b_, to);
  ASSERT_EQ(1, from_ports.size());
  ASSERT_EQ(1, to_ports.size());
  EXPECT_EQ("a.x", from_ports.front()->net());
  EXPECT_EQ("b.x", to_ports.front()->net());
}

TEST_F(LayoutEdgeSpecTest, NoMatch) {
  EdgeSpec spec;
  spec.set_from("a", {"x"});
  spec.set_to("nonexistent", {"x"});

  Instance *from = nullptr;
  Instance *to = nullptr;
  std::vector<Port*> from_ports;
  std::vector<Port*> to_ports;
  EXPECT_FALSE(
      parent_->ResolveEdgeSpec(spec, &from, &from_ports, &to, &to_ports));
}

TEST_F(LayoutSky130Test, MakeVia) {
  Layout layout(physical_db_);
  geometry::Layer mcon = physical_db_.GetLayer("mcon.drawing");
  int64_t side = physical_db_.Rules(mcon).via_width;
  ASSERT_GT(side, 0);

  Rectangle *via = layout.MakeVia(mcon, {1000, 2000}, "n");
  EXPECT_EQ(mcon, via->layer());
  EXPECT_EQ("n", via->net());
  EXPECT_EQ(Point(1000, 2000), via->centre());
  EXPECT_EQ(side, via->Width());
  EXPECT_EQ(side, via->Height());
}

TEST_F(LayoutSky130Test, MakeVia_RestoresActiveLayer) {
  Layout layout(physical_db_);
  layout.SetActiveLayerByName("met1.drawing");
  layout.MakeVia(physical_db_.GetLayer("mcon.drawing"), {0, 0});
  Rectangle *added = layout.AddRectangle(Rectangle({0, 0}, {1, 1}));
  EXPECT_EQ(physical_db_.GetLayer("met1.drawing"), added->layer());
}

TEST_F(LayoutSky130Test, StampVias) {
  Layout layout(physical_db_);
  geometry::Layer mcon = physical_db_.GetLayer("mcon.drawing");
  const auto &rules = physical_db_.Rules(mcon);
  int64_t side = std::max(rules.via_width, rules.via_height);
  int64_t pitch = 2 * side;
  int64_t length = 10 * pitch;

  layout.StampVias(mcon, {0, 0}, {length, 0}, pitch, "n");
  std::vector<Rectangle*> vias = RectanglesOn(layout, "mcon.drawing");
  ASSERT_EQ(10, vias.size());
  for (size_t i = 0; i < vias.size(); ++i) {
    // Each via sits in the middle of its pitch-sized slot.
    int64_t expected_x = static_cast<int64_t>(i) * pitch + pitch / 2;
    EXPECT_EQ(Point(expected_x, 0), vias[i]->centre());
    EXPECT_EQ("n", vias[i]->net());
  }
}

TEST_F(LayoutSky130Test, StampVias_TooShort) {
  Layout layout(physical_db_);
  geometry::Layer mcon = physical_db_.GetLayer("mcon.drawing");
  int64_t side = physical_db_.Rules(mcon).via_width;
  layout.StampVias(mcon, {0, 0}, {side - 1, 0}, 2 * side);
  EXPECT_TRUE(RectanglesOn(layout, "mcon.drawing").empty());
}

TEST_F(LayoutSky130Test, MakePin) {
  Layout layout(physical_db_);
  layout.MakePin("a", {500, 500}, "met1.drawing");
  EXPECT_TRUE(layout.HasPort("a"));
  std::vector<Rectangle*> pins = RectanglesOn(layout, "met1.drawing");
  ASSERT_EQ(1, pins.size());
  EXPECT_EQ(Point(500, 500), pins.front()->centre());
}

TEST_F(LayoutSky130Test, MakeWire) {
  Layout layout(physical_db_);
  geometry::Polygon *wire = layout.MakeWire(
      {{0, 0}, {2000, 0}, {2000, 2000}},
      "met1.drawing",
      "li.drawing",   // Via down to li at the start.
      std::nullopt,
      false,
      false,
      "n",
      true);
  ASSERT_NE(nullptr, wire);
  EXPECT_EQ(physical_db_.GetLayer("met1.drawing"), wire->layer());
  EXPECT_EQ("n", wire->net());
  EXPECT_TRUE(wire->is_connectable());

  Rectangle bounds = wire->GetBoundingBox();
  EXPECT_LE(bounds.lower_left().x(), 0);
  EXPECT_LE(bounds.lower_left().y(), 0);
  EXPECT_GE(bounds.upper_right().x(), 2000);
  EXPECT_GE(bounds.upper_right().y(), 2000);

  std::vector<Rectangle*> vias = RectanglesOn(layout, "mcon.drawing");
  ASSERT_EQ(1, vias.size());
  EXPECT_EQ(Point(0, 0), vias.front()->centre());
}

TEST_F(LayoutSky130Test, MakeWire_PadOnly) {
  Layout layout(physical_db_);
  layout.MakeWire(
      {{0, 0}, {2000, 0}}, "met1.drawing", "li.drawing", "met2.drawing",
      true, true, std::nullopt, false);
  EXPECT_TRUE(RectanglesOn(layout, "mcon.drawing").empty());
  EXPECT_TRUE(RectanglesOn(layout, "via1.drawing").empty());
  EXPECT_EQ(1, PolygonCountOn(layout, "met1.drawing"));
}

TEST_F(LayoutSky130Test, MakeAlternatingWire) {
  Layout layout(physical_db_);
  layout.MakeAlternatingWire(
      {{0, 0}, {2000, 0}, {2000, 2000}, {4000, 2000}},
      "met1.drawing", "met2.drawing", "n", true, false, false);

  // Hops alternate met1, met2, met1, with vias at the two interior points.
  EXPECT_EQ(2, PolygonCountOn(layout, "met1.drawing"));
  EXPECT_EQ(1, PolygonCountOn(layout, "met2.drawing"));
  std::vector<Rectangle*> vias = RectanglesOn(layout, "via1.drawing");
  ASSERT_EQ(2, vias.size());
  EXPECT_EQ(Point(2000, 0), vias.at(0)->centre());
  EXPECT_EQ(Point(2000, 2000), vias.at(1)->centre());
}

TEST_F(LayoutSky130Test, MakeAlternatingWire_RepeatedPointMakesPour) {
  Layout layout(physical_db_);
  layout.MakeAlternatingWire(
      {{0, 0}, {0, 0}}, "met1.drawing", "met2.drawing", "n", true, false,
      false);
  std::vector<Rectangle*> pours = RectanglesOn(layout, "met1.drawing");
  ASSERT_EQ(1, pours.size());
  EXPECT_EQ(Point(0, 0), pours.front()->centre());
  EXPECT_GE(pours.front()->Area(),
            physical_db_.Rules("met1.drawing").min_area);
}

TEST_F(LayoutSky130Test, MakeAlternatingWire_TooFewPoints) {
  Layout layout(physical_db_);
  layout.MakeAlternatingWire(
      {{0, 0}}, "met1.drawing", "met2.drawing", "n", true, false, false);
  EXPECT_EQ(Rectangle({0, 0}, {0, 0}), layout.GetBoundingBox());
}

TEST_F(LayoutSky130Test, MakeVerticalSpineWithFingers) {
  Layout layout(physical_db_);
  int64_t spine_x = 1000;
  geometry::Group created = layout.MakeVerticalSpineWithFingers(
      "met2.drawing",
      "met1.drawing",
      "n",
      // One connection on the spine, one off it, and a duplicate y further
      // away that should be dropped.
      {{spine_x, 0}, {3000, 2000}, {5000, 2000}},
      std::nullopt,
      spine_x,
      physical_db_.Rules("met2.drawing").min_width,
      &layout);

  EXPECT_EQ(1, PolygonCountOn(layout, "met2.drawing"));
  // One finger, to the closer of the two points at y = 2000.
  EXPECT_EQ(1, PolygonCountOn(layout, "met1.drawing"));
  ShapeCollection *met1 = layout.GetShapeCollection(
      physical_db_.GetLayer("met1.drawing"));
  Rectangle finger_bounds = met1->polygons().front()->GetBoundingBox();
  EXPECT_LT(finger_bounds.upper_right().x(), 5000);

  std::vector<Rectangle*> vias = RectanglesOn(layout, "via1.drawing");
  ASSERT_EQ(2, vias.size());
  std::set<Point, decltype(&Point::CompareXThenY)> centres(
      &Point::CompareXThenY);
  for (Rectangle *via : vias) {
    centres.insert(via->centre());
    EXPECT_EQ("n", via->net());
  }
  EXPECT_EQ(1, centres.count({spine_x, 0}));
  EXPECT_EQ(1, centres.count({spine_x, 2000}));

  EXPECT_FALSE(created.polygons().empty());
}

TEST_F(LayoutSky130Test, MakeVerticalSpineWithFingers_TooFewPoints) {
  Layout layout(physical_db_);
  geometry::Group created = layout.MakeVerticalSpineWithFingers(
      "met2.drawing", "met1.drawing", "n", {{0, 0}, {500, 0}}, std::nullopt,
      0, 0, &layout);
  EXPECT_TRUE(created.rectangles().empty());
  EXPECT_TRUE(created.polygons().empty());
  EXPECT_EQ(0, PolygonCountOn(layout, "met2.drawing"));
}

}  // namespace
}  // namespace bfg
