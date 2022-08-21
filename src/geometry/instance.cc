#include "instance.h"

#include <glog/logging.h>

#include "../layout.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {

namespace geometry {

const Rectangle Instance::GetBoundingBox() const {
  LOG_IF(FATAL, template_layout_ == nullptr)
      << "Why does this Instance object have no template_layout set?";
  Rectangle template_bb = template_layout_->GetBoundingBox();
  
  return Rectangle(template_bb.lower_left() + lower_left_,
                   template_bb.upper_right() + lower_left_);
}

}  // namespace geometry
}  // namespace bfg
