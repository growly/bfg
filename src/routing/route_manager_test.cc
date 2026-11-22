#include "route_manager.h"

#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../equivalent_nets.h"
#include "../design_database.h"
#include "../physical_properties_database.h"
#include "routing_grid.h"
#include "../geometry/port.h"
#include "../geometry/point.h"
#include "../dev_pdk_setup.h"
#include "../layout.h"
#include "routing_grid_test_fixture.h"

namespace bfg {
namespace routing {
namespace {

class RouteManagerTest : public testing::Test {
 protected:
  void SetUp() override {
    layout_.reset(new Layout(physical_db_));
    grid_.reset(new RoutingGrid(physical_db_));
    route_manager_.reset(new RouteManager(layout_.get(), grid_.get()));
  }

  bfg::PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<Layout> layout_;
  std::unique_ptr<RoutingGrid> grid_;
  std::unique_ptr<RouteManager> route_manager_;
};

}  // namespace

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

// RouteManager on a real met1/met2 grid.
class RouteManagerRoutingTest : public RoutingGridTest {
 protected:
  void SetUp() override {
    RoutingGridTest::SetUp();
    layout_.reset(new Layout(physical_db_));
    route_manager_.reset(new RouteManager(layout_.get(), grid_.get()));
  }

  geometry::Port *AddPort(const geometry::Point &near, const std::string &net) {
    ports_.push_back(MakePort(near, net));
    return ports_.back().get();
  }

  std::unique_ptr<Layout> layout_;
  std::unique_ptr<RouteManager> route_manager_;
  std::vector<std::unique_ptr<geometry::Port>> ports_;
};

namespace {

TEST(RouteManagerStaticTest, SummariseStatuses_AllOk) {
  EXPECT_TRUE(RouteManager::SummariseStatuses({}).ok());
  EXPECT_TRUE(RouteManager::SummariseStatuses(
      {absl::OkStatus(), absl::OkStatus()}).ok());
}

TEST(RouteManagerStaticTest, SummariseStatuses_SomeFailed) {
  absl::Status summary = RouteManager::SummariseStatuses({
      absl::OkStatus(),
      absl::NotFoundError("no path"),
      absl::InternalError("broken")});
  EXPECT_FALSE(summary.ok());
  EXPECT_NE(std::string::npos, summary.message().find("no path"));
  EXPECT_NE(std::string::npos, summary.message().find("broken"));
}

TEST_F(RouteManagerRoutingTest, ConnectAndSolve) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  geometry::Port *a1 = AddPort({4000, 3000}, "a");

  auto id = route_manager_->Connect(*a0, *a1, EquivalentNets("a"));
  ASSERT_TRUE(id.ok());
  EXPECT_FALSE(route_manager_->DescribeOrders().empty());

  auto solved = route_manager_->SolveSerially();
  ASSERT_TRUE(solved.ok()) << solved.status();

  auto order_and_result = route_manager_->GetOrderAndResult(*id);
  ASSERT_TRUE(order_and_result.ok()) << order_and_result.status();
  ASSERT_TRUE(order_and_result->result.ok()) << order_and_result->result.status();
  EXPECT_EQ(1, order_and_result->result->size());
  EXPECT_EQ(1, grid_->paths().size());
  EXPECT_NE(std::string::npos,
            route_manager_->DescribeResults().find("OK"));
}

TEST_F(RouteManagerRoutingTest, Solve_ReturnsExecutedOrders) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  geometry::Port *a1 = AddPort({4000, 3000}, "a");
  ASSERT_TRUE(route_manager_->Connect(*a0, *a1, EquivalentNets("a")).ok());

  auto solved = route_manager_->SolveSerially();
  ASSERT_TRUE(solved.ok()) << solved.status();
  ASSERT_EQ(1, solved->size());
  EXPECT_EQ(2, solved->front().nodes().size());

  // Orders are consumed by solving.
  EXPECT_TRUE(route_manager_->DescribeOrders().empty());
}

TEST_F(RouteManagerRoutingTest, ConnectMultiplePorts) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  geometry::Port *a1 = AddPort({4500, 500}, "a");
  geometry::Port *a2 = AddPort({2500, 4500}, "a");

  auto id = route_manager_->ConnectMultiplePorts(
      std::vector<geometry::Port*>{a0, a1, a2}, EquivalentNets("a"));
  ASSERT_TRUE(id.ok());
  ASSERT_TRUE(route_manager_->SolveSerially().ok());

