#include "layout.h"

#include <optional>
#include <sstream>
#include <absl/strings/str_cat.h>

#include "cell.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"
#include "poly_line_inflator.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

using geometry::Point;
using geometry::ShapeCollection;

geometry::Polygon *Layout::AddPolyLine(const geometry::PolyLine &line) {
  PolyLineInflator inflator(physical_db_);
  std::optional<geometry::Polygon> polygon = inflator.InflatePolyLine(line);
  if (polygon) {
    LOG(INFO) << "polygon: " << polygon->Describe();
    return AddPolygon(*polygon);
  } else {
    return nullptr;
  }
}

const std::string &Layout::NameOrParentName() const {
  if (name_ == "" && parent_cell_ != nullptr) {
    return parent_cell_->name();
  }
  return name_;
}

void Layout::MirrorY() {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    shapes->MirrorY();
  }
  for (const auto &instance : instances_) { instance->MirrorY(); }
  for (auto &entry : named_points_) { entry.second.MirrorY(); }
  if (tiling_bounds_) {
    tiling_bounds_->MirrorY();
  }
}

void Layout::MirrorX() {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    shapes->MirrorX();
  }
  for (const auto &instance : instances_) { instance->MirrorX(); }
  for (auto &entry : named_points_) { entry.second.MirrorX(); }
  if (tiling_bounds_) {
    tiling_bounds_->MirrorX();
  }
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
    shapes->Translate(offset);
  }
  for (const auto &instance : instances_) { instance->Translate(offset); }
  for (auto &entry : named_points_) { entry.second.Translate(offset); }
  if (tiling_bounds_) {
    tiling_bounds_->Translate(offset);
  }
}

void Layout::ResetX() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate({-bounding_box.lower_left().x(), 0});
}

void Layout::ResetY() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate({0, -bounding_box.lower_left().y()});
}

void Layout::ResetToTilingBounds() {
  geometry::Rectangle tiling_bounds = GetTilingBounds();
  Translate(-tiling_bounds.lower_left());
}

void Layout::ResetOrigin() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

const geometry::Rectangle Layout::GetBoundingBox() const {
  std::optional<Point> start;
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    if (!shapes->rectangles().empty()) {
      start = shapes->rectangles().front()->lower_left();
    } else if (!shapes->polygons().empty()) {
      start = shapes->polygons().front()->GetBoundingBox().lower_left();
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
    const geometry::Rectangle bb = shapes->GetBoundingBox();
    min_x = std::min(bb.lower_left().x(), min_x);
    min_y = std::min(bb.lower_left().y(), min_y);
    max_x = std::max(bb.upper_right().x(), max_x);
    max_y = std::max(bb.upper_right().y(), max_y);
  }
  for (const auto &instance : instances_) {
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
    num_rectangles += collection->rectangles().size();
    num_polygons += collection->polygons().size();
    num_ports += collection->ports().size();
  }

  ss << "layout: " << num_layers << " layers, "
     << num_rectangles << " rectangles, "
     << num_polygons << " polygons, "
     << num_ports << " polygons, "
     << std::endl;

  for (const auto &entry : shapes_) {
    ShapeCollection *collection = entry.second.get();
    ss << "  layer " << entry.first << std::endl;
    ss << collection->Describe();
  }
  if (!named_points_.empty()) {
    for (const auto &entry : named_points_) {
      ss << "named point " << entry.first << ": " << entry.second << std::endl;
    }
  }

  return ss.str();
}

void Layout::SetActiveLayerByName(const std::string &name) {
  set_active_layer(physical_db_.GetLayerInfo(name).internal_layer);
}

