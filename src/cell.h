#ifndef CELL_H_
#define CELL_H_

#include <string>
#include <vector>

#include "instance.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"

namespace bfg {

class Cell {
 public:
  Cell() = default;
  Cell(const std::string &name) : name_(name) {}

  void AddRectangle(const geometry::Rectangle &rectangle) {
    rectangles_.push_back(rectangle);
  }
  void AddPolygon(const geometry::Polygon &polygon) {
    polygons_.push_back(polygon);
  }
  void AddInstance(const Instance &instance) { instances_.push_back(instance); }
  void AddPort(const geometry::Port &port) { ports_.push_back(port); }

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  const std::vector<geometry::Rectangle> &rectangles() const {
    return rectangles_;
  }
  const std::vector<geometry::Polygon> &polygons() const { return polygons_; }
  const std::vector<Instance> &instances() const {
    return instances_;
  }
  const std::vector<geometry::Port> &ports() const { return ports_; }

  const std::pair<geometry::Point, geometry::Point> GetBoundingBox() const;

 private:
  std::string name_;
  std::vector<geometry::Rectangle> rectangles_;
  std::vector<geometry::Polygon> polygons_;
  std::vector<geometry::Port> ports_;

  std::vector<Instance> instances_;
};

}  // namespace bfg

#endif  // CELL_H_
