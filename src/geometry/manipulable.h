#ifndef GEOMETRY_MANIPULABLE_H_
#define GEOMETRY_MANIPULABLE_H_

#include <string>
#include <utility>

#include "layer.h"

namespace bfg {
namespace geometry {

class Point;

class Manipulable {
 public:
  // Mirror in the y-axis (x = 0).
  virtual void MirrorY() = 0;

  // Mirror in the x-axis (y = 0).
  virtual void MirrorX() = 0 ;

  // Mirror in place about the central vertical axis.
  virtual void FlipHorizontal() = 0;

  // Mirror in place about the central horizontal axis.
  virtual void FlipVertical() = 0;

  virtual void Translate(const Point &offset) = 0;

  virtual void MoveLowerLeftTo(const Point &point) {
    ResetOrigin();
    Translate(point);
  }

  virtual void Rotate(int32_t degrees_ccw) {};

  // Make the lower-left point of the bounding box the origin (0, 0).
  virtual void ResetOrigin() {}
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_MANIPULABLE_H_
