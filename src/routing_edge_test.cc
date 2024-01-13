#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "routing_edge.h"
#include "routing_vertex.h"
#include "geometry/point.h"

namespace bfg {
namespace {

using testing::ElementsAre;

TEST(RoutingEdge, AsRectangle_Vertical) {
  RoutingVertex first = RoutingVertex({10, 10});
  RoutingVertex second = RoutingVertex({10, 20});
  RoutingEdge edge = RoutingEdge(&first, &second);

  auto as_rectangle = edge.AsRectangle(10);
  ASSERT_NE(std::nullopt, as_rectangle);

  geometry::Point ll = {5, 10};
  EXPECT_EQ(ll, as_rectangle.value().lower_left());

  geometry::Point ur = {15, 20};
  EXPECT_EQ(ur, as_rectangle.value().upper_right());
}

TEST(RoutingEdge, AsRectangle_Vertical_SwappedEnds) {
  RoutingVertex first = RoutingVertex({10, 10});
  RoutingVertex second = RoutingVertex({10, 20});
  RoutingEdge edge = RoutingEdge(&second, &first);

  auto as_rectangle = edge.AsRectangle(10);
  ASSERT_NE(std::nullopt, as_rectangle);

  geometry::Point ll = {5, 10};
  EXPECT_EQ(ll, as_rectangle.value().lower_left());

  geometry::Point ur = {15, 20};
  EXPECT_EQ(ur, as_rectangle.value().upper_right());
}

TEST(RoutingEdge, AsRectangle_Horizontal) {
  RoutingVertex first = RoutingVertex({10, 10});
  RoutingVertex second = RoutingVertex({20, 10});
  RoutingEdge edge = RoutingEdge(&first, &second);

  auto as_rectangle = edge.AsRectangle(10);
  ASSERT_NE(std::nullopt, as_rectangle);

  geometry::Point ll = {10, 5};
  EXPECT_EQ(ll, as_rectangle.value().lower_left());

  geometry::Point ur = {20, 15};
  EXPECT_EQ(ur, as_rectangle.value().upper_right());
}

TEST(RoutingEdge, AsRectangle_Horizontal_SwappedEnds) {
  RoutingVertex first = RoutingVertex({10, 10});
  RoutingVertex second = RoutingVertex({20, 10});
  RoutingEdge edge = RoutingEdge(&second, &first);

  auto as_rectangle = edge.AsRectangle(10);
  ASSERT_NE(std::nullopt, as_rectangle);

  geometry::Point ll = {10, 5};
  EXPECT_EQ(ll, as_rectangle.value().lower_left());

  geometry::Point ur = {20, 15};
  EXPECT_EQ(ur, as_rectangle.value().upper_right());
}

TEST(RoutingEdge, AsRectangle_NotRectilinear) {
  RoutingVertex first = RoutingVertex({10, 10});
  RoutingVertex second = RoutingVertex({20, 20});
  RoutingEdge edge = RoutingEdge(&first, &second);

  auto as_rectangle = edge.AsRectangle(10);
  EXPECT_EQ(std::nullopt, as_rectangle);
}

}  // namespace
}  // namespace bfg
