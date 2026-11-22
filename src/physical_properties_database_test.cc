#include "physical_properties_database.h"

#include <gtest/gtest.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "dev_pdk_setup.h"
#include "geometry/layer.h"
#include "routing/routing_layer_info.h"
#include "routing/routing_via_info.h"

namespace bfg {
namespace {

using geometry::Layer;
using routing::CostedLayer;
using routing::RoutingViaInfo;

// A hand-built via graph for FindViaStackImpl. Each edge's RoutingViaInfo has
// its layer set to 100 * lesser + greater so that stacks can be compared.
class ViaGraph {
 public:
  void Connect(Layer a, Layer b, double cost) {
    edges_[a].push_back({b, cost});
    edges_[b].push_back({a, cost});
  }

  std::optional<std::vector<RoutingViaInfo>> FindStack(Layer from, Layer to) {
    return PhysicalPropertiesDatabase::FindViaStackImpl(
        from, to,
        [&](const Layer &layer) {
          auto it = edges_.find(layer);
          return it == edges_.end() ? std::vector<CostedLayer>() : it->second;
        },
        [&](const Layer &lhs, const Layer &rhs) {
          RoutingViaInfo info;
          info.set_layer(100 * std::min(lhs, rhs) + std::max(lhs, rhs));
          return info;
        });
  }

  static std::vector<Layer> Ids(const std::vector<RoutingViaInfo> &stack) {
    std::vector<Layer> ids;
    for (const RoutingViaInfo &info : stack) {
      ids.push_back(info.layer());
    }
    return ids;
  }

 private:
  std::map<Layer, std::vector<CostedLayer>> edges_;
};

TEST(FindViaStackImplTest, SameLayer) {
  ViaGraph graph;
  auto stack = graph.FindStack(1, 1);
  ASSERT_TRUE(stack);
  EXPECT_TRUE(stack->empty());
}

TEST(FindViaStackImplTest, Chain) {
  ViaGraph graph;
  graph.Connect(1, 2, 1.0);
  graph.Connect(2, 3, 1.0);
  graph.Connect(3, 4, 1.0);

  auto stack = graph.FindStack(1, 4);
  ASSERT_TRUE(stack);
  EXPECT_EQ((std::vector<Layer>{102, 203, 304}), ViaGraph::Ids(*stack));

  // The stack is always ordered from the lesser layer.
  auto reversed = graph.FindStack(4, 1);
  ASSERT_TRUE(reversed);
  EXPECT_EQ(ViaGraph::Ids(*stack), ViaGraph::Ids(*reversed));
}

TEST(FindViaStackImplTest, Disconnected) {
  ViaGraph graph;
  graph.Connect(1, 2, 1.0);
  graph.Connect(3, 4, 1.0);
  EXPECT_FALSE(graph.FindStack(1, 4));
}

TEST(FindViaStackImplTest, PicksCheapestRoute) {
  // 1 -- 2 -- 4 costs 2; 1 -- 3 -- 4 costs 10.
  ViaGraph graph;
  graph.Connect(1, 2, 1.0);
  graph.Connect(2, 4, 1.0);
  graph.Connect(1, 3, 5.0);
  graph.Connect(3, 4, 5.0);

  auto stack = graph.FindStack(1, 4);
  ASSERT_TRUE(stack);
  EXPECT_EQ((std::vector<Layer>{102, 204}), ViaGraph::Ids(*stack));
}

TEST(FindViaStackImplTest, CheaperRouteFoundLater) {
  // The direct 1 -- 3 via is expensive; going through 2 is cheaper.
  ViaGraph graph;
  graph.Connect(1, 2, 1.0);
  graph.Connect(1, 3, 5.0);
  graph.Connect(2, 3, 1.0);
  graph.Connect(3, 4, 1.0);

  auto stack = graph.FindStack(1, 4);
  ASSERT_TRUE(stack);
  EXPECT_EQ((std::vector<Layer>{102, 203, 304}), ViaGraph::Ids(*stack));
}

TEST(FindViaStackImplTest, Branching) {
  // Layer 2 has two onward neighbours, so the queue holds more than one layer
  // at a time.
  ViaGraph graph;
  graph.Connect(1, 2, 1.0);
  graph.Connect(2, 3, 1.0);
  graph.Connect(2, 5, 2.0);
  graph.Connect(3, 4, 1.0);

  auto stack = graph.FindStack(1, 4);
  ASSERT_TRUE(stack);
  EXPECT_EQ((std::vector<Layer>{102, 203, 304}), ViaGraph::Ids(*stack));
}

class PhysicalPropertiesDatabaseTest : public testing::Test {
 protected:
  void SetUp() override {
    db_.LoadTechnologyFromFile("test_data/sky130.technology.pb");
    bfg::SetUpSky130(&db_);
  }

  Layer L(const std::string &name) const { return db_.GetLayer(name); }

