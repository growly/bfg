#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "instance.h"
#include "port.h"
#include "shape_collection.h"
#include "../equivalent_nets.h"
#include "../layout.h"
#include "../physical_properties_database.h"

namespace bfg {
namespace geometry {
namespace {

TEST(Instance, BoundingBoxOnRotation) {
  bfg::Layout template_layout = bfg::Layout(PhysicalPropertiesDatabase());
  template_layout.AddRectangle(Rectangle(Point(1, 0), Point(2, 1)));

  Instance instance(&template_layout, Point(0, 0));
  instance.set_rotation_degrees_ccw(270);

  Rectangle bounding_box = instance.GetBoundingBox();

  //        1
  //      +--+ (2, 1)
  //    1 |  |
  //  O   +--+
  //    (1, 0)
  //
  //      | rotation 90 clockwise:
  //   <_/
  //
  //  O
  //
  //  +--+ (1, -1)
  //  |  |
  //  +--+
  //  (0, -2)

  EXPECT_TRUE(Rectangle(Point(0, -2), Point(1, -1)) == bounding_box);
}

// Layouts keep a reference to their PhysicalPropertiesDatabase, so it has to
// outlive them.
class InstanceTest : public testing::Test {
 protected:
  void SetUp() override {
    template_.reset(new bfg::Layout(physical_db_));
  }

  // The single port named "x" on the instance.
  Port *OnlyPort(Instance *instance) {
    std::vector<Port*> ports;
    instance->GetInstancePorts("x", &ports);
    EXPECT_EQ(1, ports.size());
    return ports.empty() ? nullptr : ports.front();
  }

  PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<bfg::Layout> template_;
};

TEST_F(InstanceTest, Ports) {
  template_->AddPort(Port({0, 0}, {2, 2}, 0, "x"));

  Instance instance(template_.get(), {10, 5});
  instance.set_name("i");
  EXPECT_TRUE(instance.HasPort("x"));
  EXPECT_FALSE(instance.HasPort("y"));

  Port *port = OnlyPort(&instance);
  ASSERT_NE(nullptr, port);
  EXPECT_EQ("i.x", port->net());
  EXPECT_EQ(Rectangle({10, 5}, {12, 7}), *port);

  std::vector<Port*> missing;
  instance.GetInstancePorts("y", &missing);
  EXPECT_TRUE(missing.empty());
}

TEST_F(InstanceTest, Ports_RegeneratedAfterMove) {
  template_->AddPort(Port({0, 0}, {2, 2}, 0, "x"));
  Instance instance(template_.get(), {0, 0});
  ASSERT_NE(nullptr, OnlyPort(&instance));

  instance.set_lower_left({100, 0});
  Port *port = OnlyPort(&instance);
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Point(100, 0), port->lower_left());
}

TEST_F(InstanceTest, Ports_Rotated180) {
  template_->AddPort(Port({1, 0}, {2, 1}, 0, "x"));
  Instance instance(template_.get(), {0, 0});
  instance.set_rotation_degrees_ccw(180);

  Port *port = OnlyPort(&instance);
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Rectangle({-2, -1}, {-1, 0}), *port);
}

// TODO(aryap): GeneratePorts converts the rotation to clockwise before calling
// BoundingBoxIfRotated, which takes counter-clockwise degrees, so ports on
// instances rotated by 90 or 270 degrees turn the opposite way to the
// instance's bounding box and copied shapes. (The VLSIR export also passes the
// ccw value as rotation_clockwise_degrees; see the FIXME in Layout.)
TEST_F(InstanceTest, DISABLED_Ports_Rotated90MatchBoundingBox) {
  template_->AddPort(Port({1, 0}, {2, 1}, 0, "x"));
  Instance instance(template_.get(), {0, 0});
  instance.set_rotation_degrees_ccw(90);

  Port *port = OnlyPort(&instance);
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(instance.GetBoundingBox(), *port);
}

// TODO(aryap): GeneratePorts, CopyAllShapes and GetPoint ignore
// reflect_vertical_, while GetBoundingBox and Layout::Flatten apply it.
TEST_F(InstanceTest, DISABLED_Ports_ReflectedMatchBoundingBox) {
  template_->AddPort(Port({1, 2}, {3, 5}, 0, "x"));
  Instance instance(template_.get(), {0, 0});
  instance.set_reflect_vertical(true);

  Port *port = OnlyPort(&instance);
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(instance.GetBoundingBox(), *port);
}

TEST_F(InstanceTest, DISABLED_CopyAllShapes_ReflectedMatchesBoundingBox) {
  template_->AddRectangle(Rectangle({1, 2}, {3, 5}));
  Instance instance(template_.get(), {0, 0});
  instance.set_reflect_vertical(true);

  ShapeCollection shapes;
  instance.CopyAllShapes(&shapes);
  ASSERT_EQ(1, shapes.rectangles().size());
  EXPECT_EQ(instance.GetBoundingBox(), *shapes.rectangles().front());
}

TEST_F(InstanceTest, DISABLED_GetPoint_Reflected) {
  template_->SavePoint("p", {1, 2});
  Instance instance(template_.get(), {0, 0});
  instance.set_reflect_vertical(true);
  EXPECT_EQ(Point(1, -2), instance.GetPointOrDie("p"));
}

TEST_F(InstanceTest, CopyAllShapes) {
  template_->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "a"));
  template_->AddRectangle(Rectangle({2, 2}, {3, 3}, 0, "vdd"));
  Instance instance(template_.get(), {10, 20});
  instance.set_name("i");

  ShapeCollection shapes;
  instance.CopyAllShapes(&shapes, std::nullopt, std::set<std::string>{"vdd"});
  ASSERT_EQ(2, shapes.rectangles().size());
  EXPECT_EQ(Rectangle({10, 20}, {11, 21}), *shapes.rectangles().at(0));
  EXPECT_EQ("i.a", shapes.rectangles().at(0)->net());
  EXPECT_EQ("vdd", shapes.rectangles().at(1)->net());
}

