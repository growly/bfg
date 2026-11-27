#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "point.h"
#include "port.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {
namespace {

TEST(PortTest, ConstructorsMakeConnectablePorts) {
  Port from_points({0, 0}, {2, 2}, 3, "a");
  EXPECT_TRUE(from_points.is_connectable());
  EXPECT_EQ(3, from_points.layer());
  EXPECT_EQ("a", from_points.net());

  Port from_centre({10, 10}, 4, 6, 3, "b");
  EXPECT_TRUE(from_centre.is_connectable());
  EXPECT_EQ(Rectangle({8, 7}, {12, 13}, 3, ""), from_centre);

  Rectangle rectangle({1, 1}, {5, 5}, 7, "");
  Port from_rectangle(rectangle, "c");
  EXPECT_TRUE(from_rectangle.is_connectable());
  EXPECT_EQ(0, from_rectangle.layer());

  Port from_rectangle_on_layer(rectangle, 9, "d");
  EXPECT_EQ(9, from_rectangle_on_layer.layer());

  Port copy(from_points);
  EXPECT_TRUE(copy.is_connectable());
  EXPECT_EQ("a", copy.net());
  EXPECT_EQ(3, copy.layer());
  EXPECT_EQ(from_points, copy);
}

TEST(PortTest, CompareOrdersByXThenY) {
  Port left({0, 5}, {1, 6}, 0, "");
  Port right({1, 0}, {2, 1}, 0, "");
  Port above({0, 6}, {1, 7}, 0, "");

  EXPECT_TRUE(Port::Compare(left, right));
  EXPECT_FALSE(Port::Compare(right, left));
  EXPECT_TRUE(Port::Compare(left, above));
  EXPECT_FALSE(Port::Compare(left, left));
}

TEST(PortTest, CompareNull) {
  Port port({0, 0}, {1, 1}, 0, "");
  EXPECT_TRUE(Port::Compare(nullptr, &port));
  EXPECT_FALSE(Port::Compare(&port, nullptr));

  std::unique_ptr<Port> uniq = std::make_unique<Port>(port);
  std::unique_ptr<Port> empty;
  EXPECT_TRUE(Port::Compare(empty, uniq));
  EXPECT_FALSE(Port::Compare(uniq, empty));
}

TEST(PortTest, MakePortSetIsOrdered) {
  Port a({5, 0}, {6, 1}, 0, "a");
  Port b({0, 0}, {1, 1}, 0, "b");
  Port c({0, 3}, {1, 4}, 0, "c");

  PortSet ports = Port::MakePortSet({&a, &b, &c});
  std::vector<std::string> order;
  for (Port *port : ports) {
    order.push_back(port->net());
  }
  std::vector<std::string> expected = {"b", "c", "a"};
  EXPECT_EQ(expected, order);
}

TEST(PortTest, Describe) {
  Port port({0, 0}, {2, 4}, 0, "a");
  std::string description = port.Describe();
  EXPECT_EQ(0, description.find("[Port a "));

  std::stringstream ss;
  ss << port;
  EXPECT_EQ(description, ss.str());
}

TEST(PortTest, DescribePorts) {
  Port a({4, 0}, {6, 2}, 0, "a");
  Port b({0, 2}, {2, 4}, 0, "b");
  Port c({0, 0}, {2, 2}, 0, "c");

  // Vectors are described in order; sets sorted by centre x then y.
  EXPECT_EQ("(5, 1), (1, 3), (1, 1)", Port::DescribePorts(
      std::vector<Port*>{&a, &b, &c}));
  EXPECT_EQ("(1, 1), (1, 3), (5, 1)", Port::DescribePorts(
      Port::MakePortSet({&a, &b, &c})));
  EXPECT_EQ("", Port::DescribePorts(std::vector<Port*>{}));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