  PhysicalPropertiesDatabase db_;
};

TEST_F(PhysicalPropertiesDatabaseTest, GetLayerName) {
  EXPECT_EQ("met1.drawing", db_.GetLayerName(L("met1.drawing")));
  EXPECT_FALSE(db_.GetLayerName(-12345));
}

TEST_F(PhysicalPropertiesDatabaseTest, FindLayersReachableThroughOneVia) {
  EXPECT_EQ((std::set<Layer>{L("li.drawing"), L("met2.drawing")}),
            db_.FindLayersReachableThroughOneVia(L("met1.drawing")));
  EXPECT_EQ((std::set<Layer>{L("met1.drawing"), L("met3.drawing")}),
            db_.FindLayersReachableThroughOneVia(L("met2.drawing")));
}

TEST_F(PhysicalPropertiesDatabaseTest, FindViaStack) {
  auto stack = db_.FindViaStack(L("met1.drawing"), L("met2.drawing"));
  ASSERT_TRUE(stack);
  ASSERT_EQ(1, stack->size());
  EXPECT_EQ(L("via1.drawing"), stack->front().layer());

  stack = db_.FindViaStack(L("li.drawing"), L("met3.drawing"));
  ASSERT_TRUE(stack);
  std::vector<Layer> vias;
  for (const RoutingViaInfo &info : *stack) {
    vias.push_back(info.layer());
  }
  EXPECT_EQ((std::vector<Layer>{
                L("mcon.drawing"), L("via1.drawing"), L("via2.drawing")}),
            vias);

  stack = db_.FindViaStack(L("met1.drawing"), L("met1.drawing"));
  ASSERT_TRUE(stack);
  EXPECT_TRUE(stack->empty());
}

TEST_F(PhysicalPropertiesDatabaseTest, TypicalViaEncap) {
  ViaEncapInfo encap = db_.TypicalViaEncap("met1.drawing", "via1.drawing");
  int64_t via_side = std::max(db_.Rules("via1.drawing").via_width,
                              db_.Rules("via1.drawing").via_height);
  EXPECT_GE(encap.length, encap.width);
  EXPECT_GE(encap.width, via_side);

  // Between two via layers, the encap must cover both.
  ViaEncapInfo both = db_.TypicalViaEncap(
      "mcon.drawing", "met1.drawing", "via1.drawing");
  ViaEncapInfo bottom = db_.TypicalViaEncap("met1.drawing", "mcon.drawing");
  EXPECT_EQ(std::max(encap.length, bottom.length), both.length);
  EXPECT_EQ(std::max(encap.width, bottom.width), both.width);
}

TEST_F(PhysicalPropertiesDatabaseTest, GetRoutingViaInfo) {
  auto info = db_.GetRoutingViaInfo("met1.drawing", "met2.drawing");
  ASSERT_TRUE(info);
  EXPECT_EQ(L("via1.drawing"), info->layer());
  EXPECT_FALSE(db_.GetRoutingViaInfo("met1.drawing", "nonexistent.drawing"));
}

TEST_F(PhysicalPropertiesDatabaseTest, GetRoutingViaInfo_Unconnected) {
  EXPECT_FALSE(db_.GetRoutingViaInfo("met1.drawing", "met3.drawing"));
}

TEST_F(PhysicalPropertiesDatabaseTest, GetRoutingLayerInfo) {
  auto info = db_.GetRoutingLayerInfo("met1.drawing");
  ASSERT_TRUE(info);
  EXPECT_EQ(L("met1.drawing"), info->layer());
  EXPECT_EQ(db_.Rules("met1.drawing").min_pitch, info->pitch());
  EXPECT_FALSE(db_.GetRoutingLayerInfo("nonexistent.drawing"));
}

TEST_F(PhysicalPropertiesDatabaseTest,
       GetRoutingLayerInfoOrDie_DiesForUnknownLayer) {
  EXPECT_DEATH(db_.GetRoutingLayerInfoOrDie("nonexistent.drawing"),
               "No routing layer info");
}

TEST_F(PhysicalPropertiesDatabaseTest, PinLayers) {
  Layer pin = L("met1.pin");
  ASSERT_TRUE(db_.IsPinLayer(pin));
  EXPECT_FALSE(db_.IsPinLayer(L("met1.drawing")));

  // met1.pin gives access to met1, and through one via to li and met2.
  std::set<Layer> accessible = db_.GetAccessibleLayersForPin(pin);
  EXPECT_EQ(1, accessible.count(L("met1.drawing")));
  EXPECT_EQ(1, accessible.count(L("met2.drawing")));
  EXPECT_EQ(1, accessible.count(L("li.drawing")));

  EXPECT_EQ(1, db_.GetPinLayersFor(L("met1.drawing")).count(pin));
}

TEST_F(PhysicalPropertiesDatabaseTest, Describe) {
  EXPECT_NE(std::string::npos, db_.DescribeLayers().find("met1"));
  EXPECT_NE(std::string::npos,
            db_.DescribeLayer(L("met1.drawing")).find("met1"));
}

}  // namespace
}  // namespace bfg
