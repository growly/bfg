#ifndef GEOMETRY_PORT_H_
#define GEOMETRY_PORT_H_

#include "layer.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

// TODO(aryap): Is a port an abstract shape or not?
class Port : public Rectangle {
 public:
  Port() {}

  // TODO(aryap): Wait, is a port just a rect with some other stuff? So this is
  // a rect:
  Port(const Point &centre, uint64_t width, uint64_t height,
       const Layer &layer, const std::string &net) {
    lower_left_ = Point(centre.x() - width / 2,
                        centre.y() - height / 2);
    upper_right_ = lower_left_ + Point(width, height);
    layer_ = layer;
    net_ = net;
  }

  Port(const Rectangle &from_rectangle,
       const std::string &net)
    : Rectangle(from_rectangle.lower_left(),
                from_rectangle.upper_right(),
                0,
                net) {}

  Port(const Rectangle &from_rectangle,
       const Layer &layer,
       const std::string &net)
    : Rectangle(from_rectangle.lower_left(),
                from_rectangle.upper_right(),
                layer,
                net) {}

  Port(const Point &lower_left, const Point &upper_right,
       const Layer &layer, const std::string &net)
    : Rectangle(lower_left, upper_right, layer, net) {}

  Port(const Port &other)
    : Rectangle(other.lower_left_, other.upper_right_, other.layer_, other.net_) {}
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_PORT_H_
