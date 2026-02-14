#include "radian.h"

#include <limits>
#include <cmath>

namespace bfg {
namespace geometry {

double Radian::kPi = std::acos(-1);

bool Radian::IsEffectivelyZero(double radians) {
  return std::abs(radians) < std::numeric_limits<double>::epsilon();
}

double Radian::RadiansToDegrees(double radians) {
  return radians / Radian::kPi * 180.0;
}

double Radian::DegreesToRadians(double degrees) {
  return degrees / 180.0 * Radian::kPi;
}

}  // namespace geometry
}  // namespace bfg
