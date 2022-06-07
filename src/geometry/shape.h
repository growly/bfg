#ifndef SHAPE_H_
#define SHAPE_H_

#include <string>
#include <utility>

#include "abstract_shape.h"
#include "layer.h"
#include "point.h"

namespace bfg {

// Shapes are AbstractShapes with more complete layout information.
class Shape : public AbstractShape {
 public:
  Shape() = default;
  virtual ~Shape() = default;

  Shape(const Layer &layer, const std::string &net)
      : AbstractShape(layer, net) {}

  // Return the lower-left and upper-right points defining the bounding box
  // around this shape.
  virtual const std::pair<Point, Point> GetBoundingBox() const = 0;
};

}   // namespace bfg

#endif  // SHAPE_H_
