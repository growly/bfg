#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>
#include <glog/logging.h>

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"
#include "physical_properties_database.h"
#include "routing_track.h"
#include "routing_track_blockage.h"

namespace bfg {
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

  // Empty track -> no neighbours:
  EXPECT_TRUE(track.GetImmediateNeighbours(*vertices[5]).empty());

  for (auto &vertex : vertices) {
    track.AddVertex(vertex.get());
  }

  // Do not return the vertex matching the requested offset.
  std::vector<RoutingVertex*> expected = {
    vertices[4].get(), vertices[6].get()
  };
  EXPECT_THAT(track.GetImmediateNeighbours(*vertices[5]),
              testing::ContainerEq(expected));

  std::unique_ptr<RoutingVertex> test(new RoutingVertex({pitch / 2, y}));
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

  for (size_t i = 7; i < 9; ++i) {
    vertices[i]->AddBlockingNet("asdf", false);
  }
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[0].get(), vertices[9].get()}));

  vertices.front()->AddBlockingNet("asdf", false);
  EXPECT_THAT(
      track.GetImmediateNeighbours(*test, true),
      testing::ContainerEq(std::vector<RoutingVertex*>{
          vertices[9].get()}));

  vertices.back()->AddBlockingNet("asdf", false);
  EXPECT_TRUE(track.GetImmediateNeighbours(*test, true).empty());
}

}  // namespace bfg
