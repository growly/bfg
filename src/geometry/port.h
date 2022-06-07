#ifndef PORT_H_
#define PORT_H_

#include "layer.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {

// TODO(aryap): Is a port an abstract shape or not?
class Port : public Rectangle {
 public:
  // TODO(aryap): Wait, is a port just a rect with some other stuff? So this is
  // a rect:
  Port(const Point &centre, uint64_t width, uint64_t height,
       const Layer &layer, const std::string &net)
    {
    lower_left_ = Point(centre.x() - width / 2,
                        centre.y() - height / 2);
    upper_right_ = lower_left_ + Point(width, height);
    layer_ = layer;
    net_ = net;
  }

  Port(const Point &lower_left, const Point &upper_right,
       const Layer &layer, const std::string &net)
    : Rectangle(lower_left, upper_right, layer, net) {}
};

}  // namespace bfg

#endif  // PORT_H_
