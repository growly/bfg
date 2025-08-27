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
    : layer_(0), net_(""), is_connectable_(false) {}
  virtual ~AbstractShape() = default;

  AbstractShape(const Layer &layer, const std::string &net)
      : layer_(layer),
        net_(net),
        is_connectable_(false) {}

  inline void SetConnectable() {
    set_is_connectable(true);
  }

  inline void SetConnectableNet(const std::string &net) {
    set_net(net);
    SetConnectable();
  }

  void set_layer(const Layer &layer) { layer_ = layer; }
  const Layer &layer() const { return layer_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

  void set_is_connectable(bool is_connectable) {
    is_connectable_ = is_connectable;
  }
  bool is_connectable() const { return is_connectable_; }

 protected:
  Layer layer_;
  std::string net_;
  // Shapes that are 'connectable' show as access ports/pins the abstract of a
  // cell. They should be associated with port nets. Routers treat these shapes
  // as sources/sinks in pathfinding.
  bool is_connectable_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_ABSTRACT_SHAPE_H_
