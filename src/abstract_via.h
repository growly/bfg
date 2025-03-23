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
      : add_port_on_top_(std::nullopt),
        centre_(centre) {
    bottom_layer_ = std::min(first, second);
    top_layer_ = std::max(first, second);
  }

  const std::optional<std::string> &port_on_top() {
    return add_port_on_top_;
  }
  void set_port_on_top(const std::string &label) {
    add_port_on_top_ = label;
  }

  const geometry::Point &centre() const { return centre_; }
  const geometry::Layer &bottom_layer() const { return bottom_layer_; }
  const geometry::Layer &top_layer() const { return top_layer_; }

 private:
  std::optional<std::string> add_port_on_top_;
  geometry::Point centre_;
  geometry::Layer bottom_layer_;
  geometry::Layer top_layer_;
};

std::ostream &operator<<(std::ostream &os, const AbstractVia &via);

}  // namespace bfg

#endif  // ABSTRACT_VIA_H_
