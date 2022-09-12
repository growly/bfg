#ifndef GEOMETRY_SHAPE_H_
#define GEOMETRY_SHAPE_H_

#include <string>
#include <utility>

#include "abstract_shape.h"
#include "layer.h"
#include "point.h"
#include "manipulable.h"

namespace bfg {
namespace geometry {

class Rectangle;

// Shapes are AbstractShapes with more complete layout information.
class Shape : public AbstractShape, public Manipulable {
 public:
  Shape() = default;
  virtual ~Shape() = default;

  Shape(const Layer &layer, const std::string &net)
      : AbstractShape(layer, net) {}

  void MoveLowerLeftTo(const Point &point) override {
    ResetOrigin();
    Translate(point);
  }

  // Return the lower-left and upper-right points defining the bounding box
  // around this shape.
  virtual const Rectangle GetBoundingBox() const = 0;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_SHAPE_H_
