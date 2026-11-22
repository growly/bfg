#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <glog/logging.h>

#include "../geometry/point.h"
#include "../geometry/rectangle.h"
#include "../geometry/polygon.h"
#include "../physical_properties_database.h"
#include "routing_blockage_cache.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_track.h"
#include "routing_track_blockage.h"

namespace bfg {
namespace routing {
namespace {

// Fixtures in the anonymous namespace cannot be friended by the class under
// test so much not touch private parts.

TEST(RoutingTrackTest, Empty) {
  //EquivalentNets nets = EquivalentNets(std::set<std::string>());
  //EXPECT_FALSE(nets.Contains("a"));
  //EXPECT_FALSE(nets.Contains("b"));
  //EXPECT_FALSE(nets.Contains("c"));
  //EXPECT_EQ("", nets.primary());
}

}  // namespace

TEST(RoutingTrackTest, MergesBlockages) {
  RoutingTrack track = RoutingTrack(
      0,                                        // Layer
      RoutingTrackDirection::kTrackHorizontal,  // Direction
      100,                                      // Pitch
      50,                                       // Width
      25,                                       // Vertex via width
      25,                                       // Vertex via length
      50,                                       // Minimum separation
      50,                                       // Offset
      false);                                   // Neighbours only

  // We have a horizontal track at y=50:
  //
  // y=50 -----------------------------------
  //

  track.AddBlockage(
      geometry::Rectangle({0, 0}, {100, 100}), 0, "", nullptr, nullptr);

  std::vector<RoutingTrackBlockage*> &vertex_blockages =
      track.blockages_.vertex_blockages;

  EXPECT_EQ(1U, vertex_blockages.size());
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());

  // Second blockages is more than min_separation away, so not merged.

  track.AddBlockage(
      geometry::Rectangle({200, 0}, {300, 100}), 0, "", nullptr, nullptr);

