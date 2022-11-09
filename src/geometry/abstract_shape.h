#ifndef GEOMETRY_ABSTRACT_SHAPE_H_
#define GEOMETRY_ABSTRACT_SHAPE_H_

#include <string>
#include <utility>

#include "layer.h"

namespace bfg {
namespace geometry {

// This isn't actually an abstract class, it's an abstract (as in concept)
// shape. Which is confusing. Oh well.
class AbstractShape {
 public:
  AbstractShape()
    : layer_(0), net_("") {}
  virtual ~AbstractShape() = default;

  AbstractShape(const Layer &layer, const std::string &net)
      : layer_(layer),
        net_(net) {}

  void set_layer(const Layer &layer) { layer_ = layer; }
  const Layer &layer() const { return layer_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

 protected:
  Layer layer_;
  std::string net_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_ABSTRACT_SHAPE_H_
