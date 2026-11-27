#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

#include "compass.h"
#include "proto/compass_direction.pb.h"

namespace bfg {
namespace geometry {
namespace {

const std::vector<Compass> kAllDirections = {
    Compass::WEST,
    Compass::NORTH_WEST,
    Compass::NORTH,
    Compass::NORTH_EAST,
    Compass::EAST,
    Compass::SOUTH_EAST,
    Compass::SOUTH,
    Compass::SOUTH_WEST
};

std::string ToString(const Compass &compass) {
  std::stringstream ss;
  ss << compass;
  return ss.str();
}

TEST(CompassTest, ToProto) {
  EXPECT_EQ(proto::CompassDirection::WEST,
            CompassToProtoCompassDirection(Compass::WEST));
  EXPECT_EQ(proto::CompassDirection::NORTH_WEST,
            CompassToProtoCompassDirection(Compass::NORTH_WEST));
  EXPECT_EQ(proto::CompassDirection::NORTH,
            CompassToProtoCompassDirection(Compass::NORTH));
  EXPECT_EQ(proto::CompassDirection::NORTH_EAST,
            CompassToProtoCompassDirection(Compass::NORTH_EAST));
  EXPECT_EQ(proto::CompassDirection::EAST,
            CompassToProtoCompassDirection(Compass::EAST));
  EXPECT_EQ(proto::CompassDirection::SOUTH_EAST,
            CompassToProtoCompassDirection(Compass::SOUTH_EAST));
  EXPECT_EQ(proto::CompassDirection::SOUTH,
            CompassToProtoCompassDirection(Compass::SOUTH));
  EXPECT_EQ(proto::CompassDirection::SOUTH_WEST,
            CompassToProtoCompassDirection(Compass::SOUTH_WEST));
}

TEST(CompassTest, RoundTripThroughProto) {
  for (const Compass &compass : kAllDirections) {
    EXPECT_EQ(compass, ProtoCompassDirectionToCompass(
        CompassToProtoCompassDirection(compass)));
  }
}

TEST(CompassTest, AliasesMapToSameProto) {
  EXPECT_EQ(CompassToProtoCompassDirection(Compass::WEST),
            CompassToProtoCompassDirection(Compass::LEFT));
  EXPECT_EQ(CompassToProtoCompassDirection(Compass::NORTH_EAST),
            CompassToProtoCompassDirection(Compass::UPPER_RIGHT));
  EXPECT_EQ(CompassToProtoCompassDirection(Compass::SOUTH),
            CompassToProtoCompassDirection(Compass::LOWER));
}

TEST(CompassTest, HasNorth) {
  EXPECT_TRUE(CompassHasNorth(Compass::NORTH_WEST));
  EXPECT_TRUE(CompassHasNorth(Compass::NORTH));
  EXPECT_TRUE(CompassHasNorth(Compass::NORTH_EAST));

  EXPECT_FALSE(CompassHasNorth(Compass::WEST));
  EXPECT_FALSE(CompassHasNorth(Compass::EAST));
  EXPECT_FALSE(CompassHasNorth(Compass::SOUTH_EAST));
  EXPECT_FALSE(CompassHasNorth(Compass::SOUTH));
  EXPECT_FALSE(CompassHasNorth(Compass::SOUTH_WEST));
}

TEST(CompassTest, HasSouth) {
  EXPECT_TRUE(CompassHasSouth(Compass::SOUTH_WEST));
  EXPECT_TRUE(CompassHasSouth(Compass::SOUTH));
  EXPECT_TRUE(CompassHasSouth(Compass::SOUTH_EAST));

  EXPECT_FALSE(CompassHasSouth(Compass::WEST));
  EXPECT_FALSE(CompassHasSouth(Compass::EAST));
  EXPECT_FALSE(CompassHasSouth(Compass::NORTH_WEST));
  EXPECT_FALSE(CompassHasSouth(Compass::NORTH));
  EXPECT_FALSE(CompassHasSouth(Compass::NORTH_EAST));
}

TEST(CompassTest, NorthAndSouthAreExclusive) {
  for (const Compass &compass : kAllDirections) {
    EXPECT_FALSE(CompassHasNorth(compass) && CompassHasSouth(compass));
  }
}

TEST(CompassTest, Print) {
  EXPECT_EQ("LEFT", ToString(Compass::LEFT));
  EXPECT_EQ("UPPER_LEFT", ToString(Compass::UPPER_LEFT));
  EXPECT_EQ("UPPER", ToString(Compass::UPPER));
  EXPECT_EQ("UPPER_RIGHT", ToString(Compass::UPPER_RIGHT));
  EXPECT_EQ("RIGHT", ToString(Compass::RIGHT));
  EXPECT_EQ("LOWER_RIGHT", ToString(Compass::LOWER_RIGHT));
  EXPECT_EQ("LOWER", ToString(Compass::LOWER));
  EXPECT_EQ("LOWER_LEFT", ToString(Compass::LOWER_LEFT));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