  auto order_and_result = route_manager_->GetOrderAndResult(*id);
  ASSERT_TRUE(order_and_result.ok());
  ASSERT_TRUE(order_and_result->result.ok()) << order_and_result->result.status();
  // Three ports need two paths.
  EXPECT_EQ(2, order_and_result->result->size());
}

TEST_F(RouteManagerRoutingTest, ConnectMultiplePorts_Sets) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  geometry::Port *a0_alt = AddPort({500, 1500}, "a");
  geometry::Port *a1 = AddPort({4500, 4500}, "a");

  auto id = route_manager_->ConnectMultiplePorts(
      std::vector<std::set<geometry::Port*>>{{a0, a0_alt}, {a1}},
      EquivalentNets("a"));
  ASSERT_TRUE(id.ok());
  ASSERT_TRUE(route_manager_->SolveSerially().ok());

  auto order_and_result = route_manager_->GetOrderAndResult(*id);
  ASSERT_TRUE(order_and_result.ok());
  ASSERT_TRUE(order_and_result->result.ok()) << order_and_result->result.status();
  // Only one port from each set needs connecting.
  EXPECT_EQ(1, order_and_result->result->size());
}

TEST_F(RouteManagerRoutingTest, ConnectToNet) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  geometry::Port *a1 = AddPort({4500, 500}, "a");
  geometry::Port *b0 = AddPort({2500, 4500}, "b");
  ASSERT_TRUE(route_manager_->Connect(*a0, *a1, EquivalentNets("a")).ok());
  ASSERT_TRUE(route_manager_->SolveSerially().ok());

  auto id = route_manager_->ConnectToNet(
      {{b0}}, EquivalentNets("a"), EquivalentNets("a"));
  ASSERT_TRUE(id.ok());
  ASSERT_TRUE(route_manager_->SolveSerially().ok());

  auto order_and_result = route_manager_->GetOrderAndResult(*id);
  ASSERT_TRUE(order_and_result.ok());
  ASSERT_TRUE(order_and_result->result.ok()) << order_and_result->result.status();
  EXPECT_EQ(2, grid_->paths().size());
}

TEST_F(RouteManagerRoutingTest, GetOrderAndResult_Unknown) {
  auto order_and_result = route_manager_->GetOrderAndResult(12345);
  EXPECT_EQ(absl::StatusCode::kNotFound, order_and_result.status().code());
}

TEST_F(RouteManagerRoutingTest, OrderWithOneNodeFails) {
  geometry::Port *a0 = AddPort({500, 500}, "a");
  auto id = route_manager_->ConnectMultiplePorts(
      std::vector<geometry::Port*>{a0}, EquivalentNets("a"));
  ASSERT_TRUE(id.ok());
  route_manager_->SolveSerially().IgnoreError();

  auto order_and_result = route_manager_->GetOrderAndResult(*id);
  ASSERT_TRUE(order_and_result.ok());
  EXPECT_FALSE(order_and_result->result.ok());
  EXPECT_NE(std::string::npos,
            route_manager_->DescribeResults().find("FAILED"));
}

TEST_F(RouteManagerRoutingTest, SolveInParallel) {
  geometry::Port *a0 = AddPort({0, 500}, "a");
  geometry::Port *a1 = AddPort({kSize, 500}, "a");
  geometry::Port *b0 = AddPort({0, 4500}, "b");
  geometry::Port *b1 = AddPort({kSize, 4500}, "b");
  auto a = route_manager_->Connect(*a0, *a1, EquivalentNets("a"));
  auto b = route_manager_->Connect(*b0, *b1, EquivalentNets("b"));
  ASSERT_TRUE(a.ok());
  ASSERT_TRUE(b.ok());

  ASSERT_TRUE(route_manager_->Solve().ok());
  for (int64_t id : {*a, *b}) {
    auto order_and_result = route_manager_->GetOrderAndResult(id);
    ASSERT_TRUE(order_and_result.ok());
    EXPECT_TRUE(order_and_result->result.ok())
        << order_and_result->result.status();
  }
  EXPECT_EQ(2, grid_->paths().size());
}

TEST(NetRouteOrderTest, Describe) {
  geometry::Port port({0, 0}, {1, 1}, 0, "p");
  NetRouteOrder order(7, EquivalentNets("n"));
  order.nodes().push_back({&port});
  std::string description = order.Describe();
  EXPECT_NE(std::string::npos, description.find("ID: 7"));
  EXPECT_NE(std::string::npos, description.find("Step 0"));
}

}  // namespace

}  // namespace routing
}  // namespace bfg
