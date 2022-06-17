#ifndef GEOMETRY_INSTANCE_H_
#define GEOMETRY_INSTANCE_H_

#include "point.h"

namespace bfg {

class Layout;

namespace geometry {

class Instance {
 public:
  Instance(bfg::Layout *template_cell,
           const Point &lower_left)
      : template_cell_(template_cell), lower_left_(lower_left) {}

  const std::pair<Point, Point> GetBoundingBox() const;

  bfg::Layout *template_cell() const { return template_cell_; }
  const Point &lower_left() const { return lower_left_; }

 private:
  // This is the template cell.
  bfg::Layout *template_cell_;

  Point lower_left_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_INSTANCE_H_
