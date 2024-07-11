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

  track.AddBlockage(geometry::Rectangle({0, 0}, {100, 100}), 0, "");

  EXPECT_EQ(1U, track.blockages_.size());
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());

  // Second blockages is more than min_separation away, so not merged.

  track.AddBlockage(geometry::Rectangle({200, 0}, {300, 100}), 0, "");

  EXPECT_EQ(2U, track.blockages_.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(200, track.blockages_.back()->start());
  EXPECT_EQ(300, track.blockages_.back()->end());
  EXPECT_EQ("", track.blockages_.back()->net());

  // Third blockage should also not conflict:
  track.AddBlockage(geometry::Rectangle({600, 0}, {650, 100}), 0, "");
  EXPECT_EQ(3U, track.blockages_.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(600, track.blockages_.back()->start());
  EXPECT_EQ(650, track.blockages_.back()->end());
  EXPECT_EQ("", track.blockages_.back()->net());

  // Fourth blockage should span the first two existing blockages, and should
  // thus be merged:
  track.AddBlockage(geometry::Rectangle({-50, 0}, {450, 100}), 0, "");
  EXPECT_EQ(2U, track.blockages_.size());
  EXPECT_EQ(-50, track.blockages_.front()->start());
  EXPECT_EQ(450, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
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

  track.AddBlockage(geometry::Rectangle({0, 0}, {100, 100}), 0, "");

  EXPECT_EQ(1U, track.blockages_.size());
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());

  // Second blockages is more than min_separation away, so not merged.

  track.AddBlockage(geometry::Rectangle({200, 0}, {300, 100}), 0, "some_net");

  EXPECT_EQ(2U, track.blockages_.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(200, track.blockages_.back()->start());
  EXPECT_EQ(300, track.blockages_.back()->end());
  EXPECT_EQ("some_net", track.blockages_.back()->net());

  // Third blockage should also not conflict:
  track.AddBlockage(geometry::Rectangle({600, 0}, {650, 100}), 0, "");
  EXPECT_EQ(3U, track.blockages_.size());
  // First blockage should not have changed.
  EXPECT_EQ(0, track.blockages_.front()->start());
  EXPECT_EQ(100, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
  // New blockage should be the back (because it's sorted).
  EXPECT_EQ(600, track.blockages_.back()->start());
  EXPECT_EQ(650, track.blockages_.back()->end());
  EXPECT_EQ("", track.blockages_.back()->net());

  // Fourth blockage should span two of the three existing blockages, and should
  // thus be merged. The blockage with a differing net should be untouched.
  track.AddBlockage(geometry::Rectangle({-50, 0}, {625, 100}), 0, "");
  EXPECT_EQ(2U, track.blockages_.size());
  EXPECT_EQ(-50, track.blockages_.front()->start());
  EXPECT_EQ(650, track.blockages_.front()->end());
  EXPECT_EQ("", track.blockages_.front()->net());
  EXPECT_EQ(200, track.blockages_.back()->start());
  EXPECT_EQ(300, track.blockages_.back()->end());
  EXPECT_EQ("some_net", track.blockages_.back()->net());
}

}  // namespace bfg
