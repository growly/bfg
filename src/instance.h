#ifndef INSTANCE_H_
#define INSTANCE_H_

#include "geometry/point.h"

namespace bfg {

class Layout;

class Instance {
 public:
  Instance(Layout *template_cell,
           const geometry::Point &lower_left)
      : template_cell_(template_cell), lower_left_(lower_left) {}

  const std::pair<geometry::Point, geometry::Point> GetBoundingBox() const;

  Layout *template_cell() const { return template_cell_; }
  const geometry::Point &lower_left() const { return lower_left_; }

 private:
  // This is the template cell.
  Layout *template_cell_;

  geometry::Point lower_left_;
};

}  // namespace bfg

#endif  // INSTANCE_H_
