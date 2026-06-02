#include "label.h"

namespace bfg {
namespace geometry {

std::ostream &operator<<(std::ostream &os, const Label &label) {
  os << "[Label " << label.anchor() << " " << label.layer() << " "
     << "";
  return os;
}

}  // namespace geometry
}  // namespace bfg
