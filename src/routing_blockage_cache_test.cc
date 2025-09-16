#include "routing_blockage_cache.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "routing_track_direction.h"
#include "design_database.h"
#include "physical_properties_database.h"
#include "routing_grid.h"
#include "geometry/rectangle.h"
#include "dev_pdk_setup.h"

namespace bfg {
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
    const PhysicalPropertiesDatabase &db = design_db_.physical_db();
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
  DesignDatabase design_db_;
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

}   // namespace
}   // namespace bfg
