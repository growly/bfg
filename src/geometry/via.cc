#include "via.h"

namespace bfg {

std::ostream &operator<<(std::ostream &os, const Via &via) {
  os << "[Via " << via.centre() << " between " << via.bottom_layer()
     << ", " << via.top_layer() << "]";
  return os;
}

}   // namespace bfg
