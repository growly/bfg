#include "route_manager.h"

#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "equivalent_nets.h"
#include "design_database.h"
#include "physical_properties_database.h"
#include "routing_grid.h"
#include "geometry/port.h"
#include "geometry/point.h"
#include "dev_pdk_setup.h"

namespace bfg {
namespace {

class RouteManagerTest : public testing::Test {
 protected:
  void SetUp() override {
    layout_.reset(new Layout(physical_db_));
    grid_.reset(new RoutingGrid(physical_db_));
    route_manager_.reset(new RouteManager(layout_.get(), grid_.get()));
  }

  PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<Layout> layout_;
  std::unique_ptr<RoutingGrid> grid_;
  std::unique_ptr<RouteManager> route_manager_;
};

}   // namespace

// These fixtures are not nested in an anonymous namespace to enable them to be
// a friend for private access.

TEST_F(RouteManagerTest, ConsolidateOrders) {
  std::unique_ptr<geometry::Port> p1(
      new geometry::Port({0, 0}, 10, 10, 0, "p1"));
  std::unique_ptr<geometry::Port> p2(
      new geometry::Port({0, 0}, 10, 10, 0, "p2"));
  std::unique_ptr<geometry::Port> p3(
      new geometry::Port({0, 0}, 10, 10, 0, "p3"));
  std::unique_ptr<geometry::Port> p4(
      new geometry::Port({0, 0}, 10, 10, 0, "p4"));
  std::unique_ptr<geometry::Port> p5(
      new geometry::Port({0, 0}, 10, 10, 0, "p5"));
  std::unique_ptr<geometry::Port> p6(
      new geometry::Port({0, 0}, 10, 10, 0, "p6"));

  // Diagrammatically (ignore their coordinates):
  //   p1 x<------->x<----+
  //          (1)   p2    |
  //                      | (4)
  //   p3 x         x p4  |
  //      ^         ^     |
  //      | (2)     | (3) |
  //      v         v     |
  //   p5 x         x <---+
  //                p6
  route_manager_->Connect(*p1, *p2).IgnoreError();
  route_manager_->Connect(*p3, *p5).IgnoreError();
  route_manager_->Connect(*p4, *p6).IgnoreError();
  route_manager_->Connect(*p2, *p6).IgnoreError();

  EXPECT_EQ(4, route_manager_->orders_.size());

  route_manager_->ConsolidateOrders().IgnoreError();

  EXPECT_EQ(2, route_manager_->orders_.size());

  // The order matters! Given the order in which we specified the pairs, we
  // expect this order of NetRouteOrders:
  // 
  // order 0: p1, p2, p4, p6
  std::vector<const geometry::Port*> expected_order_0 = {
      p1.get(), p2.get(), p4.get(), p6.get()};
  // order 1: p3, p5
  std::vector<const geometry::Port*> expected_order_1 = {
      p3.get(), p5.get()};
 
  size_t i = 0;
  const auto &consolidated_0 = route_manager_->orders_[0].nodes();
  ASSERT_EQ(4, consolidated_0.size());
  for (const auto &node : consolidated_0) {
    EXPECT_EQ(1, node.size());
    EXPECT_EQ(expected_order_0[i], *node.begin());
    ++i;
  }

  i = 0;
  const auto &consolidated_1 = route_manager_->orders_[1].nodes();
  ASSERT_EQ(2, consolidated_1.size());
  for (const auto &node : consolidated_1) {
    EXPECT_EQ(1, node.size());
    EXPECT_EQ(expected_order_1[i], *node.begin());
    ++i;
  }

  //for (const auto &order : route_manager_->orders_) {
  //  std::cout << order.Describe();
  //}
}

TEST_F(RouteManagerTest, MergeAndReplaceEquivalentNets) {
  EquivalentNets *a = route_manager_->routed_nets_.emplace_back(
      new EquivalentNets("a")).get();
  EquivalentNets *b = route_manager_->routed_nets_.emplace_back(
      new EquivalentNets("b")).get();
  EquivalentNets *c = route_manager_->routed_nets_.emplace_back(
      new EquivalentNets("c")).get();
  EquivalentNets *d = route_manager_->routed_nets_.emplace_back(
      new EquivalentNets("d")).get();

  ASSERT_EQ(4, route_manager_->routed_nets_.size());

  std::unique_ptr<geometry::Port> p1(
      new geometry::Port({0, 0}, 10, 10, 0, "p1"));
  std::unique_ptr<geometry::Port> p2(
      new geometry::Port({0, 0}, 10, 10, 0, "p2"));
  std::unique_ptr<geometry::Port> p3(
      new geometry::Port({0, 0}, 10, 10, 0, "p3"));
  std::unique_ptr<geometry::Port> p4(
      new geometry::Port({0, 0}, 10, 10, 0, "p4"));

  route_manager_->routed_nets_by_port_[p1.get()] = a;
  route_manager_->routed_nets_by_port_[p2.get()] = b;
  route_manager_->routed_nets_by_port_[p3.get()] = c;
  route_manager_->routed_nets_by_port_[p4.get()] = d;

  ASSERT_EQ(4, route_manager_->routed_nets_by_port_.size());

  // Merge EquivalentNets a, b and d together. Everything that used to point to
  // a or b must now point to d.
  route_manager_->MergeAndReplaceEquivalentNets({a, c}, d);

  ASSERT_EQ(2, route_manager_->routed_nets_.size());

  std::vector<EquivalentNets*> expected_routed_nets = {b, d};
  std::vector<EquivalentNets*> actual_routed_nets;
  for (const auto &uniq : route_manager_->routed_nets_) {
    actual_routed_nets.push_back(uniq.get());
  }
  EXPECT_THAT(actual_routed_nets, testing::ContainerEq(expected_routed_nets));
  EXPECT_EQ(d, route_manager_->routed_nets_by_port_[p1.get()]);
  EXPECT_EQ(b, route_manager_->routed_nets_by_port_[p2.get()]);
  EXPECT_EQ(d, route_manager_->routed_nets_by_port_[p3.get()]);
  EXPECT_EQ(d, route_manager_->routed_nets_by_port_[p4.get()]);
}

}   // namespace bfg
