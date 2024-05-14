#ifndef GEOMETRY_LAYER_H_
#define GEOMETRY_LAYER_H_

#include <utility>
#include <cstdint>

namespace bfg {
namespace geometry {

typedef int64_t Layer;

// TODO(aryap): "inline" is redundant here I think?
inline std::pair<const Layer&, const Layer&> OrderFirstAndSecondLayers(
    const Layer &lhs, const Layer &rhs) {
  const Layer &first = lhs <= rhs ? lhs : rhs;
  const Layer &second = rhs >= lhs ? rhs : lhs;
  return std::pair<const Layer&, const Layer&>(first, second);
}

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_LAYER_H_