  EXPECT_EQ(2U, vertex_blockages.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(200, vertex_blockages.back()->start());
  EXPECT_EQ(300, vertex_blockages.back()->end());
  EXPECT_EQ("", vertex_blockages.back()->net());

  // Third blockage should also not conflict:
  track.AddBlockage(
      geometry::Rectangle({600, 0}, {650, 100}), 0, "", nullptr, nullptr);
  EXPECT_EQ(3U, vertex_blockages.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(600, vertex_blockages.back()->start());
  EXPECT_EQ(650, vertex_blockages.back()->end());
  EXPECT_EQ("", vertex_blockages.back()->net());

  // Fourth blockage should span the first two existing blockages, and should
  // thus be merged:
  track.AddBlockage(
      geometry::Rectangle({-50, 0}, {450, 100}), 0, "", nullptr, nullptr);
  EXPECT_EQ(2U, vertex_blockages.size());
  EXPECT_EQ(-50, vertex_blockages.front()->start());
  EXPECT_EQ(450, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
}

TEST(RoutingTrackTest, DoesNotMergeDifferingNets) {
  RoutingTrack track = RoutingTrack(
      0,                                        // Layer
      RoutingTrackDirection::kTrackHorizontal,  // Direction
      100,                                      // Pitch
      50,                                       // Width
      25,                                       // Vertex via width
      25,                                       // Vertex via length
      50,                                       // Minimum separation
      50,                                       // Offset
      false);                                   // Neighbours only

  // We have a horizontal track at y=50:
  //
  // y=50 -----------------------------------
  //

  track.AddBlockage(
      geometry::Rectangle({0, 0}, {100, 100}), 0, "", nullptr, nullptr);

  std::vector<RoutingTrackBlockage*> &vertex_blockages =
      track.blockages_.vertex_blockages;

  EXPECT_EQ(1U, vertex_blockages.size());
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());

  // Second blockages is more than min_separation away, so not merged.

  track.AddBlockage(geometry::Rectangle({200, 0}, {300, 100}),
                    0, "some_net", nullptr, nullptr);

  EXPECT_EQ(2U, vertex_blockages.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(200, vertex_blockages.back()->start());
  EXPECT_EQ(300, vertex_blockages.back()->end());
  EXPECT_EQ("some_net", vertex_blockages.back()->net());

  // Third blockage should also not conflict:
  track.AddBlockage(
      geometry::Rectangle({600, 0}, {650, 100}), 0, "", nullptr, nullptr);
  EXPECT_EQ(3U, vertex_blockages.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, vertex_blockages.front()->start());
  EXPECT_EQ(100, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(600, vertex_blockages.back()->start());
  EXPECT_EQ(650, vertex_blockages.back()->end());
  EXPECT_EQ("", vertex_blockages.back()->net());

  // Fourth blockage should span two of the three existing blockages, and should
  // thus be merged. The blockage with a differing net should be untouched.
  track.AddBlockage(
      geometry::Rectangle({-50, 0}, {625, 100}), 0, "", nullptr, nullptr);
  EXPECT_EQ(2U, vertex_blockages.size());
  EXPECT_EQ(-50, vertex_blockages.front()->start());
  EXPECT_EQ(650, vertex_blockages.front()->end());
  EXPECT_EQ("", vertex_blockages.front()->net());
  EXPECT_EQ(200, vertex_blockages.back()->start());
  EXPECT_EQ(300, vertex_blockages.back()->end());
  EXPECT_EQ("some_net", vertex_blockages.back()->net());
}

TEST(RoutingTrackTest, GetImmediateNeighbours) {
  int64_t y = 50;

  RoutingTrack track = RoutingTrack(
      0,                                        // Layer
      RoutingTrackDirection::kTrackHorizontal,  // Direction
      100,                                      // Pitch
      50,                                       // Width
      25,                                       // Vertex via width
      25,                                       // Vertex via length
      50,                                       // Minimum separation
      y,                                        // Offset
      false);                                   // Neighbours only

  // We have a horizontal track at y=50:
  //
  // y=50 -----------------------------------
  //

  int64_t pitch = 200;

  std::vector<std::unique_ptr<RoutingVertex>> vertices;
  for (int64_t i = 0; i < 10; ++i) {
    RoutingVertex *vertex = new RoutingVertex({i * pitch, y});
    vertices.emplace_back(vertex);
  }

  bfg::PhysicalPropertiesDatabase physical_db;
  RoutingGrid routing_grid(physical_db);
  RoutingBlockageCache empty_blockage_cache(routing_grid);

  // Empty track -> no neighbours:
  EXPECT_TRUE(track.GetImmediateNeighbours(*vertices[5]).empty());

  for (auto &vertex : vertices) {
    track.AddVertex(vertex.get(), empty_blockage_cache);
  }

  // Do not return the vertex matching the requested offset.
  std::vector<RoutingVertex*> expected = {
    vertices[4].get(), vertices[6].get()
  };
  EXPECT_THAT(track.GetImmediateNeighbours(*vertices[5]),
              testing::ContainerEq(expected));

  // Off the end at the front:
  std::unique_ptr<RoutingVertex> test(new RoutingVertex({-5 * pitch, y}));
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, false),
      testing::ContainerEq(std::vector<RoutingVertex*>{vertices[0].get()}));

  // Off the end at the back:
  test.reset(new RoutingVertex({(
      static_cast<int64_t>(vertices.size()) + 5) * pitch, y}));
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, false),
      testing::ContainerEq(std::vector<RoutingVertex*>{vertices[9].get()}));

