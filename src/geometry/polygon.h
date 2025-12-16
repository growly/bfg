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

  // TODO(aryap): It would also be useful to have an Overlaps(...) method to
  // test for collision with another polygon. It shouldn't be That Hard (TM):
  // - Do the bounding boxes collide? If not, stop. If so,
  // - Do any of the lines of the first polygon cross any of the lines of the
  // second polygon?
  // - Are any of the points of the first polygon interior to the second
  // polygon?
  bool Overlaps(const Rectangle &rectangle) const;
  bool HasVertex(const Point &point) const;

  bool Intersects(const Point &point) const;
  bool Intersects(const Point &point, int64_t margin) const;

  std::vector<PointPair> IntersectingPoints(const Line &line) const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;
  void Rotate(int32_t degrees_ccw) override;

  // TODO(aryap): It would be very useful to have a "Widen" or "Inflate" method
  // that will add some width to the polygon in every direction. This is useful
  // for hit-testing collisions within some margin. We can employ basically the
  // same algorithm as for poly-line inflation.

  const Rectangle GetBoundingBox() const override;

  const std::string Describe() const;

  const std::vector<Point> &vertices() const { return vertices_; }

 private:
  static std::vector<PointPair> ResolveIntersectingPointsFrom(
      const std::vector<PointOrChoice> &choices,
      const Point &reference_point);

  std::vector<Point> vertices_;
};

bool operator==(const Polygon &lhs, const Polygon &rhs);

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Polygon &polygon);

}  // namespace bfg

#endif  // GEOMETRY_POLYGON_H_
