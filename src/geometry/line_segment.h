#ifndef GEOMETRY_LINE_SEGMENT_H_
#define GEOMETRY_LINE_SEGMENT_H_

#include "point.h"

namespace bfg {
namespace geometry {

enum LineOrientation {
  kOther,
  kVertical,
  kHorizontal
};

enum AnchorPosition {
  kCenterAutomatic,   // Grow wide when vertical, tall when horizontal.
  kBottom,
  kTop,
  kCenterHorizontal,
  kCenterVertical,
  kLeft,
  kRight
};

struct LineSegment {
  Point end;
  uint64_t width; // 0 -> unspecified; use default.
  AnchorPosition growth_anchor;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_LINE_SEGMENT_H_
