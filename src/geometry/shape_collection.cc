#include "shape_collection.h"

#include <sstream>
#include <map>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <optional>

#include <absl/strings/str_cat.h>

#include "layer.h"
#include "polygon.h"
#include "port.h"
#include "rectangle.h"
#include "poly_line.h"
#include "../equivalent_nets.h"
#include "../physical_properties_database.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {
namespace geometry {

std::string ShapeCollection::Describe() const {
  std::stringstream ss;
  for (const auto &rectangle : rectangles_) {
    ss << "    rect " << rectangle->lower_left().x() << " "
       << rectangle->lower_left().y() << " "
       << rectangle->upper_right().x() << " "
       << rectangle->upper_right().y();
    if (rectangle->net() != "") {
      ss << " net: " << rectangle->net();
    }
    ss << std::endl;
  }
  for (const auto &poly : polygons_) {
    ss << "    polygon ";
    for (const geometry::Point &point : poly->vertices()) {
      ss << "(" << point.x() << ", " << point.y() << ") ";
    }
    if (poly->net() != "") {
      ss << " net: " << poly->net();
    }
    ss << std::endl;
  }
  for (const auto &port : ports_) {
    ss << "    port " << port->lower_left().x() << " "
       << port->lower_left().y() << " "
       << port->upper_right().x() << " "
       << port->upper_right().y();
    if (port->net() != "") {
      ss << " net: " << port->net();
    }
    ss << std::endl;
  }
  for (const auto &line : poly_lines_) {
    ss << "    poly line " << line->Describe() << std::endl;
  }
  return ss.str();
}

bool ShapeCollection::Empty() const {
  return rectangles_.empty() && polygons_.empty() && ports_.empty() &&
      poly_lines_.empty();
}

void ShapeCollection::AddConnectableShapesNotOnNets(
    const ShapeCollection &other, const EquivalentNets &nets) {
  auto is_connectable_and_not_on_net = [&](const Shape &shape) {
    return shape.is_connectable() && !nets.Contains(shape.net());
  };
  Add(other,
      is_connectable_and_not_on_net,
      is_connectable_and_not_on_net,
      is_connectable_and_not_on_net,
      is_connectable_and_not_on_net);
}

void ShapeCollection::AddConnectableShapes(const ShapeCollection &other) {
  auto is_connectable = [&](const Shape &shape) {
    return shape.is_connectable();
  };
  Add(other,
      is_connectable,
      is_connectable,
      is_connectable,
      is_connectable);
}

void ShapeCollection::AddNonConnectableShapes(const ShapeCollection &other) {
  auto is_not_connectable = [&](const Shape &shape) {
    return !shape.is_connectable();
  };
  Add(other,
      is_not_connectable,
      is_not_connectable,
      is_not_connectable,
      is_not_connectable);
}

void ShapeCollection::Add(const ShapeCollection &other) {
  auto always_true = [](const Shape &shape) { return true; };
  Add(other,
      always_true,
      always_true,
      always_true,
      always_true);
}

void ShapeCollection::Consume(ShapeCollection *other) {
  for (auto &rectangle : other->rectangles_) {
    rectangles_.emplace_back(std::move(rectangle));
  }
  for (auto &polygon : other->polygons_) {
    polygons_.emplace_back(std::move(polygon));
  }
  for (auto &port : other->ports_) {
    ports_.emplace_back(std::move(port));
  }
  for (auto &poly_line : other->poly_lines_) {
    poly_lines_.emplace_back(std::move(poly_line));
  }
}

void ShapeCollection::Add(
    const ShapeCollection &other,
    std::function<bool(const Rectangle&)> include_rectangle,
    std::function<bool(const Polygon&)> include_polygon,
    std::function<bool(const Port&)> include_port,
    std::function<bool(const PolyLine&)> include_poly_line) {
  for (const auto &rectangle : other.rectangles_) {
    if (!include_rectangle(*rectangle))
      continue;
    Rectangle *copy = new Rectangle(*rectangle);
    rectangles_.emplace_back(copy);
  }
  for (const auto &polygon : other.polygons_) {
    if (!include_polygon(*polygon))
      continue;
    Polygon *copy = new Polygon(*polygon);
    polygons_.emplace_back(copy);
  }
  for (const auto &port : other.ports_) {
    if (!include_port(*port))
      continue;
    Port *copy = new Port(*port);
    ports_.emplace_back(copy);
  }
  for (const auto &poly_line : other.poly_lines_) {
    if (!include_poly_line(*poly_line))
      continue;
    PolyLine *copy = new PolyLine(*poly_line);
    poly_lines_.emplace_back(copy);
  }
}

void ShapeCollection::MirrorY() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorY(); }
  for (const auto &polygon : polygons_) { polygon->MirrorY(); }
  for (const auto &port : ports_) { port->MirrorY(); }
  for (const auto &poly_line : poly_lines_) { poly_line->MirrorY(); }
}

