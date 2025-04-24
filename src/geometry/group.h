#ifndef GEOMETRY_GROUP_H_
#define GEOMETRY_GROUP_H_

#include <vector>
#include <optional>

#include "rectangle.h"

namespace bfg {
namespace geometry {

class Polygon;
class Port;
class PolyLine;

// A Group is a collection of shape references (well, pointers). Groups do not
// own the shape objects they refer to. They are used for convenient grouping of
// shapes that someone else owns. For a container of shapes that keeps copies,
// see ShapeCollection.
class Group {
 public:
  Group() = default;

  Group(const std::vector<Rectangle*> rectangles)
      : rectangles_(rectangles) {}

  void Add(Rectangle *rectangle);
  void Add(Polygon *polygon);
  void Add(Port *port);
  void Add(PolyLine *poly_line);

  std::optional<Rectangle> GetBoundingBox() const;

  std::vector<Rectangle*> &rectangles() {
    return rectangles_;
  }
  std::vector<Polygon*> &polygons() {
    return polygons_;
  }
  std::vector<Port*> &ports() {
    return ports_;
  }
  std::vector<PolyLine*> &poly_lines() {
    return poly_lines_;
  }

  const std::vector<Rectangle*> &rectangles() const {
    return rectangles_;
  }
  const std::vector<Polygon*> &polygons() const {
    return polygons_;
  }
  const std::vector<Port*> &ports() const {
    return ports_;
  }
  const std::vector<PolyLine*> &poly_lines() const {
    return poly_lines_;
  }

 private:
  std::vector<Rectangle*> rectangles_;
  std::vector<Polygon*> polygons_;
  std::vector<Port*> ports_;
  std::vector<PolyLine*> poly_lines_;
};

}   // namespace geometry
}   // namespace bfg

#endif    // GEOMETRY_GROUP_H_
