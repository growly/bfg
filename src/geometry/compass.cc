#include "compass.h"

#include <string>
#include <ostream>

#include "proto/compass_direction.pb.h"

namespace bfg {
namespace geometry {

proto::CompassDirection CompassToProtoCompassDirection(
    const Compass &compass) {
  switch (compass) {
    case Compass::WEST:
      return proto::CompassDirection::WEST;
    case Compass::NORTH_WEST:
      return proto::CompassDirection::NORTH_WEST;
    case Compass::NORTH:
      return proto::CompassDirection::NORTH;
    case Compass::NORTH_EAST:
      return proto::CompassDirection::NORTH_EAST;
    case Compass::EAST:
      return proto::CompassDirection::EAST;
    case Compass::SOUTH_EAST:
      return proto::CompassDirection::SOUTH_EAST;
    case Compass::SOUTH:
      return proto::CompassDirection::SOUTH;
    case Compass::SOUTH_WEST:
      return proto::CompassDirection::SOUTH_WEST;
  }
  return proto::CompassDirection::WEST;
}

Compass ProtoCompassDirectionToCompass(
    const proto::CompassDirection &compass_pb) {
  switch (compass_pb) {
    case proto::CompassDirection::WEST:
      return Compass::WEST;
    case proto::CompassDirection::NORTH_WEST:
      return Compass::NORTH_WEST;
    case proto::CompassDirection::NORTH:
      return Compass::NORTH;
    case proto::CompassDirection::NORTH_EAST:
      return Compass::NORTH_EAST;
    case proto::CompassDirection::EAST:
      return Compass::EAST;
    case proto::CompassDirection::SOUTH_EAST:
      return Compass::SOUTH_EAST;
    case proto::CompassDirection::SOUTH:
      return Compass::SOUTH;
    case proto::CompassDirection::SOUTH_WEST:
      return Compass::SOUTH_WEST;
  }
  return Compass::WEST;
}

bool CompassHasNorth(const Compass &compass) {
  switch (compass) {
    case Compass::NORTH_WEST:
      // Fallthrough intended.
    case Compass::NORTH:
      // Fallthrough intended.
    case Compass::NORTH_EAST:
      // Fallthrough intended.
      return true;
    default:
      return false;
  }
}

bool CompassHasSouth(const Compass &compass) {
  switch (compass) {
    case Compass::SOUTH_WEST:
      // Fallthrough intended.
    case Compass::SOUTH:
      // Fallthrough intended.
    case Compass::SOUTH_EAST:
      // Fallthrough intended.
      return true;
    default:
      return false;
  }
}

}  // namespace geometry

// NOTE(aryap): namespace geometry is not in this file, because nothing in it
// needs to be defined.

std::ostream &operator<<(std::ostream &os, const geometry::Compass &compass) {
  switch (compass) {
    case geometry::Compass::LEFT:
      os << "LEFT";
      break;
    case geometry::Compass::UPPER_LEFT:
      os << "UPPER_LEFT";
      break;
    case geometry::Compass::UPPER:
      os << "UPPER";
      break;
    case geometry::Compass::UPPER_RIGHT:
      os << "UPPER_RIGHT";
      break;
    case geometry::Compass::RIGHT:
      os << "RIGHT";
      break;
    case geometry::Compass::LOWER_RIGHT:
      os << "LOWER_RIGHT";
      break;
    case geometry::Compass::LOWER:
      os << "LOWER:";
      break;
    case geometry::Compass::LOWER_LEFT:
      os << "LOWER_LEFT:";
      break;
  }
  return os;
}

}   // namespace
