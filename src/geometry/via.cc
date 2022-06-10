#include "via.h"

namespace bfg {
namespace geometry {

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Via &via) {
  os << "[Via " << via.centre() << " between " << via.bottom_layer()
     << ", " << via.top_layer() << "]";
  return os;
}

}  // namespace bfg
