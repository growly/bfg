#include "port.h"

#include <sstream>
#include <string>

#include "point.h"

namespace bfg {

namespace geometry {

std::string Port::Describe() const {
  std::stringstream ss;
  ss << "[Port " << net_ << " " << lower_left_ << " " << upper_right_ << "]";
  return ss.str();
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Port &port) {
  return os << port.Describe();
}

}  // namespace bfg