::vlsir::raw::Abstract Layout::ToVLSIRAbstract(
      std::optional<geometry::Layer> top_layer) const {
  //
  // HACK HACK HACK

  top_layer = physical_db_.GetLayer("li.drawing");

  //  
  //
  ::vlsir::raw::Abstract abstract_pb;

  abstract_pb.set_name(parent_cell_->name());

  *abstract_pb.mutable_outline() =
      GetTilingBounds().ToVLSIRPolygon(physical_db_);

  std::unordered_map<
      std::string,
      std::map<geometry::Layer, std::unique_ptr<ShapeCollection>>>
          pins_by_layer_by_net;

  for (const auto &entry : shapes_) {
    const geometry::Layer &layer = entry.first;
    if (top_layer && *top_layer != layer) {
      continue;
    }
    ShapeCollection *shape_collection = entry.second.get();

    shape_collection->CopyPins(layer, &pins_by_layer_by_net);

    size_t num_obstructions = 0;
    ::vlsir::raw::LayerShapes obstructions = 
        shape_collection->ToVLSIRLayerShapes(
            physical_db_, true, false, &num_obstructions);

    if (num_obstructions > 0) {
      ::vlsir::raw::LayerShapes *layer_shapes_pb = abstract_pb.add_blockages();
      *layer_shapes_pb = obstructions;

      const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);
      layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
      layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);
    }
  }

  // This should include all Port objects explicitly created, since they should
  // be assigned is_pin = true and will have an associated net.
  for (const auto &entry : pins_by_layer_by_net) {
    const std::string &net = entry.first;
    ::vlsir::raw::AbstractPort *port_pb = abstract_pb.add_ports();
    port_pb->set_net(net);
    for (const auto &sub_entry : entry.second) {
      const geometry::Layer &layer = sub_entry.first;
      ShapeCollection *shape_collection = sub_entry.second.get();

      size_t num_pins = 0;

      ::vlsir::raw::LayerShapes *layer_shapes_pb = port_pb->add_shapes();
      *layer_shapes_pb =
          shape_collection->ToVLSIRLayerShapes(
              physical_db_, false, true, &num_pins);

      const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);
      layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
      layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);
    }
  }

  // TODO(aryap):
  // Collect explicit Port objects and any shape that is labelled as a pin:
  //for (const auto &entry : ports_by_net_) {
  //  const std::string &net = entry.first;
  //  ::vlsir::raw::AbstractPort *port_pb = abstract_pb.add_ports();
  //  port_pb->set_net(net);
  //  for (const auto &port : entry.second) {
  //    ::vlsir::raw::LayerShapes *layer_shapes_pb = port_pb->add_shapes();
  //    ::vlsir::raw::Rectangle *rect_pb = layer_shapes_pb->add_rectangles();
  //    *rect_pb = port->ToVLSIRRectangle();
  //    const geometry::Layer &layer = port->layer();
  //    const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);
  //    layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
  //    layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);
  //  }
  //}

  return abstract_pb;
}

