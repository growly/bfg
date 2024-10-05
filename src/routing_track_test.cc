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
      50);                                      // Offset

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
      50);                                      // Offset

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

}  // namespace bfg