void ShapeCollection::MirrorX() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorX(); }
  for (const auto &polygon : polygons_) { polygon->MirrorX(); }
  for (const auto &port : ports_) { port->MirrorX(); }
  for (const auto &poly_line : poly_lines_) { poly_line->MirrorX(); }
}

void ShapeCollection::Translate(const Point &offset) {
  for (const auto &rectangle : rectangles_) {
    rectangle->Translate(offset);
  }
  for (const auto &polygon : polygons_) { polygon->Translate(offset); }
  for (const auto &port : ports_) { port->Translate(offset); }
  for (const auto &poly_line : poly_lines_) { poly_line->Translate(offset); }
}

void ShapeCollection::Rotate(int32_t degrees_ccw) {
  for (const auto &rectangle : rectangles_) { rectangle->Rotate(degrees_ccw); }
  for (const auto &polygon : polygons_) { polygon->Rotate(degrees_ccw); }
  for (const auto &port : ports_) { port->Rotate(degrees_ccw); }
  for (const auto &poly_line : poly_lines_) { poly_line->Rotate(degrees_ccw); }
}

void ShapeCollection::ResetOrigin() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

void ShapeCollection::FlipHorizontal() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorY();
  Translate(Point(bounding_box.Width() * 2, 0));
}

void ShapeCollection::FlipVertical() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorX();
  Translate(Point(0, bounding_box.Height() * 2));
}

