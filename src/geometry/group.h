#ifndef GEOMETRY_GROUP_H_
#define GEOMETRY_GROUP_H_

#include <vector>

namespace bfg {
namespace geometry {

class Rectangle;
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

  void Add(geometry::Rectangle *rectangle);
  void Add(geometry::Polygon *polygon);
  void Add(geometry::Port *port);
  void Add(geometry::PolyLine *poly_line);

  std::vector<geometry::Rectangle*> &rectangles() {
    return rectangles_;
  }
  std::vector<geometry::Polygon*> &polygons() {
    return polygons_;
  }
  std::vector<geometry::Port*> &ports() {
    return ports_;
  }
  std::vector<geometry::PolyLine*> &poly_lines() {
    return poly_lines_;
  }

  const std::vector<geometry::Rectangle*> &rectangles() const {
    return rectangles_;
  }
  const std::vector<geometry::Polygon*> &polygons() const {
    return polygons_;
  }
  const std::vector<geometry::Port*> &ports() const {
    return ports_;
  }
  const std::vector<geometry::PolyLine*> &poly_lines() const {
    return poly_lines_;
  }

 private:
  std::vector<geometry::Rectangle*> rectangles_;
  std::vector<geometry::Polygon*> polygons_;
  std::vector<geometry::Port*> ports_;
  std::vector<geometry::PolyLine*> poly_lines_;
};

}   // namespace geometry
}   // namespace bfg

#endif    // GEOMETRY_GROUP_H_