TEST_F(InstanceTest, CopyAllShapes_Empty) {
  Instance instance(template_.get(), {10, 20});
  ShapeCollection shapes;
  instance.CopyAllShapes(&shapes);
  EXPECT_TRUE(shapes.Empty());
}

TEST_F(InstanceTest, CopyAllShapes_RotatedMatchesBoundingBox) {
  template_->AddRectangle(Rectangle({1, 0}, {2, 1}));
  Instance instance(template_.get(), {5, 5});
  instance.set_rotation_degrees_ccw(90);

  ShapeCollection shapes;
  instance.CopyAllShapes(&shapes);
  ASSERT_EQ(1, shapes.rectangles().size());
  EXPECT_EQ(instance.GetBoundingBox(), *shapes.rectangles().front());
}

TEST_F(InstanceTest, CopyConnectableShapes) {
  Rectangle *connectable =
      template_->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "a"));
  connectable->set_is_connectable(true);
  template_->AddRectangle(Rectangle({2, 2}, {3, 3}, 0, "b"));
  Rectangle *other = template_->AddRectangle(Rectangle({4, 4}, {5, 5}, 0, "c"));
  other->set_is_connectable(true);
  Instance instance(template_.get(), {0, 0});
  instance.set_name("i");

  ShapeCollection all_connectable;
  instance.CopyConnectableShapes(&all_connectable);
  ASSERT_EQ(2, all_connectable.rectangles().size());

  ShapeCollection on_nets;
  instance.CopyConnectableShapesOnNets(EquivalentNets("i.a"), &on_nets);
  ASSERT_EQ(1, on_nets.rectangles().size());
  EXPECT_EQ("i.a", on_nets.rectangles().front()->net());

  ShapeCollection not_on_nets;
  instance.CopyConnectableShapesNotOnNets(EquivalentNets("i.a"), &not_on_nets);
  ASSERT_EQ(1, not_on_nets.rectangles().size());
  EXPECT_EQ("i.c", not_on_nets.rectangles().front()->net());
}

TEST_F(InstanceTest, CopyShapesOnLayer) {
  template_->set_active_layer(1);
  template_->AddRectangle(Rectangle({0, 0}, {1, 1}, 1, "a"));
  template_->set_active_layer(2);
  template_->AddRectangle(Rectangle({0, 0}, {1, 1}, 2, "b"));
  Instance instance(template_.get(), {3, 3});
  instance.set_name("i");

  ShapeCollection shapes;
  instance.CopyShapesOnLayer(1, &shapes);
  ASSERT_EQ(1, shapes.rectangles().size());
  EXPECT_EQ("i.a", shapes.rectangles().front()->net());
  EXPECT_EQ(Point(3, 3), shapes.rectangles().front()->lower_left());
}

