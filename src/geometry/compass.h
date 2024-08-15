#ifndef GEOMETRY_COMPASS_H_
#define GEOMETRY_COMPASS_H_

#include <ostream>

namespace bfg {
namespace geometry {

enum class Compass : uint8_t {
  WEST        = 0,
  NORTH_WEST  = 1,
  NORTH       = 2,
  NORTH_EAST  = 3,
  EAST        = 4,
  SOUTH_EAST  = 5,
  SOUTH       = 6,
  SOUTH_WEST  = 7,
  // These names alias the above set:
  LEFT        = 0,
  UPPER_LEFT  = 1,
  UPPER       = 2,
  UPPER_RIGHT = 3,
  RIGHT       = 4,
  LOWER_RIGHT = 5,
  LOWER       = 6,
  LOWER_LEFT  = 7
};

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os,
    const geometry::Compass &compass);

}  // namespace bfg

#endif  // GEOMETRY_COMPASS_H_
