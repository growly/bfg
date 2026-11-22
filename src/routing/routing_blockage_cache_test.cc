#include "routing_blockage_cache.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "routing_track_direction.h"
#include "../design_database.h"
#include "../physical_properties_database.h"
#include "routing_grid.h"
#include "../geometry/rectangle.h"
#include "../dev_pdk_setup.h"

namespace bfg {
namespace routing {
namespace {

class RoutingBlockageCacheTest : public testing::Test {
 protected:
  void SetUp() override {
    bfg::PhysicalPropertiesDatabase &physical_db = design_db_.physical_db();
    design_db_.physical_db().LoadTechnologyFromFile(
        "test_data/sky130.technology.pb");
    bfg::SetUpSky130(&physical_db);

    routing_grid_.reset(new RoutingGrid(physical_db));
    cache_.reset(new RoutingBlockageCache(*routing_grid_));

    ConfigureRoutingGrid();
  }

  void ConfigureRoutingGrid() const {
    const bfg::PhysicalPropertiesDatabase &db = design_db_.physical_db();
    const auto &met1_rules = db.Rules("met1.drawing");
    const auto &met2_rules = db.Rules("met2.drawing");

    RoutingLayerInfo met1_layer_info =
        db.GetRoutingLayerInfoOrDie("met1.drawing");
    met1_layer_info.set_direction(RoutingTrackDirection::kTrackHorizontal);
    met1_layer_info.set_area(geometry::Rectangle({0, 0}, {1500, 1500}));
    met1_layer_info.set_offset(170);  // Half a pitch.

    RoutingLayerInfo met2_layer_info =
        db.GetRoutingLayerInfoOrDie("met2.drawing");
    met2_layer_info.set_direction(RoutingTrackDirection::kTrackVertical);
    met2_layer_info.set_area(geometry::Rectangle({0, 0}, {1500, 1500}));
    met2_layer_info.set_offset(0);

    RoutingViaInfo routing_via_info =
        db.GetRoutingViaInfoOrDie("met1.drawing", "met2.drawing");
    routing_via_info.set_cost(0.5);
    routing_grid_->AddRoutingViaInfo(
        met1_layer_info.layer(), met2_layer_info.layer(), routing_via_info)
        .IgnoreError();

    routing_via_info = db.GetRoutingViaInfoOrDie("li.drawing", "met1.drawing");
    routing_via_info.set_cost(0.5);
    routing_grid_->AddRoutingViaInfo(
        met1_layer_info.layer(), db.GetLayer("li.drawing"), routing_via_info)
        .IgnoreError();

    routing_via_info = db.GetRoutingViaInfoOrDie(
        "met2.drawing", "met3.drawing");
    routing_via_info.set_cost(0.5);
    routing_grid_->AddRoutingViaInfo(
        db.GetLayer("met3.drawing"), met2_layer_info.layer(), routing_via_info)
        .IgnoreError();

    routing_grid_->AddRoutingLayerInfo(met1_layer_info).IgnoreError();
    routing_grid_->AddRoutingLayerInfo(met2_layer_info).IgnoreError();

    routing_grid_->ConnectLayers(
        met1_layer_info.layer(), met2_layer_info.layer()).IgnoreError();
  }

  std::unique_ptr<RoutingBlockageCache> cache_;

