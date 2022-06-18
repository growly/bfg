#include "label.h"

namespace bfg {
namespace geometry {

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Label &label) {
  os << "[Label " << label.anchor() << " " << label.layer() << " "
     << "";
  return os;
}

}  // namespace bfg
