#include "instance.h"

#include <utility>

#include <glog/logging.h>

#include "../layout.h"
#include "point.h"

namespace bfg {

namespace geometry {

const std::pair<Point, Point> Instance::GetBoundingBox() const {
  LOG_IF(FATAL, template_cell_ == nullptr)
      << "Why does this Instance object have no template_cell set?";
  std::pair<Point, Point> template_bb = template_cell_->GetBoundingBox();
  
  return std::make_pair(template_bb.first + lower_left_,
                        template_bb.second + lower_left_);
}

}  // namespace geometry
}  // namespace bfg