TEST_F(InstanceTest, CopyNonConnectableShapesOnLayer) {
  Rectangle *connectable =
      template_->AddRectangle(Rectangle({0, 0}, {1, 1}, 0, "a"));
  connectable->set_is_connectable(true);
  template_->AddRectangle(Rectangle({2, 2}, {3, 3}, 0, "b"));
  Instance instance(template_.get(), {0, 0});
  instance.set_name("i");

  ShapeCollection shapes;
  instance.CopyNonConnectableShapesOnLayer(0, &shapes);
  ASSERT_EQ(1, shapes.rectangles().size());
  EXPECT_EQ("i.b", shapes.rectangles().front()->net());
}

TEST_F(InstanceTest, GetPoint) {
  template_->SavePoint("p", {1, 0});
  Instance instance(template_.get(), {10, 10});
  EXPECT_EQ(Point(11, 10), instance.GetPointOrDie("p"));
  EXPECT_FALSE(instance.GetPoint("missing"));

  // Rotations are counter-clockwise, as for the bounding box.
  instance.set_rotation_degrees_ccw(90);
  EXPECT_EQ(Point(10, 11), instance.GetPointOrDie("p"));
}

TEST_F(InstanceTest, GetTilingBounds) {
  template_->AddRectangle(Rectangle({0, 0}, {1, 1}));
  template_->SetTilingBounds(Rectangle({-1, -1}, {3, 3}));
  Instance instance(template_.get(), {10, 10});
  EXPECT_EQ(Rectangle({9, 9}, {13, 13}), instance.GetTilingBounds());
  EXPECT_EQ(Point(9, 9), instance.TilingLowerLeft());
  EXPECT_EQ(4, instance.TilingWidth());
  EXPECT_EQ(4, instance.TilingHeight());

  instance.MoveTilingLowerLeft({0, 0});
  EXPECT_EQ(Point(1, 1), instance.lower_left());
  EXPECT_EQ(Rectangle({0, 0}, {4, 4}), instance.GetTilingBounds());
}

class InstancePortSearchTest : public InstanceTest {
 protected:
  void SetUp() override {
    InstanceTest::SetUp();
    template_->AddPort(Port({0, 0}, {2, 2}, 0, "x"));
    template_->AddPort(Port({10, 0}, {12, 2}, 0, "x"));
    template_->AddPort(Port({20, 0}, {22, 2}, 0, "x"));
    instance_.reset(new Instance(template_.get(), {0, 0}));
  }

  std::unique_ptr<Instance> instance_;
};

TEST_F(InstancePortSearchTest, GetNearestPortNamed) {
  Port *port = instance_->GetNearestPortNamed(Point(9, 0), "x");
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Point(10, 0), port->lower_left());

  Port to_port({18, 0}, {20, 2}, 0, "");
  port = instance_->GetNearestPortNamed(to_port, "x");
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Point(20, 0), port->lower_left());

  EXPECT_EQ(nullptr, instance_->GetNearestPortNamed(Point(0, 0), "y"));
}

TEST_F(InstancePortSearchTest, GetFurthestPortNamed) {
  Port *port = instance_->GetFurthestPortNamed(Point(9, 0), "x");
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Point(20, 0), port->lower_left());

  EXPECT_EQ(nullptr, instance_->GetFurthestPortNamed(Point(0, 0), "y"));
}

TEST_F(InstancePortSearchTest, GetFirstPortNamed) {
  Port *port = instance_->GetFirstPortNamed("x");
  ASSERT_NE(nullptr, port);
  // Ports are ordered by position.
  EXPECT_EQ(Point(0, 0), port->lower_left());
  EXPECT_EQ(nullptr, instance_->GetFirstPortNamed("y"));
}

TEST_F(InstancePortSearchTest, GetMidwayPortNamed) {
  Port *port = instance_->GetMidwayPortNamed(Point(0, 0), "x");
  ASSERT_NE(nullptr, port);
  EXPECT_EQ(Point(10, 0), port->lower_left());

  EXPECT_EQ(nullptr, instance_->GetMidwayPortNamed(Point(0, 0), "y"));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
