#ifndef ABSTRACT_VIA_H_
#define ABSTRACT_VIA_H_

#include <algorithm>
#include <ostream>

#include "geometry/layer.h"
#include "geometry/point.h"

namespace bfg {

// Abstract representation of a via.
class AbstractVia {
 public:
  AbstractVia(const geometry::Point &centre,
              const geometry::Layer &first,
              const geometry::Layer &second)
      : centre_(centre) {
    bottom_layer_ = std::min(first, second);
    top_layer_ = std::max(first, second);
  }

  const geometry::Point &centre() const { return centre_; }
  const geometry::Layer &bottom_layer() const { return bottom_layer_; }
  const geometry::Layer &top_layer() const { return top_layer_; }

 private:
  geometry::Point centre_;
  geometry::Layer bottom_layer_;
  geometry::Layer top_layer_;
};

std::ostream &operator<<(std::ostream &os, const AbstractVia &via);

}  // namespace bfg

#endif  // ABSTRACT_VIA_H_
