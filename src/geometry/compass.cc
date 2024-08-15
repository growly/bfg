#include "compass.h"

#include <string>
#include <ostream>

namespace bfg {

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
