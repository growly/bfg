#ifndef GEOMETRY_POLYGON_H_
#define GEOMETRY_POLYGON_H_

#include <glog/logging.h>
#include <ostream>
#include <utility>
#include <vector>

#include "point.h"
#include "point_or_choice.h"
#include "rectangle.h"
#include "shape.h"
#include "line.h"

namespace bfg {
namespace geometry {

class Polygon : public Shape {
 public:
  static void ResolveIntersectingPointsFrom(
      const std::vector<PointOrChoice> &choices,
      const Point &reference_point,
      std::vector<std::pair<Point, Point>> *intersections);

  Polygon() = default;

  Polygon(const std::vector<Point> &vertices) {
    for (const auto &vertex : vertices) {
      vertices_.push_back(vertex);
    }
  }

  void AddVertex(const Point &point) {
    vertices_.push_back(point);
  }

  void IntersectingPoints(
      const Line &line,
      std::vector<std::pair<Point, Point>> *points) const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;
  void Rotate(int32_t degrees_ccw) override;

  const Rectangle GetBoundingBox() const override;

  const std::string Describe() const;

  const std::vector<Point> &vertices() const { return vertices_; }

 private:
  std::vector<Point> vertices_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Polygon &polygon);

}  // namespace bfg

#endif  // GEOMETRY_POLYGON_H_
