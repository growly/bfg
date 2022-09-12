#ifndef GEOMETRY_POLYGON_H_
#define GEOMETRY_POLYGON_H_

#include <glog/logging.h>
#include <ostream>
#include <vector>

#include "point.h"
#include "rectangle.h"
#include "shape.h"

namespace bfg {
namespace geometry {

class Polygon : public Shape {
 public:
  Polygon() = default;

  Polygon(const std::vector<Point> &vertices) {
    for (const auto &vertex : vertices) {
      vertices_.push_back(vertex);
    }
  }

  void AddVertex(const Point &point) {
    vertices_.push_back(point);
  }

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;

  const Rectangle GetBoundingBox() const override;

  const std::vector<Point> &vertices() const { return vertices_; }

 private:
  std::vector<Point> vertices_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Polygon &polygon);

}  // namespace bfg

#endif  // GEOMETRY_POLYGON_H_
