#include "routing_track_direction.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../geometry/radian.h"

namespace bfg {
namespace routing {
namespace {

std::string ToString(const RoutingTrackDirection &direction) {
  std::stringstream ss;
  ss << direction;
  return ss.str();
}

TEST(RoutingTrackDirectionTest, DirectionToAngle) {
  EXPECT_DOUBLE_EQ(0.0, RoutingTrackDirectionUtility::DirectionToAngle(
      RoutingTrackDirection::kTrackHorizontal));
  EXPECT_DOUBLE_EQ(geometry::Radian::kPi / 2,
                   RoutingTrackDirectionUtility::DirectionToAngle(
                       RoutingTrackDirection::kTrackVertical));
}

TEST(RoutingTrackDirectionTest, AllDirections) {
  EXPECT_EQ(2, RoutingTrackDirectionUtility::kAllDirections.size());
  EXPECT_EQ(1, RoutingTrackDirectionUtility::kAllDirections.count(
      RoutingTrackDirection::kTrackHorizontal));
  EXPECT_EQ(1, RoutingTrackDirectionUtility::kAllDirections.count(
      RoutingTrackDirection::kTrackVertical));
}

TEST(RoutingTrackDirectionTest, Print) {
  EXPECT_EQ("horizontal", ToString(RoutingTrackDirection::kTrackHorizontal));
  EXPECT_EQ("vertical", ToString(RoutingTrackDirection::kTrackVertical));
}

}  // namespace
}  // namespace routing
}  // namespace bfg