  test.reset(new RoutingVertex({pitch / 2, y}));
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, false),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[0].get(), vertices[1].get()}));

  // Return unavailable vertices.
  vertices[5]->AddBlockingNet("test", false);
  test.reset(new RoutingVertex({5 * pitch + pitch / 2, y}));
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[5].get(), vertices[6].get()}));

  // Unless requested to only include available ones.
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[4].get(), vertices[6].get()}));

  vertices[4]->AddBlockingNet("test", false);
  vertices[6]->AddBlockingNet("test", false);
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[3].get(), vertices[7].get()}));

  // Block 1, 2, 3, 4, 5, 6:
  for (size_t i = 1; i < 7; ++i) {
    vertices[i]->AddBlockingNet("asdf", false);
  }
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[0].get(), vertices[7].get()}));

  // Block 1 - 9:
  for (size_t i = 1; i < 9; ++i) {
    vertices[i]->AddBlockingNet("asdf2", false);
  }
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[0].get(), vertices[9].get()}));

  // Block 0:
  vertices.front()->AddBlockingNet("asdf3", false);
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[9].get()}));

  // Block 9. Now they're all blocked:
  vertices.back()->AddBlockingNet("asdf4", false);
  EXPECT_TRUE(track.GetImmediateNeighbours(*test, true).empty());
}

// A horizontal track at y = 50 with vertices every 200 units from x = 0.
class RoutingTrackWithVerticesTest : public testing::Test {
 protected:
  static constexpr int64_t kY = 50;
  static constexpr int64_t kPitch = 200;

  RoutingTrackWithVerticesTest()
      : track_(0,                                        // Layer
               RoutingTrackDirection::kTrackHorizontal,  // Direction
               100,                                      // Pitch
               50,                                       // Width
               25,                                       // Vertex via width
               25,                                       // Vertex via length
               50,                                       // Min. separation
               kY,                                       // Offset
               false),                                   // Neighbours only
        routing_grid_(physical_db_),
        cache_(routing_grid_) {}

  void SetUp() override {
    for (int64_t i = 0; i < 10; ++i) {
      vertices_.emplace_back(new RoutingVertex({i * kPitch, kY}));
      ASSERT_TRUE(track_.AddVertex(vertices_.back().get(), cache_));
    }
  }

  RoutingVertex *V(size_t i) { return vertices_.at(i).get(); }

  // Declared first so that it outlives the track's edges, which refer to it.
  std::vector<std::unique_ptr<RoutingVertex>> vertices_;
  RoutingTrack track_;
  bfg::PhysicalPropertiesDatabase physical_db_;
  RoutingGrid routing_grid_;
  RoutingBlockageCache cache_;
};

TEST_F(RoutingTrackWithVerticesTest, VertexLookup) {
  EXPECT_TRUE(track_.ContainsVertex(V(3)));
  EXPECT_EQ(V(3), track_.GetVertexAt({3 * kPitch, kY}));
  EXPECT_EQ(V(3), track_.GetVertexAtOffset(3 * kPitch));
  EXPECT_EQ(nullptr, track_.GetVertexAt({3 * kPitch + 1, kY}));

  EXPECT_TRUE(track_.IsPointOnTrack({123, kY}));
  EXPECT_FALSE(track_.IsPointOnTrack({123, kY + 1}));

  RoutingVertex elsewhere({0, 0});
  EXPECT_FALSE(track_.ContainsVertex(&elsewhere));
}

TEST_F(RoutingTrackWithVerticesTest, EdgesConnectNeighbours) {
  RoutingEdge *edge = track_.GetEdgeBetween(V(4), V(5));
  ASSERT_NE(nullptr, edge);
  EXPECT_EQ(edge, track_.GetEdgeBetween(V(5), V(4)));
  EXPECT_EQ(&track_, edge->track());

  std::vector<RoutingVertex*> available;
  track_.ReportAvailableVertices(&available);
  EXPECT_EQ(vertices_.size(), available.size());
}

TEST_F(RoutingTrackWithVerticesTest, ReportAvailableEdges) {
  std::vector<RoutingEdge*> edges;
  track_.ReportAvailableEdges(&edges);
  EXPECT_EQ(track_.edges().size(), edges.size());
  EXPECT_GE(edges.size(), vertices_.size() - 1);
}

