#include "layout.h"

#include <sstream>

#include "geometry/point.h"
#include "geometry/layer.h"

#include "raw.pb.h"

namespace bfg {

using geometry::Point;

const std::pair<Point, Point> Layout::GetBoundingBox() const {
  Point start;
  if (!polygons_.empty()) {
    start = polygons_.front().GetBoundingBox().first;
  } else if (!instances_.empty()) {
    start = instances_.front().GetBoundingBox().first;
  } else {
    // Layout is empty.
    return std::make_pair(Point(0, 0), Point(0, 0));
  }

  int64_t min_x = start.x();
  int64_t max_x = start.x();
  int64_t min_y = start.y();
  int64_t max_y = start.y();

  for (const auto &polygon : polygons_) {
    std::pair<Point, Point> bounding_box = polygon.GetBoundingBox();
    const Point &lower_left = bounding_box.first;
    const Point &upper_right = bounding_box.second;
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  for (const auto &instance : instances_) {
    std::pair<Point, Point> bounding_box = instance.GetBoundingBox();
    const Point &lower_left = bounding_box.first;
    const Point &upper_right = bounding_box.second;
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  return std::make_pair(Point(min_x, min_y), Point(max_x, max_y));
}

std::string Layout::Describe() const {
  std::stringstream ss;

  ss << "layout: " << rectangles_.size() << " rectangles, "
     << polygons_.size() << " polygons, "
     << std::endl;
  for (const geometry::Rectangle &rectangle : rectangles_) {
    ss << "rect " << rectangle.lower_left().x() << " "
       << rectangle.lower_left().y() << " "
       << rectangle.upper_right().x() << " "
       << rectangle.upper_right().y() << std::endl;
  }

  for (const geometry::Polygon &poly : polygons_) {
    ss << "polygon ";
    for (const geometry::Point &point : poly.vertices()) {
      ss << "(" << point.x() << ", " << point.y() << ") ";
    }
    ss << std::endl;
  }

  return ss.str();
}

void Layout::SetActiveLayerByName(const std::string &name) {
  set_active_layer(physical_db_.GetLayerInfo(name).internal_layer);
}

::vlsir::raw::Layout *Layout::ToVLSIRLayout() const {
  std::unique_ptr<::vlsir::raw::Layout> layout_pb(
      new ::vlsir::raw::Layout());

  std::map<geometry::Layer, ::vlsir::raw::LayerShapes*> shapes;

  // Collect shapes by layer.
  for (const auto &rect : rectangles_) {
    ::vlsir::raw::LayerShapes *layer_shapes_pb =
        GetOrInsertLayerShapes(rect.layer(), &shapes);
    ::vlsir::raw::Rectangle *rect_pb = layer_shapes_pb->add_rectangles();
    rect_pb->mutable_lower_left()->set_x(rect.lower_left().x());
    rect_pb->mutable_lower_left()->set_y(rect.lower_left().y());
    rect_pb->set_width(rect.upper_right().x() - rect.lower_left().x());
    rect_pb->set_height(rect.upper_right().y() - rect.lower_left().y());
  }
  for (const auto &poly : polygons_) {
    ::vlsir::raw::LayerShapes *layer_shapes_pb =
        GetOrInsertLayerShapes(poly.layer(), &shapes);
    ::vlsir::raw::Polygon *poly_pb = layer_shapes_pb->add_polygons();
    for (const auto &point : poly.vertices()) {
      ::vlsir::raw::Point *point_pb = poly_pb->add_vertices();
      point_pb->set_x(point.x());
      point_pb->set_y(point.y());
    }
  }
  LOG_IF(FATAL, !ports_.empty()) << "ports not yet written";
  LOG_IF(FATAL, !instances_.empty()) << "instances not yet written";

  for (const auto pair : shapes) {
    *layout_pb->add_shapes() = *pair.second;
  }

  return layout_pb.release();
};

::vlsir::raw::LayerShapes *Layout::GetOrInsertLayerShapes(
    const geometry::Layer &layer,
    std::map<geometry::Layer, ::vlsir::raw::LayerShapes*> *shapes) const {
  auto shapes_it = shapes->find(layer);
  if (shapes_it != shapes->end()) {
    return shapes_it->second;
  }
  ::vlsir::raw::LayerShapes *layer_shapes_pb =
      new ::vlsir::raw::LayerShapes();
  const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);
  layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
  layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);
  shapes->insert({layer, layer_shapes_pb});
  return layer_shapes_pb;
}

}  // namespace bfg
