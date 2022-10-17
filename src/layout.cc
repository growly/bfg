#include "layout.h"

#include <optional>
#include <sstream>
#include <absl/strings/str_cat.h>

#include "cell.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

using geometry::Point;
using geometry::ShapeCollection;

const std::string &Layout::NameOrParentName() const {
  if (name_ == "" && parent_cell_ != nullptr) {
    return parent_cell_->name();
  }
  return name_;
}

void Layout::MirrorY() {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    for (const auto &rectangle : shapes->rectangles) { rectangle->MirrorY(); }
    for (const auto &polygon : shapes->polygons) { polygon->MirrorY(); }
    for (const auto &port : shapes->ports) { port->MirrorY(); }
  }
  for (const auto &instance : instances_) { instance->MirrorY(); }
  for (auto &entry : named_points_) { entry.second.MirrorY(); }
}

void Layout::MirrorX() {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    for (const auto &rectangle : shapes->rectangles) { rectangle->MirrorX(); }
    for (const auto &polygon : shapes->polygons) { polygon->MirrorX(); }
    for (const auto &port : shapes->ports) { port->MirrorX(); }
  }
  for (const auto &instance : instances_) { instance->MirrorX(); }
  for (auto &entry : named_points_) { entry.second.MirrorX(); }
}

void Layout::FlipHorizontal() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorY();
  Translate(Point(bounding_box.Width() * 2, 0));
}

void Layout::FlipVertical() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  MirrorX();
  Translate(Point(0, bounding_box.Height() * 2));
}

void Layout::Translate(const Point &offset) {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    for (const auto &rectangle : shapes->rectangles) {
      rectangle->Translate(offset);
    }
    for (const auto &polygon : shapes->polygons) { polygon->Translate(offset); }
    for (const auto &port : shapes->ports) { port->Translate(offset); }
  }
  for (const auto &instance : instances_) { instance->Translate(offset); }
  for (auto &entry : named_points_) { entry.second.Translate(offset); }
}

void Layout::ResetOrigin() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

const geometry::Rectangle Layout::GetBoundingBox() const {
  std::optional<Point> start;
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    if (!shapes->rectangles.empty()) {
      start = shapes->rectangles.front()->lower_left();
    } else if (!shapes->polygons.empty()) {
      start = shapes->polygons.front()->GetBoundingBox().lower_left();
    }
  }
  if (!start && !instances_.empty()) {
    start = instances_.front()->GetBoundingBox().lower_left();
  }
  if (!start) {
    // Layout is empty.
    return geometry::Rectangle(Point(0, 0), Point(0, 0));
  }

  int64_t min_x = start.value().x();
  int64_t max_x = start.value().x();
  int64_t min_y = start.value().y();
  int64_t max_y = start.value().y();

  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    for (const auto &rectangle : shapes->rectangles) {
      min_x = std::min(rectangle->lower_left().x(), min_x);
      min_y = std::min(rectangle->lower_left().y(), min_y);
      max_x = std::max(rectangle->upper_right().x(), max_x);
      max_y = std::max(rectangle->upper_right().y(), max_y);
    }

    for (const auto &polygon : shapes->polygons) {
      geometry::Rectangle bounding_box = polygon->GetBoundingBox();
      const Point &lower_left = bounding_box.lower_left();
      const Point &upper_right = bounding_box.upper_right();
      min_x = std::min(lower_left.x(), min_x);
      min_y = std::min(lower_left.y(), min_y);
      max_x = std::max(upper_right.x(), max_x);
      max_y = std::max(upper_right.y(), max_y);
    }

    for (const auto &port : shapes->ports) {
      min_x = std::min(port->lower_left().x(), min_x);
      min_y = std::min(port->lower_left().y(), min_y);
      max_x = std::max(port->upper_right().x(), max_x);
      max_y = std::max(port->upper_right().y(), max_y);
    }
  }
  for (const auto &instance : instances_) {
    LOG(INFO) << "Computing bounding box for " << instance->name();
    geometry::Rectangle bounding_box = instance->GetBoundingBox();
    const Point &lower_left = bounding_box.lower_left();
    const Point &upper_right = bounding_box.upper_right();
    min_x = std::min(lower_left.x(), min_x);
    min_y = std::min(lower_left.y(), min_y);
    max_x = std::max(upper_right.x(), max_x);
    max_y = std::max(upper_right.y(), max_y);
  }

  // TODO(growly): Include saved points in bounds? No...?

  return geometry::Rectangle(Point(min_x, min_y), Point(max_x, max_y));
}

