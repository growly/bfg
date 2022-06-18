#ifndef LABEL_H_
#define LABEL_H_

#include <algorithm>
#include <ostream>
#include <string>

#include "layer.h"
#include "shape.h"
#include "point.h"

namespace bfg {
namespace geometry {

// Abstract representation of a via.
class Label {
 public:
  Label(const Point &anchor, const Layer &layer, const std::string &text)
      : anchor_(anchor),
        layer_(layer),
        text_(text){}

  const Point &anchor() const { return anchor_; }
  const Layer &layer() const { return layer_; }
  const std::string &text() const { return text_; }

 private:
  Point anchor_;
  Layer layer_;
  std::string text_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Label &label);

}  // namespace bfg

#endif  // LABEL_H_