  std::unique_ptr<RoutingGrid> routing_grid_;
  bfg::DesignDatabase design_db_;
};

TEST_F(RoutingBlockageCacheTest, RectangleBlockageParityWithRoutingGrid_1) {
  geometry::Rectangle hazard({100, 100}, {500, 500});
  hazard.set_layer(design_db_.physical_db().GetLayer("met1.drawing"));

  routing_grid_->AddBlockage(hazard);

  cache_->AddBlockage(hazard, 0);

  for (const auto *vertex : routing_grid_->vertices()) {
    EXPECT_EQ(
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt),
        !vertex->Available());
  }
}

TEST_F(RoutingBlockageCacheTest, RectangleBlockageParityWithRoutingGrid_2) {
  // This rectangle mimicks a met1 horizontal rail (for power or ground), and
  // is positioned such that it blocks vertices at y = 0, and restricts
  // vertices at y = 340 to only be horizontal.
  geometry::Rectangle hazard({0, -240}, {1500, 240});
  geometry::Layer met1 = design_db_.physical_db().GetLayer("met1.drawing");
  hazard.set_layer(met1);

  routing_grid_->AddBlockage(hazard);

  cache_->AddBlockage(hazard, 0);

  std::vector<RoutingVertex*> expected_blocked;
  std::vector<RoutingVertex*> expected_restricted;
  std::vector<RoutingVertex*> expected_free;

  for (RoutingVertex *vertex : routing_grid_->vertices()) {
    if (vertex->centre().y() == 170) {
      expected_blocked.push_back(vertex);
    } else if (vertex->centre().y() == 510) {
      expected_restricted.push_back(vertex);
    } else {
      expected_free.push_back(vertex);
    }
  }

  for (const RoutingVertex *vertex : expected_blocked) {
    EXPECT_EQ(
        !vertex->Available(),
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
    EXPECT_EQ(
        true,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }
  for (const RoutingVertex *vertex : expected_free) {
    EXPECT_EQ(
        !vertex->Available(),
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
    EXPECT_EQ(
        false,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }

  for (const RoutingVertex *vertex : expected_restricted) {
    auto forced_encap_direction = vertex->GetForcedEncapDirection(met1);
    ASSERT_TRUE(forced_encap_direction);
    EXPECT_EQ(RoutingTrackDirection::kTrackHorizontal,
              forced_encap_direction.value());

    EXPECT_EQ(
        true,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
    EXPECT_EQ(
        false, 
        cache_->IsVertexBlocked(
            *vertex, {}, RoutingTrackDirection::kTrackHorizontal, met1));
  }
}

TEST_F(RoutingBlockageCacheTest, PolygonBlockageParityWithRoutingGrid_1) {
  geometry::Polygon hazard({
      {0, 690}, 
      {690, 690},
      {690, 0},
      {670, 0},
      {670, 670},
      {0, 670}
  });
  hazard.set_layer(design_db_.physical_db().GetLayer("met1.drawing"));

  routing_grid_->AddBlockage(hazard);

  cache_->AddBlockage(hazard, 0);

  for (const auto *vertex : routing_grid_->vertices()) {
    EXPECT_EQ(
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt),
        !vertex->Available());
  }
}

TEST_F(RoutingBlockageCacheTest, RectangleBlockageCancelled_1) {
  // This rectangle mimicks a met1 horizontal rail (for power or ground), and
  // is positioned such that it blocks vertices at y = 0, and restricts
  // vertices at y = 340 to only be horizontal.
  geometry::Rectangle hazard({0, -240}, {1500, 240});
  geometry::Layer met1 = design_db_.physical_db().GetLayer("met1.drawing");
  hazard.set_layer(met1);

  cache_->AddBlockage(hazard, 0);

  std::vector<RoutingVertex*> expected_blocked;
  std::vector<RoutingVertex*> expected_restricted;
  std::vector<RoutingVertex*> expected_free;

  for (RoutingVertex *vertex : routing_grid_->vertices()) {
    if (vertex->centre().y() == 170) {
      expected_blocked.push_back(vertex);
    } else if (vertex->centre().y() == 510) {
      expected_restricted.push_back(vertex);
    } else {
      expected_free.push_back(vertex);
    }
  }

  for (const RoutingVertex *vertex : expected_blocked) {
    EXPECT_EQ(
        true,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }
  for (const RoutingVertex *vertex : expected_free) {
    EXPECT_EQ(
        false,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }

  for (const RoutingVertex *vertex : expected_restricted) {
    EXPECT_EQ(
        true,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
    EXPECT_EQ(
        false, 
        cache_->IsVertexBlocked(
            *vertex, {}, RoutingTrackDirection::kTrackHorizontal, met1));
  }

  // Now we cancel:
  cache_->CancelBlockage(hazard);

  for (const RoutingVertex *vertex : expected_blocked) {
    EXPECT_EQ(
        false,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }
  for (const RoutingVertex *vertex : expected_free) {
    EXPECT_EQ(
        false,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
  }

  for (const RoutingVertex *vertex : expected_restricted) {
    EXPECT_EQ(
        false,
        cache_->IsVertexBlocked(*vertex, {}, std::nullopt, std::nullopt));
    EXPECT_EQ(
        false, 
        cache_->IsVertexBlocked(
            *vertex, {}, RoutingTrackDirection::kTrackHorizontal, met1));
  }
}

// A met1 rail on net "vdd" covering the vertices at y = 170.
class RoutingBlockageCacheNetTest : public RoutingBlockageCacheTest {
 protected:
  void SetUp() override {
    RoutingBlockageCacheTest::SetUp();
    met1_ = design_db_.physical_db().GetLayer("met1.drawing");
    rail_ = geometry::Rectangle({0, -240}, {1500, 240}, met1_, "vdd");
    cache_->AddBlockage(rail_, 0);
  }

  RoutingVertex *VertexAt(const geometry::Point &point) {
    for (RoutingVertex *vertex : routing_grid_->vertices()) {
      if (vertex->centre() == point) {
        return vertex;
      }
    }
    return nullptr;
  }

  // A met1 edge along the rail.
  RoutingEdge *EdgeOnRail() {
    RoutingVertex *vertex = VertexAt({0, 170});
    for (RoutingEdge *edge : vertex->edges()) {
      if (edge->EffectiveLayer() == met1_) {
        return edge;
      }
    }
    return nullptr;
  }

  geometry::Layer met1_;
  geometry::Rectangle rail_;
};

TEST_F(RoutingBlockageCacheNetTest, VertexOnRailIsBlocked) {
  RoutingVertex *on_rail = VertexAt({0, 170});
  RoutingVertex *off_rail = VertexAt({0, 1190});
  ASSERT_NE(nullptr, on_rail);
  ASSERT_NE(nullptr, off_rail);
  EXPECT_TRUE(cache_->IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));
  EXPECT_FALSE(cache_->AvailableForAll(*on_rail));
  EXPECT_TRUE(cache_->AvailableForAll(*off_rail));
  // Vertex blockages on the vertex's own net still inhibit via directions, so
  // the vertex is not freed for "vdd" (see PLAN.md observations).
}

TEST_F(RoutingBlockageCacheNetTest, IsEdgeBlocked) {
  RoutingEdge *edge = EdgeOnRail();
  ASSERT_NE(nullptr, edge);
  EXPECT_TRUE(cache_->IsEdgeBlocked(*edge, {}));
  EXPECT_FALSE(cache_->IsEdgeBlocked(*edge, EquivalentNets("vdd")));
  EXPECT_TRUE(cache_->IsEdgeBlocked(*edge, EquivalentNets("other")));
}

TEST_F(RoutingBlockageCacheNetTest, CancelBlockagesByNet) {
  RoutingVertex *on_rail = VertexAt({0, 170});
  ASSERT_NE(nullptr, on_rail);

  cache_->CancelBlockages(EquivalentNets("other"));
  EXPECT_TRUE(cache_->IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));

  // Restricting cancellation to another layer leaves the rail in place.
  cache_->CancelBlockages(EquivalentNets("vdd"), "met2.drawing");
  EXPECT_TRUE(cache_->IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));

  cache_->CancelBlockages(EquivalentNets("vdd"), "met1.drawing");
  EXPECT_FALSE(
      cache_->IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));
  EXPECT_FALSE(cache_->IsEdgeBlocked(*EdgeOnRail(), {}));
}

TEST_F(RoutingBlockageCacheNetTest, ChildCacheCancellationsDoNotAffectParent) {
  RoutingVertex *on_rail = VertexAt({0, 170});
  ASSERT_NE(nullptr, on_rail);

  RoutingBlockageCache child(*routing_grid_, *cache_);
  EXPECT_TRUE(child.IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));

  child.CancelBlockages(EquivalentNets("vdd"));
  EXPECT_FALSE(child.IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));
  EXPECT_TRUE(cache_->IsVertexBlocked(*on_rail, {}, std::nullopt, std::nullopt));
}

TEST_F(RoutingBlockageCacheNetTest, ValidAgainstKnownBlockages) {
  geometry::Rectangle overlapping({100, 0}, {200, 100}, met1_, "");
  geometry::Rectangle far_away({100, 1200}, {200, 1300}, met1_, "");

  absl::Status status = cache_->ValidAgainstKnownBlockages(overlapping);
  EXPECT_EQ(absl::StatusCode::kResourceExhausted, status.code()) << status;
  EXPECT_TRUE(cache_->ValidAgainstKnownBlockages(far_away).ok());
  EXPECT_TRUE(cache_->ValidAgainstKnownBlockages(
      overlapping, EquivalentNets("vdd")).ok());

  RoutingVertex *on_rail = VertexAt({0, 170});
  RoutingVertex *off_rail = VertexAt({0, 1190});
  ASSERT_NE(nullptr, on_rail);
  ASSERT_NE(nullptr, off_rail);
  EXPECT_FALSE(cache_->ValidAgainstKnownBlockages(*on_rail).ok());
  EXPECT_TRUE(cache_->ValidAgainstKnownBlockages(*off_rail).ok());
}

TEST_F(RoutingBlockageCacheNetTest, ValidAgainstKnownBlockages_Child) {
  // Hazards in the parent apply to the child.
  RoutingBlockageCache child(*routing_grid_, *cache_);
  geometry::Rectangle overlapping({100, 0}, {200, 100}, met1_, "");
  EXPECT_FALSE(child.ValidAgainstKnownBlockages(overlapping).ok());
}

TEST_F(RoutingBlockageCacheNetTest, Summary) {
  std::string summary = cache_->Summary();
  EXPECT_NE(std::string::npos, summary.find("1 rectangles"));
  EXPECT_NE(std::string::npos, summary.find("parent: none"));

  RoutingBlockageCache child(*routing_grid_, *cache_);
  EXPECT_NE(std::string::npos, child.Summary().find("parent: [Routing"));
}

}  // namespace
}  // namespace routing
}  // namespace bfg