std::string Layout::Describe() const {
  std::stringstream ss;

  size_t num_layers = shapes_.size();
  size_t num_rectangles = 0;
  size_t num_polygons = 0;
  size_t num_ports = 0;

  for (const auto &entry : shapes_) {
    ShapeCollection *collection = entry.second.get();
    num_rectangles += collection->rectangles.size();
    num_polygons += collection->polygons.size();
    num_ports += collection->ports.size();
  }

  ss << "layout: " << num_layers << " layers, "
     << num_rectangles << " rectangles, "
     << num_polygons << " polygons, "
     << num_ports << " polygons, "
     << std::endl;

  for (const auto &entry : shapes_) {
    ShapeCollection *collection = entry.second.get();
    ss << "  layer " << entry.first << std::endl;
    for (const auto &rectangle : collection->rectangles) {
      ss << "    rect " << rectangle->lower_left().x() << " "
         << rectangle->lower_left().y() << " "
         << rectangle->upper_right().x() << " "
         << rectangle->upper_right().y() << std::endl;
    }
    for (const auto &poly : collection->polygons) {
      ss << "    polygon ";
      for (const geometry::Point &point : poly->vertices()) {
        ss << "(" << point.x() << ", " << point.y() << ") ";
      }
      ss << std::endl;
    }

    if (!named_points_.empty()) {
      for (const auto &entry : named_points_) {
        ss << "named point " << entry.first << ": " << entry.second << std::endl;
      }
    }
  }

  return ss.str();
}

void Layout::SetActiveLayerByName(const std::string &name) {
  set_active_layer(physical_db_.GetLayerInfo(name).internal_layer);
}

::vlsir::raw::Layout Layout::ToVLSIRLayout() const {
  ::vlsir::raw::Layout layout_pb;

  layout_pb.set_name(parent_cell_->name());

  for (const auto &entry : shapes_) {
    const geometry::Layer &layer = entry.first;
    ShapeCollection *shape_collection = entry.second.get();
    const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);

    ::vlsir::raw::LayerShapes *layer_shapes_pb = layout_pb.add_shapes();
    layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
    layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);

    // Collect shapes by layer.
    for (const auto &rect : shape_collection->rectangles) {
      ::vlsir::raw::Rectangle *rect_pb = layer_shapes_pb->add_rectangles();
      rect_pb->mutable_lower_left()->set_x(rect->lower_left().x());
      rect_pb->mutable_lower_left()->set_y(rect->lower_left().y());
      rect_pb->set_width(rect->upper_right().x() - rect->lower_left().x());
      rect_pb->set_height(rect->upper_right().y() - rect->lower_left().y());
    }
    for (const auto &poly : shape_collection->polygons) {
      ::vlsir::raw::Polygon *poly_pb = layer_shapes_pb->add_polygons();
      for (const auto &point : poly->vertices()) {
        ::vlsir::raw::Point *point_pb = poly_pb->add_vertices();
        point_pb->set_x(point.x());
        point_pb->set_y(point.y());
      }
    }
    LOG_IF(WARNING, !shape_collection->ports.empty())
        << "vlsir does not support ports yet";
  }
  for (const auto &instance : instances_) {
    ::vlsir::raw::Instance *instance_pb = layout_pb.add_instances();
    instance_pb->set_name(instance->name());
    ::vlsir::utils::Reference cell_reference;
    cell_reference.set_local(
        instance->template_layout()->NameOrParentName());
    *instance_pb->mutable_cell() = cell_reference;
    *instance_pb->mutable_origin_location() =
        instance->lower_left().ToVLSIRPoint();
    instance_pb->set_reflect_vert(instance->reflect_vertical());
    // FIXME(aryap): VLSIR needs to make this ccw:
    instance_pb->set_rotation_clockwise_degrees(
        instance->rotation_degrees_ccw());
  }
  for (const auto &entry : named_points_) {
    ::vlsir::raw::TextElement *text = layout_pb.add_annotations();
    text->set_string(entry.first);
    text->mutable_loc()->set_x(entry.second.x());
    text->mutable_loc()->set_y(entry.second.y());
  }

  return layout_pb;
};

ShapeCollection *Layout::GetOrInsertLayerShapes(
    const geometry::Layer &layer) {
  ShapeCollection *shape_collection = GetShapeCollection(layer);
  if (shape_collection) return shape_collection;
  // Create a new one.
  shape_collection = new ShapeCollection();
  shapes_.insert({layer, std::unique_ptr<ShapeCollection>(shape_collection)});
  return shape_collection;
}

