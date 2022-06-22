#include "abstract_via.h"

namespace bfg {

std::ostream &operator<<(std::ostream &os, const AbstractVia &via) {
  os << "[Abstract Via " << via.centre() << " between " << via.bottom_layer()
     << ", " << via.top_layer() << "]";
  return os;
}

}  // namespace bfg
