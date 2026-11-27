#ifndef ROUTING_ROUTING_GRID_TEST_FIXTURE_H_
#define ROUTING_ROUTING_GRID_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "../dev_pdk_setup.h"
#include "../geometry/point.h"
#include "../geometry/port.h"
#include "../geometry/rectangle.h"
#include "../physical_properties_database.h"
#include "routing_blockage_cache.h"
#include "routing_grid.h"
#include "routing_layer_info.h"
#include "routing_path.h"
#include "routing_track_direction.h"
#include "routing_vertex.h"
#include "routing_via_info.h"

namespace bfg {
namespace routing {

// A met1 (horizontal) and met2 (vertical) grid over a small area, configured
// the way Interconnect::ConfigureRoutingGrid does it.
//
// Not in an anonymous namespace so that it can be a friend of RoutingGrid.
class RoutingGridTest : public testing::Test {
 protected:
  static constexpr int64_t kSize = 5000;

  void SetUp() override {
    physical_db_.LoadTechnologyFromFile("test_data/sky130.technology.pb");
    bfg::SetUpSky130(&physical_db_);
    met1_ = physical_db_.GetLayer("met1.drawing");
    met2_ = physical_db_.GetLayer("met2.drawing");
    grid_.reset(new RoutingGrid(physical_db_));

    geometry::Rectangle area({0, 0}, {kSize, kSize});
    RoutingLayerInfo met1_info =
        physical_db_.GetRoutingLayerInfoOrDie("met1.drawing");
    met1_info.set_direction(RoutingTrackDirection::kTrackHorizontal);
    met1_info.set_area(area);
    met1_info.set_offset(0);
    RoutingLayerInfo met2_info =
        physical_db_.GetRoutingLayerInfoOrDie("met2.drawing");
    met2_info.set_direction(RoutingTrackDirection::kTrackVertical);
    met2_info.set_area(area);
    met2_info.set_offset(0);

    RoutingViaInfo via_info =
        physical_db_.GetRoutingViaInfoOrDie("met1.drawing", "met2.drawing");
    via_info.set_cost(0.5);
    ASSERT_TRUE(grid_->AddRoutingViaInfo(met1_, met2_, via_info).ok());
    ASSERT_TRUE(grid_->AddRoutingLayerInfo(met1_info).ok());
    ASSERT_TRUE(grid_->AddRoutingLayerInfo(met2_info).ok());
    ASSERT_TRUE(grid_->ConnectLayers(met1_, met2_).ok());

    met1_pitch_ = met1_info.pitch();
    met2_pitch_ = met2_info.pitch();
  }

  // The on-grid vertex nearest to the given point.
  RoutingVertex *NearestVertex(const geometry::Point &point) {
    RoutingVertex *nearest = nullptr;
    for (RoutingVertex *vertex : grid_->vertices()) {
      if (!nearest || vertex->centre().L1DistanceTo(point) <
              nearest->centre().L1DistanceTo(point)) {
        nearest = vertex;
      }
    }
    return nearest;
  }

  // A met1 port on the vertex nearest the given point.
  std::unique_ptr<geometry::Port> MakePort(
      const geometry::Point &near, const std::string &net) {
    geometry::Point centre = NearestVertex(near)->centre();
    int64_t side = physical_db_.Rules("mcon.drawing").via_width;
    return std::make_unique<geometry::Port>(centre, side, side, met1_, net);
  }

  bool VerticesAreTooCloseForVias(
      const RoutingVertex &lhs, const RoutingVertex &rhs) const {
    return grid_->VerticesAreTooCloseForVias(lhs, rhs);
  }

  std::vector<CostedLayer> LayersReachableByVia(
      const geometry::Layer &from) const {
    return grid_->LayersReachableByVia(from);
  }

  absl::StatusOr<RoutingPath*> ShortestPath(
      RoutingVertex *begin, RoutingVertex *end,
      const RoutingBlockageCache &cache) {
    return grid_->ShortestPath(begin, end, cache);
  }

  PhysicalPropertiesDatabase physical_db_;
  std::unique_ptr<RoutingGrid> grid_;
  geometry::Layer met1_;
  geometry::Layer met2_;
  int64_t met1_pitch_;
  int64_t met2_pitch_;
};

}  // namespace routing
}  // namespace bfg

#endif  // ROUTING_ROUTING_GRID_TEST_FIXTURE_H_
