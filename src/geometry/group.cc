#include "group.h"

#include "rectangle.cc"
#include "polygon.cc"
#include "poly_line.cc"
#include "port.cc"

namespace bfg {
namespace geometry {

void Group::Add(Rectangle *rectangle) {
  rectangles_.push_back(rectangle);
}

void Group::Add(Polygon *polygon) {
  polygons_.push_back(polygon);
}

void Group::Add(Port *port) {
  ports_.push_back(port);
}

void Group::Add(PolyLine *poly_line) {
  poly_lines_.push_back(poly_line);
}

std::optional<Rectangle> Group::GetBoundingBox() const {
  std::optional<Rectangle> overall;
  for (Rectangle *rectangle : rectangles_) {
    if (!overall) {
      overall = *rectangle;
      continue;
    }
    overall->ExpandToCover(*rectangle);
  }
  for (Polygon *polygon : polygons_) {
    Rectangle bounding_box = polygon->GetBoundingBox();
    if (!overall) {
      overall = bounding_box;
      continue;
    }
    overall->ExpandToCover(bounding_box);
  }
  for (Port *port : ports_) {
    Rectangle bounding_box = port->GetBoundingBox();
    if (!overall) {
      overall = bounding_box;
      continue;
    }
    overall->ExpandToCover(bounding_box);
  }
  LOG_IF(WARNING, !poly_lines_.empty())
      << "Not sure how to compute bounding box of PolyLines in this Group - "
      << "maybe inflate them first? They are being ignored for now.";
  return overall;
}

}   // namespace geometry
}   // namespace bfg
