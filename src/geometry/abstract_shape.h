#ifndef ABSTRACT_SHAPE_H_
#define ABSTRACT_SHAPE_H_

#include <string>
#include <utility>

#include "layer.h"
#include "point.h"

namespace bfg {

class AbstractShape {
 public:
  AbstractShape() = default;
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

}   // namespace bfg

#endif  // ABSTRACT_SHAPE_H_
