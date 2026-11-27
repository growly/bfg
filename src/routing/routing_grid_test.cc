#include "routing_grid.h"
#include "routing_grid_test_fixture.h"

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../dev_pdk_setup.h"
#include "../equivalent_nets.h"
#include "../geometry/point.h"
#include "../geometry/port.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../layout.h"
#include "../physical_properties_database.h"
#include "routing_blockage_cache.h"
#include "routing_layer_info.h"
#include "routing_path.h"
#include "routing_track_direction.h"
#include "routing_vertex.h"
#include "routing_via_info.h"

namespace bfg {
namespace routing {

namespace {

TEST_F(RoutingGridTest, ConnectLayersMakesAVertexAtEveryCrossing) {
  ASSERT_GT(met1_pitch_, 0);
  ASSERT_GT(met2_pitch_, 0);
  ASSERT_FALSE(grid_->vertices().empty());

  std::set<int64_t> xs;
  std::set<int64_t> ys;
  for (RoutingVertex *vertex : grid_->vertices()) {
    xs.insert(vertex->centre().x());
    ys.insert(vertex->centre().y());
    EXPECT_TRUE(vertex->connected_layers().count(met1_));
    EXPECT_TRUE(vertex->connected_layers().count(met2_));
    EXPECT_GE(vertex->centre().x(), 0);
    EXPECT_LE(vertex->centre().x(), kSize);
  }
  EXPECT_EQ(xs.size() * ys.size(), grid_->vertices().size());

  // Vertices are spaced at the pitch of the layer whose tracks set them.
  std::vector<int64_t> sorted_xs(xs.begin(), xs.end());
  for (size_t i = 1; i < sorted_xs.size(); ++i) {
    EXPECT_EQ(met2_pitch_, sorted_xs[i] - sorted_xs[i - 1]);
  }
  std::vector<int64_t> sorted_ys(ys.begin(), ys.end());
  for (size_t i = 1; i < sorted_ys.size(); ++i) {
    EXPECT_EQ(met1_pitch_, sorted_ys[i] - sorted_ys[i - 1]);
  }
}

TEST_F(RoutingGridTest, FindViaStack) {
  auto stack = grid_->FindViaStack(met1_, met2_);
  ASSERT_TRUE(stack);
  EXPECT_EQ(1, stack->size());
  EXPECT_TRUE(grid_->FindViaStack(met2_, met1_));

  auto same_layer = grid_->FindViaStack(met1_, met1_);
  ASSERT_TRUE(same_layer);
  EXPECT_TRUE(same_layer->empty());

  auto cost = grid_->FindViaStackCost(met1_, met2_);
  ASSERT_TRUE(cost);
  EXPECT_DOUBLE_EQ(0.5, *cost);

  // No via info was given for li.
  geometry::Layer li = physical_db_.GetLayer("li.drawing");
  EXPECT_FALSE(grid_->FindViaStack(li, met2_));
  EXPECT_FALSE(grid_->FindViaStackCost(li, met2_));
}

TEST_F(RoutingGridTest, LayersReachableByVia) {
  std::vector<CostedLayer> reachable = LayersReachableByVia(met1_);
  ASSERT_EQ(1, reachable.size());
  EXPECT_EQ(met2_, reachable.front().layer);
  EXPECT_DOUBLE_EQ(0.5, reachable.front().cost);
}

TEST_F(RoutingGridTest, VerticesAreTooCloseForVias) {
  RoutingVertex *origin = NearestVertex({0, 0});
  RoutingVertex *next = NearestVertex({met2_pitch_, 0});
  RoutingVertex *far = NearestVertex({4 * met2_pitch_, 0});
  ASSERT_NE(origin, next);
  ASSERT_NE(origin, far);

  // Vias on adjacent tracks at minimum pitch can't both be placed.
  EXPECT_TRUE(VerticesAreTooCloseForVias(*origin, *next));
  EXPECT_FALSE(VerticesAreTooCloseForVias(*origin, *far));
}

TEST_F(RoutingGridTest, ShortestPath_StraightLine) {
  RoutingBlockageCache cache(*grid_);
  RoutingVertex *begin = NearestVertex({0, 2000});
  RoutingVertex *end = NearestVertex({kSize, 2000});
  ASSERT_EQ(begin->centre().y(), end->centre().y());

  auto path = ShortestPath(begin, end, cache);
  ASSERT_TRUE(path.ok()) << path.status();
  std::unique_ptr<RoutingPath> owned(*path);
  EXPECT_EQ(begin, owned->Begin());
  EXPECT_EQ(end, owned->End());
  // A straight horizontal run needs no vias, so it all stays on met1.
  for (RoutingEdge *edge : owned->edges()) {
    EXPECT_EQ(met1_, edge->EffectiveLayer());
  }
  EXPECT_GT(owned->Cost(), 0);
}

TEST_F(RoutingGridTest, ShortestPath_AroundCorner) {
  RoutingBlockageCache cache(*grid_);
  RoutingVertex *begin = NearestVertex({0, 0});
  RoutingVertex *end = NearestVertex({kSize, kSize});

  auto path = ShortestPath(begin, end, cache);
  ASSERT_TRUE(path.ok()) << path.status();
  std::unique_ptr<RoutingPath> owned(*path);
  std::set<geometry::Layer> layers;
  for (RoutingEdge *edge : owned->edges()) {
    layers.insert(edge->EffectiveLayer());
  }
  // Turning the corner needs both layers.
  EXPECT_EQ((std::set<geometry::Layer>{met1_, met2_}), layers);
}

TEST_F(RoutingGridTest, AddRouteBetween) {
  std::unique_ptr<geometry::Port> begin = MakePort({500, 500}, "a");
  std::unique_ptr<geometry::Port> end = MakePort({4000, 3000}, "a");
  RoutingBlockageCache cache(*grid_);

  auto path = grid_->AddRouteBetween(*begin, *end, cache, EquivalentNets("a"));
  ASSERT_TRUE(path.ok()) << path.status();
  ASSERT_EQ(1, grid_->paths().size());
  EXPECT_EQ(*path, grid_->paths().front());
  EXPECT_FALSE((*path)->Empty());
  EXPECT_TRUE((*path)->nets().Contains("a"));

  std::vector<geometry::Point> points;
  std::vector<geometry::Layer> layers;
  (*path)->ToPointsAndLayers(&points, &layers);
  ASSERT_FALSE(points.empty());
  std::set<geometry::Point, decltype(&geometry::Point::CompareXThenY)> ends(
      &geometry::Point::CompareXThenY);
  ends.insert(points.front());
  ends.insert(points.back());
  EXPECT_EQ(1, ends.count(begin->centre()));
  EXPECT_EQ(1, ends.count(end->centre()));
}

TEST_F(RoutingGridTest, AddRouteBetween_SecondNetAvoidsFirst) {
  std::unique_ptr<geometry::Port> a0 = MakePort({0, 2500}, "a");
  std::unique_ptr<geometry::Port> a1 = MakePort({kSize, 2500}, "a");
  std::unique_ptr<geometry::Port> b0 = MakePort({2500, 0}, "b");
  std::unique_ptr<geometry::Port> b1 = MakePort({2500, kSize}, "b");
  RoutingBlockageCache cache(*grid_);

  auto path_a = grid_->AddRouteBetween(*a0, *a1, cache, EquivalentNets("a"));
  ASSERT_TRUE(path_a.ok()) << path_a.status();
  auto path_b = grid_->AddRouteBetween(*b0, *b1, cache, EquivalentNets("b"));
  ASSERT_TRUE(path_b.ok()) << path_b.status();

  // The nets may cross at a vertex, but only on different layers.
  auto layers_at = [](const RoutingPath &path) {
    std::map<RoutingVertex*, std::set<geometry::Layer>> layers;
    for (RoutingEdge *edge : path.edges()) {
      layers[edge->first()].insert(edge->EffectiveLayer());
      layers[edge->second()].insert(edge->EffectiveLayer());
    }
    return layers;
  };
  auto a_layers = layers_at(**path_a);
  for (const auto &[vertex, b_layers] : layers_at(**path_b)) {
    auto it = a_layers.find(vertex);
    if (it == a_layers.end()) {
      continue;
    }
    for (const geometry::Layer &layer : b_layers) {
      EXPECT_EQ(0, it->second.count(layer))
          << "nets a and b both use layer " << layer << " at "
          << vertex->centre();
    }
  }
}

TEST_F(RoutingGridTest, AddRouteBetween_FailsWhenEndIsWalledIn) {
  std::unique_ptr<geometry::Port> begin = MakePort({500, 500}, "a");
  std::unique_ptr<geometry::Port> end = MakePort({2500, 2500}, "a");

  // Wall off everything around the end port on both layers, leaving a gap
  // around the port itself that is too small to route through.
  geometry::Point centre = end->centre();
  int64_t inner = 2 * std::max(met1_pitch_, met2_pitch_);
  int64_t outer = 4 * std::max(met1_pitch_, met2_pitch_);
  for (const geometry::Layer &layer : {met1_, met2_}) {
    for (const geometry::Rectangle &wall : {
        geometry::Rectangle(centre - geometry::Point(outer, outer),
                            centre + geometry::Point(outer, -inner)),
        geometry::Rectangle(centre - geometry::Point(outer, -inner),
                            centre + geometry::Point(outer, outer)),
        geometry::Rectangle(centre - geometry::Point(outer, inner),
                            centre + geometry::Point(-inner, inner)),
        geometry::Rectangle(centre - geometry::Point(-inner, inner),
                            centre + geometry::Point(outer, inner))}) {
      geometry::Rectangle on_layer = wall;
      on_layer.set_layer(layer);
      grid_->AddBlockage(on_layer);
    }
  }

  RoutingBlockageCache cache(*grid_);
  auto path = grid_->AddRouteBetween(*begin, *end, cache, EquivalentNets("a"));
  EXPECT_FALSE(path.ok());
  EXPECT_TRUE(grid_->paths().empty());
}

TEST_F(RoutingGridTest, AddBlockageMakesVerticesUnavailable) {
  RoutingVertex *vertex = NearestVertex({2500, 2500});
  ASSERT_TRUE(vertex->Available());

  geometry::Rectangle blockage = geometry::Rectangle::CentredAt(
      vertex->centre(), 10, 10);
  blockage.set_layer(met1_);
  std::set<RoutingVertex*> blocked;
  grid_->AddBlockage(blockage, 0, false, &blocked);
  EXPECT_EQ(1, blocked.count(vertex));
  EXPECT_FALSE(vertex->Available());
}

TEST_F(RoutingGridTest, AddRouteToNet) {
  std::unique_ptr<geometry::Port> a0 = MakePort({500, 500}, "a");
  std::unique_ptr<geometry::Port> a1 = MakePort({4500, 500}, "a");
  std::unique_ptr<geometry::Port> a2 = MakePort({2500, 4500}, "a");
  RoutingBlockageCache cache(*grid_);

  ASSERT_TRUE(grid_->AddRouteBetween(*a0, *a1, cache, EquivalentNets("a"))
                  .ok());
  // Connect the third port to anywhere on the existing net.
  auto path = grid_->AddRouteToNet(
      *a2, EquivalentNets("a"), EquivalentNets("a"), cache);
  ASSERT_TRUE(path.ok()) << path.status();
  EXPECT_EQ(2, grid_->paths().size());
}

TEST_F(RoutingGridTest, GenerateLayout) {
  std::unique_ptr<geometry::Port> begin = MakePort({500, 500}, "a");
  std::unique_ptr<geometry::Port> end = MakePort({4000, 3000}, "a");
  RoutingBlockageCache cache(*grid_);
  ASSERT_TRUE(
      grid_->AddRouteBetween(*begin, *end, cache, EquivalentNets("a")).ok());

  std::unique_ptr<Layout> layout(grid_->GenerateLayout());
  ASSERT_NE(nullptr, layout);
  geometry::ShapeCollection *met1 = layout->GetShapeCollection(met1_);
  geometry::ShapeCollection *met2 = layout->GetShapeCollection(met2_);
  ASSERT_NE(nullptr, met1);
  ASSERT_NE(nullptr, met2);
  EXPECT_FALSE(met1->polygons().empty());
  EXPECT_FALSE(met2->polygons().empty());
  geometry::ShapeCollection *vias = layout->GetShapeCollection(
      physical_db_.GetLayer("via1.drawing"));
  ASSERT_NE(nullptr, vias);
  EXPECT_FALSE(vias->rectangles().empty());

  // The route covers both ports.
  geometry::Rectangle bounds = layout->GetBoundingBox();
  EXPECT_TRUE(bounds.Intersects(begin->centre()));
  EXPECT_TRUE(bounds.Intersects(end->centre()));
}

}  // namespace
}  // namespace routing
}  // namespace bfg
