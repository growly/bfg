#include "group.h"

#include "rectangle.cc"
#include "polygon.cc"
#include "poly_line.cc"
#include "port.cc"

namespace bfg {
namespace geometry {

void Group::Add(geometry::Rectangle *rectangle) {
  rectangles_.push_back(rectangle);
}

void Group::Add(geometry::Polygon *polygon) {
  polygons_.push_back(polygon);
}

void Group::Add(geometry::Port *port) {
  ports_.push_back(port);
}

void Group::Add(geometry::PolyLine *poly_line) {
  poly_lines_.push_back(poly_line);
}

}   // namespace geometry
}   // namespace bfg