void Layout::AddPort(
    const geometry::Port &port, const std::string &net_prefix) {
  LOG_IF(FATAL, port.net().empty()) << "Can't add a port with net \"\".";
  std::string net_name = net_prefix.empty() ?
      port.net() : absl::StrCat(net_prefix, ".", port.net());
  geometry::Port *copy = new geometry::Port(port);
  copy->set_layer(active_layer_);
  copy->set_net(net_name);
  ShapeCollection *shape_collection = GetOrInsertLayerShapes(active_layer_);
  shape_collection->ports.emplace_back(copy);
  ports_by_net_[net_name].insert(copy);
}

void Layout::GetPorts(
    const std::string &net_name, std::set<geometry::Port*> *out) const {
  auto it = ports_by_net_.find(net_name);
  LOG_IF(FATAL, it == ports_by_net_.end())
      << "No port associated with net: " << net_name;
  out->insert(it->second.begin(), it->second.end());
}

void Layout::AddLayout(const Layout &other, const std::string &name_prefix) {
  // To be able to support this we'd need to make a temporary copy of all the
  // containers:
  LOG_IF(FATAL, this == &other) << "Can't add layout to itself.";
  for (const auto &entry : other.shapes_) {
    active_layer_ = entry.first;
    ShapeCollection *other_collection = entry.second.get();
    for (const auto &rectangle : other_collection->rectangles) {
      AddRectangle(*rectangle);
    }
    for (const auto &polygon : other_collection->polygons) {
      AddPolygon(*polygon);
    }
    for (const auto &port : other_collection->ports) {
      AddPort(*port, name_prefix);
    }
  }
  for (const auto &instance : other.instances_) {
    AddInstance(*instance);
  }
  for (auto &entry : other.named_points_) {
    std::string name = entry.first;
    if (name_prefix != "") {
      name = absl::StrCat(name_prefix, ".", entry.first);
    }
    SavePoint(name, entry.second);
  }
}

void Layout::MakeVia(const std::string &layer_name, const geometry::Point &centre) {
  geometry::Layer last_layer = active_layer_;
  SetActiveLayerByName(layer_name);
  int64_t via_side = physical_db_.Rules(layer_name).via_width;
  AddSquare(centre, via_side);
  active_layer_ = last_layer;
}

void Layout::GetShapesOnLayer(const geometry::Layer &layer,
                              ShapeCollection *shapes) const {
  ShapeCollection *direct = GetShapeCollection(layer);
  if (direct) {
    for (const auto &rectangle : direct->rectangles) {
      shapes->rectangles.emplace_back(new geometry::Rectangle(*rectangle));
    }
    for (const auto &polygon : direct->polygons) {
      shapes->polygons.emplace_back(new geometry::Polygon(*polygon));
    }
    for (const auto &port : direct->ports) {
      shapes->ports.emplace_back(new geometry::Port(*port));
    }
  }
  for (const auto &instance : instances_) {
    instance->GetShapesOnLayer(layer, shapes);
  }
}

ShapeCollection *Layout::GetShapeCollection(const geometry::Layer &layer) const {
  auto shapes_it = shapes_.find(layer);
  if (shapes_it != shapes_.end()) {
    return shapes_it->second.get();
  }
  return nullptr;
}

const std::set<geometry::Port*> Layout::Ports() const {
  std::set<geometry::Port*> all_ports;
  for (const auto &entry : shapes_) {
    // How is this:
    std::transform(entry.second->ports.begin(),
                   entry.second->ports.end(),
                   std::inserter(all_ports, all_ports.begin()),
                   [](const std::unique_ptr<geometry::Port> &u) { return u.get(); });
    // Nicer than this:
    //for (const auto &port : entry.second->ports) {
    //  all_ports.insert(port.get());
    //}
  }
  return all_ports;
}

void Layout::SavePoint(const std::string &name, const geometry::Point &point) {
  auto it = named_points_.find(name);
  LOG_IF(WARNING, it != named_points_.end())
      << "Saving " << name << " overrides an existing point " << it->second;
  named_points_[name] = point;
}

geometry::Point Layout::GetPoint(const std::string &name) const {
  auto it = named_points_.find(name);
  LOG_IF(FATAL, it == named_points_.end()) << "Point " << name << " not found";
  return it->second;
}

}  // namespace bfg
