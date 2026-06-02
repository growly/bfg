#ifndef GEOMETRY_LABEL_H_
#define GEOMETRY_LABEL_H_

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

std::ostream &operator<<(std::ostream &os, const Label &label);

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_LABEL_H_