const Rectangle ShapeCollection::GetBoundingBox() const {
  std::optional<Point> start;
  if (!rectangles_.empty()) {
    start = rectangles_.front()->lower_left();
  } else if (!polygons_.empty()) {
    start = polygons_.front()->GetBoundingBox().lower_left();
  } else if (!ports_.empty()) {
    start = ports_.front()->GetBoundingBox().lower_left();
  }

  int64_t min_x = start.value().x();
  int64_t max_x = start.value().x();
  int64_t min_y = start.value().y();
  int64_t max_y = start.value().y();

  for (const auto &rectangle : rectangles_) {
    min_x = std::min(rectangle->lower_left().x(), min_x);
    min_y = std::min(rectangle->lower_left().y(), min_y);
    max_x = std::max(rectangle->upper_right().x(), max_x);
    max_y = std::max(rectangle->upper_right().y(), max_y);
  }

  for (const auto &polygon : polygons_) {
    geometry::Rectangle bounding_box = polygon->GetBoundingBox();
    const Point &lower_left = bounding_box.lower_left();
    const Point &upper_right = bounding_box.upper_right();
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  for (const auto &port : ports_) {
    min_x = std::min(port->lower_left().x(), min_x);
    min_y = std::min(port->lower_left().y(), min_y);
    max_x = std::max(port->upper_right().x(), max_x);
    max_y = std::max(port->upper_right().y(), max_y);
  }

  if (!poly_lines_.empty()) {
    LOG(WARNING) << "PolyLines are not accounted for in bounding boxes yet";
  }

  return Rectangle({min_x, min_y}, {max_x, max_y});
}

bool ShapeCollection::Overlaps(const Rectangle &rectangle) const {
  for (const auto &our_rectangle : rectangles_) {
    if (our_rectangle->Overlaps(rectangle))
      return true;
  }
  for (const auto &polygon : polygons_) {
    if (polygon->Overlaps(rectangle))
      return true;
  }
  for (const auto &port : ports_) {
    if (port->Overlaps(rectangle))
      return true;
  }
  LOG_IF(WARNING, !poly_lines_.empty())
      << "Will not test if poly lines overlap rectangle";
  return false;
}

namespace {

ShapeCollection *FindOrCreateCollection(
    const std::string &net,
    const geometry::Layer &layer,
    std::unordered_map<
        std::string,
        std::map<geometry::Layer,
                 std::unique_ptr<ShapeCollection>>> *by_layer_by_net) {
  auto outer_it = by_layer_by_net->find(net);
  if (outer_it == by_layer_by_net->end()) {
    auto insertion_result = by_layer_by_net->insert({
        net, std::map<geometry::Layer, std::unique_ptr<ShapeCollection>>()});
    ShapeCollection *new_collection = new ShapeCollection();
    insertion_result.first->second.insert(
        {layer, std::unique_ptr<ShapeCollection>(new_collection)});
    return new_collection;
  }
  std::map<geometry::Layer, std::unique_ptr<ShapeCollection>> &by_layer =
      outer_it->second;
  auto inner_it = by_layer.find(layer);
  if (inner_it == by_layer.end()) {
    ShapeCollection *new_collection = new ShapeCollection();
    by_layer.insert({layer, std::unique_ptr<ShapeCollection>(new_collection)});
    return new_collection;
  }
  return inner_it->second.get();
}

}   // namespace

void ShapeCollection::PrefixNetNames(
    const std::string &prefix,
    const std::string &separator) {
  for (const auto &rectangle : rectangles_) {
    if (rectangle->net() != "") {
      rectangle->set_net(absl::StrCat(prefix, separator, rectangle->net()));
    }
  }
  for (const auto &polygon : polygons_) {
    if (polygon->net() != "") {
      polygon->set_net(absl::StrCat(prefix, separator, polygon->net()));
    }
  }
  for (const auto &port : ports_) {
    if (port->net() != "") {
      port->set_net(absl::StrCat(prefix, separator, port->net()));
    }
  }
  for (const auto &poly_line : poly_lines_) {
    if (poly_line->net() != "") {
      poly_line->set_net(absl::StrCat(prefix, separator, poly_line->net()));
    }
  }
}

void ShapeCollection::CopyConnectables(
    const std::optional<Layer> expected_layer,
    std::unordered_map<
        std::string,
        std::map<geometry::Layer,
                 std::unique_ptr<ShapeCollection>>> *shapes_by_layer_by_net)
    const {
  // Collect shapes by layer.
  for (const auto &rect : rectangles_) {
    if (!rect->is_connectable()) {
      continue;
    }
    LOG_IF(FATAL,
        expected_layer.has_value() && *expected_layer != rect->layer())
        << "Expected layer mismatch: " << *expected_layer
        << " vs " << rect->layer();
    ShapeCollection *collection = FindOrCreateCollection(
        rect->net(), rect->layer(), shapes_by_layer_by_net);
    Rectangle *copy = new Rectangle();
    *copy = *rect;
    collection->rectangles_.emplace_back(copy);
  }
  for (const auto &poly : polygons_) {
    if (!poly->is_connectable()) {
      continue;
    }
    LOG_IF(FATAL,
        expected_layer.has_value() && *expected_layer != poly->layer())
        << "Expected layer mismatch: " << *expected_layer
        << " vs " << poly->layer();
    ShapeCollection *collection = FindOrCreateCollection(
        poly->net(), poly->layer(), shapes_by_layer_by_net);
    Polygon *copy = new Polygon();
    *copy = *poly;
    collection->polygons_.emplace_back(copy);
  }
  for (const auto &port : ports_) {
    if (!port->is_connectable()) {
      continue;
    }
    LOG_IF(FATAL,
        expected_layer.has_value() && *expected_layer != port->layer())
        << "Expected layer mismatch: " << *expected_layer
        << " vs " << port->layer();
    ShapeCollection *collection = FindOrCreateCollection(
        port->net(), port->layer(), shapes_by_layer_by_net);
    Port *copy = new Port();
    *copy = *port;
    collection->ports_.emplace_back(copy);
  }
}

void ShapeCollection::RemoveNets(const EquivalentNets &nets) {
  rectangles_.erase(
      std::remove_if(rectangles_.begin(), rectangles_.end(),
                     [&](const std::unique_ptr<Rectangle> &shape) {
                       return nets.Contains(shape->net());
                     }),
      rectangles_.end());
  polygons_.erase(
      std::remove_if(polygons_.begin(), polygons_.end(),
                     [&](const std::unique_ptr<Polygon> &shape) {
                       return nets.Contains(shape->net());
                     }),
      polygons_.end());
  ports_.erase(
      std::remove_if(ports_.begin(), ports_.end(),
                     [&](const std::unique_ptr<Port> &shape) {
                       return nets.Contains(shape->net());
                     }),
      ports_.end());
  poly_lines_.erase(
      std::remove_if(poly_lines_.begin(), poly_lines_.end(),
                     [&](const std::unique_ptr<PolyLine> &shape) {
                       return nets.Contains(shape->net());
                     }),
      poly_lines_.end());
}

void ShapeCollection::KeepOnlyLayers(const std::set<geometry::Layer> &layers) {
  rectangles_.erase(
      std::remove_if(rectangles_.begin(), rectangles_.end(),
                     [&](const std::unique_ptr<Rectangle> &shape) {
                       return layers.find(shape->layer()) == layers.end();
                     }),
      rectangles_.end());
  polygons_.erase(
      std::remove_if(polygons_.begin(), polygons_.end(),
                     [&](const std::unique_ptr<Polygon> &shape) {
                       return layers.find(shape->layer()) == layers.end();
                     }),
      polygons_.end());
  ports_.erase(
      std::remove_if(ports_.begin(), ports_.end(),
                     [&](const std::unique_ptr<Port> &shape) {
                       return layers.find(shape->layer()) == layers.end();
                     }),
      ports_.end());
  poly_lines_.erase(
      std::remove_if(poly_lines_.begin(), poly_lines_.end(),
                     [&](const std::unique_ptr<PolyLine> &shape) {
                       return layers.find(shape->layer()) == layers.end();
                     }),
      poly_lines_.end());
}

::vlsir::raw::LayerShapes ShapeCollection::ToVLSIRLayerShapes(
    const PhysicalPropertiesDatabase &db,
    bool include_non_pins,
    bool include_pins,
    size_t *count_out) const {
  ::vlsir::raw::LayerShapes layer_shapes_pb;

  size_t count = 0;

  // Collect shapes by layer.
  for (const auto &rect : rectangles_) {
    if ((rect->is_connectable() && !include_pins) ||
        (!rect->is_connectable() && !include_non_pins)) {
      continue;
    }
    count++;
    *layer_shapes_pb.add_rectangles() = rect->ToVLSIRRectangle(db);
  }
  for (const auto &poly : polygons_) {
    if ((poly->is_connectable() && !include_pins) ||
        (!poly->is_connectable() && !include_non_pins)) {
      continue;
    }
    ::vlsir::raw::Polygon *poly_pb = layer_shapes_pb.add_polygons();

    for (const auto &point : poly->vertices()) {
      ::vlsir::raw::Point *point_pb = poly_pb->add_vertices();
      point_pb->set_x(db.ToExternalUnits(point.x()));
      point_pb->set_y(db.ToExternalUnits(point.y()));
    }
    if (!poly->net().empty()) {
      poly_pb->set_net(poly->net());
    }
    count++;
  }
  if (!ports_.empty()) {
    LOG_EVERY_N(WARNING, 100) << "vlsir does not support ports yet";
  }

  if (count_out) {
    *count_out = count;
  }
  return layer_shapes_pb;
}

}   // namespace geometry
}   // namespace bfg
