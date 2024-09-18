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
  Polygon() = default;

  Polygon(const std::vector<Point> &vertices) {
    for (const auto &vertex : vertices) {
      vertices_.push_back(vertex);
    }
  }

  void AddVertex(const Point &point) {
    if (!vertices_.empty() && vertices_.back() == point)
      return;
    vertices_.push_back(point);
  }

  void AddVertex(const size_t offset, const Point &point) {
    if (*(vertices_.begin() + offset) == point)
      return;
    vertices_.insert(vertices_.begin() + offset, point);
  }

  void RemoveLastVertex() {
    vertices_.pop_back();
  }

  bool Overlaps(const Rectangle &rectangle) const;
  bool HasVertex(const Point &point) const;

  std::vector<PointPair> IntersectingPoints(const Line &line) const;

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
  static std::vector<PointPair> ResolveIntersectingPointsFrom(
      const std::vector<PointOrChoice> &choices,
      const Point &reference_point);

  std::vector<Point> vertices_;
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Polygon &polygon);

}  // namespace bfg

#endif  // GEOMETRY_POLYGON_H_
