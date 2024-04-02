#include "radian.h"

#include <limits>
#include <cmath>

namespace bfg {
namespace geometry {

bool Radian::IsEffectivelyZero(double radians) {
  return std::abs(radians) < std::numeric_limits<double>::epsilon();
}

}  // namespace geometry
}  // namespace bfg