TEST_F(RoutingTrackWithVerticesTest, VerticesInSpan) {
  // Vertices are in track order, but the direction is not guaranteed.
  auto in_order = [](std::vector<RoutingVertex*> vertices) {
    if (!vertices.empty() &&
        vertices.front()->centre().x() > vertices.back()->centre().x()) {
      std::reverse(vertices.begin(), vertices.end());
    }
    return vertices;
  };
  std::vector<RoutingVertex*> expected = {V(1), V(2), V(3)};
  EXPECT_EQ(expected,
            in_order(track_.VerticesInSpan({kPitch, kY}, {3 * kPitch, kY})));
  EXPECT_EQ(expected,
            in_order(track_.VerticesInSpan({3 * kPitch, kY}, {kPitch, kY})));
  EXPECT_TRUE(track_.VerticesInSpan({10, kY}, {20, kY}).empty());
}

TEST_F(RoutingTrackWithVerticesTest, RemoveEdge) {
  RoutingEdge *edge = track_.GetEdgeBetween(V(4), V(5));
  ASSERT_NE(nullptr, edge);
  EXPECT_TRUE(track_.RemoveEdge(edge, false));
  EXPECT_EQ(nullptr, track_.GetEdgeBetween(V(4), V(5)));
  EXPECT_FALSE(track_.RemoveEdge(edge, false));
  delete edge;
}

TEST_F(RoutingTrackWithVerticesTest, RemoveVertex) {
  ASSERT_TRUE(track_.RemoveVertex(V(5), cache_));
  EXPECT_FALSE(track_.ContainsVertex(V(5)));
  EXPECT_EQ(nullptr, track_.GetEdgeBetween(V(4), V(5)));
  EXPECT_EQ(nullptr, track_.GetEdgeBetween(V(5), V(6)));
  // The neighbours are still connected around the gap.
  EXPECT_NE(nullptr, track_.GetEdgeBetween(V(4), V(6)));
  EXPECT_FALSE(track_.RemoveVertex(V(5), cache_));
}

TEST_F(RoutingTrackWithVerticesTest, TemporaryBlockage) {
  geometry::Rectangle over_v5 = geometry::Rectangle::CentredAt(
      V(5)->centre(), 10, 10);
  RoutingTrackBlockage *vertex_blockage = nullptr;
  RoutingTrackBlockage *edge_blockage = nullptr;
  std::set<RoutingVertex*> blocked_vertices;
  std::set<RoutingEdge*> blocked_edges;
  track_.AddTemporaryBlockage(over_v5, 0, "", &vertex_blockage,
                              &edge_blockage, &blocked_vertices,
                              &blocked_edges);
  ASSERT_NE(nullptr, vertex_blockage);
  EXPECT_EQ(1, blocked_vertices.count(V(5)));
  EXPECT_FALSE(V(5)->Available());
  EXPECT_TRUE(V(4)->Available());

  EXPECT_TRUE(track_.RemoveTemporaryBlockage(vertex_blockage));
  delete vertex_blockage;
  if (edge_blockage) {
    EXPECT_TRUE(track_.RemoveTemporaryBlockage(edge_blockage));
    delete edge_blockage;
  }
}

TEST_F(RoutingTrackWithVerticesTest, TemporaryBlockageAwayFromTrack) {
  geometry::Rectangle far_away({0, 1000}, {10, 1010});
  RoutingTrackBlockage *vertex_blockage = nullptr;
  RoutingTrackBlockage *edge_blockage = nullptr;
  track_.AddTemporaryBlockage(far_away, 0, "", &vertex_blockage,
                              &edge_blockage, nullptr, nullptr);
  EXPECT_EQ(nullptr, vertex_blockage);
  EXPECT_EQ(nullptr, edge_blockage);
}

// TODO(aryap): RemoveTemporaryBlockage returns true even when the blockage
// was not found.
TEST_F(RoutingTrackWithVerticesTest,
       DISABLED_RemoveTemporaryBlockage_NotFound) {
  RoutingTrackBlockage unknown(0, 10, "");
  EXPECT_FALSE(track_.RemoveTemporaryBlockage(&unknown));
}

TEST_F(RoutingTrackWithVerticesTest, Describe) {
  EXPECT_FALSE(track_.Describe().empty());
}

}  // namespace routing
}  // namespace bfg
