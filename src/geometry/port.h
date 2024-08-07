#ifndef GEOMETRY_PORT_H_
#define GEOMETRY_PORT_H_

#include <string>

#include "layer.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

// A port, or pin, defines an access region either on the given layer or on
// adjacent layers (mapped by PhysicalPropertiesDatabase).
class Port : public Rectangle {
 public:
  Port() {}

  std::string Describe() const;

  // TODO(aryap): Wait, is a port just a rect with some other stuff? So this is
  // a rect:
  Port(const Point &centre, uint64_t width, uint64_t height,
       const Layer &layer, const std::string &net) {
    lower_left_ = Point(centre.x() - width / 2,
                        centre.y() - height / 2);
    upper_right_ = lower_left_ + Point(width, height);
    layer_ = layer;
    net_ = net;
    is_connectable_ = true;
  }

  Port(const Rectangle &from_rectangle,
       const std::string &net)
    : Rectangle(from_rectangle.lower_left(),
                from_rectangle.upper_right(),
                0,
                net) {
    is_connectable_ = true;
  }

  Port(const Rectangle &from_rectangle,
       const Layer &layer,
       const std::string &net)
    : Rectangle(from_rectangle.lower_left(),
                from_rectangle.upper_right(),
                layer,
                net) {
    is_connectable_ = true;
  }

  Port(const Point &lower_left, const Point &upper_right,
       const Layer &layer, const std::string &net)
    : Rectangle(lower_left, upper_right, layer, net) {
    is_connectable_ = true;
  }

  Port(const Port &other)
    : Rectangle(
        other.lower_left_, other.upper_right_, other.layer_, other.net_) {
    is_connectable_ = true;
  }
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Port &port);

}  // namespace bfg

#endif  // GEOMETRY_PORT_H_