::vlsir::raw::Layout Layout::ToVLSIRLayout() const {
  ::vlsir::raw::Layout layout_pb;

  layout_pb.set_name(parent_cell_->name());

  for (const auto &entry : shapes_) {
    const geometry::Layer &layer = entry.first;
    ShapeCollection *shape_collection = entry.second.get();
    const LayerInfo &layer_info = physical_db_.GetLayerInfo(layer);

    ::vlsir::raw::LayerShapes *layer_shapes_pb = layout_pb.add_shapes();
    *layer_shapes_pb = shape_collection->ToVLSIRLayerShapes(physical_db_);

    layer_shapes_pb->mutable_layer()->set_number(layer_info.gds_layer);
    layer_shapes_pb->mutable_layer()->set_purpose(layer_info.gds_datatype);
  }
  for (const auto &instance : instances_) {
    ::vlsir::raw::Instance *instance_pb = layout_pb.add_instances();
    instance_pb->set_name(instance->name());
    ::vlsir::utils::Reference cell_reference;
    cell_reference.set_local(
        instance->template_layout()->NameOrParentName());
    *instance_pb->mutable_cell() = cell_reference;
    *instance_pb->mutable_origin_location() =
        instance->lower_left().ToVLSIRPoint(physical_db_);
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

geometry::Rectangle *Layout::AddRectangleAsPort(
    const geometry::Rectangle &rectangle,
    const std::string &net,
    const std::string &net_prefix) {
  geometry::Rectangle *installed = AddRectangle(rectangle);
  AddPort({rectangle, net}, net_prefix);
  return installed;
}

void Layout::AddPort(const geometry::Port &port,
                     const std::string &net_prefix) {
  LOG_IF(FATAL, port.net().empty()) << "Can't add a port with net \"\".";
  std::string net_name = net_prefix.empty() ?
      port.net() : absl::StrCat(net_prefix, ".", port.net());
  geometry::Port *copy = new geometry::Port(port);
  copy->set_layer(active_layer_);
  copy->set_net(net_name);
  ShapeCollection *shape_collection = GetOrInsertLayerShapes(active_layer_);
  shape_collection->ports().emplace_back(copy);
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
    for (const auto &rectangle : other_collection->rectangles()) {
      AddRectangle(*rectangle);
    }
    for (const auto &polygon : other_collection->polygons()) {
      AddPolygon(*polygon);
    }
    for (const auto &port : other_collection->ports()) {
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

geometry::Rectangle *Layout::MakeVia(
    const std::string &layer_name, const geometry::Point &centre) {
  SetActiveLayerByName(layer_name);
  int64_t via_side = physical_db_.Rules(layer_name).via_width;
  geometry::Rectangle *via = AddSquare(centre, via_side);
  RestoreLastActiveLayer();
  return via;
}

void Layout::MakePort(
    const std::string &net_name,
    const geometry::Point &centre,
    const std::string &layer_name) {
  int64_t layer = layer_name == "" ? 0 : physical_db_.GetLayer(layer_name);
  int64_t via_side = layer_name == "" ?
    100 : physical_db_.Rules(layer_name).via_width;
  geometry::Port port = geometry::Port(
      centre, via_side, via_side, layer, net_name);
  AddPort(port);
}

void Layout::GetShapesOnLayer(const geometry::Layer &layer,
                              ShapeCollection *shapes) const {
  ShapeCollection *direct = GetShapeCollection(layer);
  // TODO(aryap): This is just
  // shapes->Add(direct);
  if (direct) {
    for (const auto &rectangle : direct->rectangles()) {
      shapes->rectangles().emplace_back(new geometry::Rectangle(*rectangle));
    }
    for (const auto &polygon : direct->polygons()) {
      shapes->polygons().emplace_back(new geometry::Polygon(*polygon));
    }
    for (const auto &port : direct->ports()) {
      shapes->ports().emplace_back(new geometry::Port(*port));
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

void Layout::GetInstancesByName(
    std::unordered_map<std::string, geometry::Instance *const> *mapping)
    const {
  for (const auto &instance : instances_) {
    if (instance->name().empty()) {
      continue;
    }
    mapping->insert({instance->name(), instance.get()});
  }
}

void Layout::GetAllPorts(std::set<geometry::Port*> *ports) const {
  for (const auto &instance : instances_) {
    instance->GetInstancePorts(ports);
  }
}

void Layout::GetAllPortsExceptNamed(
    std::set<geometry::Port*> *ports,
    const std::string &named) const {
  for (const auto &instance : instances_) {
    for (const auto &entry : instance->instance_ports()) {
      const std::string &port_name = entry.first;
      if (port_name == named)
        continue;
      for (const auto &uniq : entry.second) {
        ports->insert(uniq.get());
      }
    }
  }
}

const std::set<geometry::Port*> Layout::Ports() const {
  std::set<geometry::Port*> all_ports;
  for (const auto &entry : shapes_) {
    // How is this:
    std::transform(entry.second->ports().begin(),
                   entry.second->ports().end(),
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

void Layout::SavePoints(std::map<const std::string, const Point> named_points) {
  for (const auto &entry : named_points) {
    SavePoint(entry.first, entry.second);
  }
};

geometry::Point Layout::GetPoint(const std::string &name) const {
  auto it = named_points_.find(name);
  LOG_IF(FATAL, it == named_points_.end()) << "Point " << name << " not found";
  return it->second;
}

}  // namespace bfg
