#ifndef GEOMETRY_PORT_H_
#define GEOMETRY_PORT_H_

#include <string>
#include <set>

#include "layer.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

class Port;
typedef bool (*PtrPortCompare)(const Port *const, const Port *const);
typedef std::set<Port*, PtrPortCompare> PortSet;

// A port, or pin, defines an access region either on the given layer or on
// adjacent layers (mapped by PhysicalPropertiesDatabase).
class Port : public Rectangle {
 public:
  static bool Compare(const Port &lhs, const Port &rhs);
  static bool Compare(
      const std::unique_ptr<Port> &lhs, const std::unique_ptr<Port> &rhs);
  static bool Compare(const Port *const lhs, const Port *const rhs);

  static inline PortSet MakePortSet() {
    return PortSet(Port::Compare);
  }
  static PortSet MakePortSet(const std::set<const Port*> &ports);

  Port() {}
  ~Port() {}

  std::string Describe() const;
  static std::string DescribePorts(const std::vector<geometry::Port*> &ports);
  static std::string DescribePorts(const PortSet &ports);

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
