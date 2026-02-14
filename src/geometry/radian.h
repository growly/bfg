#ifndef GEOMETRY_RADIAN_H_
#define GEOMETRY_RADIAN_H_

#include "point.h"

namespace bfg {
namespace geometry {

class Radian {
 public:
  static double kPi;
  static bool IsEffectivelyZero(double radians);
  static double RadiansToDegrees(double radians);
  static double DegreesToRadians(double radians);
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_RADIAN_H_
