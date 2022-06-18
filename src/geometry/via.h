#ifndef GEOMETRY_VIA_H_
#define GEOMETRY_VIA_H_

#include <algorithm>
#include <ostream>

#include "layer.h"
#include "shape.h"
#include "point.h"

namespace bfg {
namespace geometry {

// Abstract representation of a via.
class Via {
 public:
  Via(const Point &centre, const Layer &first, const Layer &second)
      : centre_(centre) {
    bottom_layer_ = std::min(first, second);
    top_layer_ = std::max(first, second);
  }

  const Point &centre() const { return centre_; }
  const Layer &bottom_layer() const { return bottom_layer_; }
  const Layer &top_layer() const { return top_layer_; }

 private:
  Point centre_;
  Layer bottom_layer_;
  Layer top_layer_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Via &via);

}  // namespace bfg

#endif  // GEOMETRY_VIA_H_
