#include "layout.h"

#include <optional>
#include <sstream>
#include <absl/strings/str_cat.h>
#include <glog/logging.h>

#include "cell.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/port.h"
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
    VLOG(20) << "Inflated line: " << line
             << " to polygon: " << polygon->Describe();
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

void Layout::Rotate(int32_t degrees_ccw) {
  for (const auto &entry : shapes_) {
    ShapeCollection *shapes = entry.second.get();
    shapes->Rotate(degrees_ccw);
  }
  for (const auto &instance : instances_) { instance->Rotate(degrees_ccw); }
  for (auto &entry : named_points_) { entry.second.Rotate(degrees_ccw); }
  if (tiling_bounds_) {
    tiling_bounds_->Rotate(degrees_ccw);
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

const geometry::Rectangle Layout::GetBoundingBoxOrDie(
    const geometry::Layer &layer) const {
  ShapeCollection *layer_shapes = GetShapeCollection(layer);
  LOG_IF(FATAL, !layer_shapes) << "Layer not found: " << layer;
  return layer_shapes->GetBoundingBox();
}

const geometry::Rectangle Layout::GetBoundingBoxByNameOrDie(
    const std::string &layer_name) const {
  return GetBoundingBoxOrDie(physical_db_.GetLayer(layer_name));
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

    shape_collection->CopyConnectables(layer, &pins_by_layer_by_net);

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
  // be assigned is_connectable = true and will have an associated net.
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
  installed->set_net(net);
  AddPort({rectangle, net}, net_prefix);
  return installed;
}

geometry::Rectangle *Layout::AddSquareAsPort(
    const geometry::Point &point,
    uint64_t side_width,
    const std::string &net,
    const std::string &net_prefix) {
  geometry::Rectangle *installed = AddSquare(point, side_width);
  AddPort({*installed, net}, net_prefix);
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
  AddPortByNet(net_name, copy);
}

void Layout::AddPortByNet(const std::string &name, geometry::Port *port) {
  auto it = ports_by_net_.find(name);
  if (it != ports_by_net_.end()) {
    it->second.insert(port);
    return;
  }
  auto insertion = ports_by_net_.insert({name, geometry::Port::MakePortSet()});
  if (insertion.second) {
    insertion.first->second.insert(port);
  }
}

void Layout::DeletePorts(const std::string &name) {
  // Get the set of ports.
  geometry::PortSet matching_ports;
  GetPorts(name, &matching_ports);
  for (const geometry::Port *port : matching_ports) {
    delete port;
  }
  ports_by_net_.erase(name);
}

void Layout::GetPorts(
    const std::string &net_name, geometry::PortSet *out) const {
  auto it = ports_by_net_.find(net_name);
  LOG_IF(FATAL, it == ports_by_net_.end())
      << "No port associated with net: " << net_name;
  out->insert(it->second.begin(), it->second.end());
}

void Layout::ConsumeLayout(Layout *other, const std::string &name_prefix) {
  for (auto &entry : other->shapes_) {
    active_layer_ = entry.first;
    ShapeCollection *other_collection = entry.second.get();
    if (name_prefix != "") {
      // Shapes are modified in place.
      // TODO(aryap): How to handle exclusion of prefixes on e.g. global nets
      // here? Is it necessary?
      other_collection->PrefixNetNames(name_prefix, ".");
    }

    ShapeCollection *layer_shapes = GetOrInsertLayerShapes(active_layer_);
    layer_shapes->Consume(other_collection);
  }
  for (auto &instance : other->instances_) {
    instances_.push_back(std::move(instance));
  }

  // Points are copied since they are not stored by pointer.
  for (auto &entry : other->named_points_) {
    std::string name = entry.first;
    if (name_prefix != "") {
      name = absl::StrCat(name_prefix, ".", entry.first);
    }
    SavePoint(name, entry.second);
  }
}

void Layout::AddLayout(const Layout &other,
                       const std::string &name_prefix,
                       bool include_ports) {
  // To be able to support this we'd need to make a temporary copy of all the
  // containers:
  LOG_IF(FATAL, this == &other) << "Can't add layout to itself.";
  std::unique_ptr<ShapeCollection> owner_of_copy;
  for (const auto &entry : other.shapes_) {
    active_layer_ = entry.first;
    ShapeCollection *other_collection = entry.second.get();

    // If a shape's net appears in global_nets_ in the other Layout, no prefix
    // should be applied.

    if (name_prefix != "") {
      owner_of_copy.reset(new ShapeCollection(*other_collection));
      other_collection = owner_of_copy.get();
      // This is now the copy!
      other_collection->PrefixNetNames(name_prefix, ".", other.global_nets());
    }
    for (const auto &rectangle : other_collection->rectangles()) {
      AddRectangle(*rectangle);
    }
    for (const auto &polygon : other_collection->polygons()) {
      AddPolygon(*polygon);
    }
    if (include_ports) {
      for (const auto &port : other_collection->ports()) {
        AddPort(*port);
      }
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

void Layout::AlignPointTo(const geometry::Point &reference,
                          const geometry::Point &target) {
  Point diff = target - reference;
  Translate(diff);
}

geometry::Rectangle *Layout::MakeVia(
    const geometry::Layer &layer,
    const geometry::Point &centre,
    const std::optional<std::string> &net) {
  set_active_layer(layer);
  int64_t via_side = physical_db_.Rules(layer).via_width;
  geometry::Rectangle *via = AddSquare(centre, via_side);
  if (net) {
    via->set_net(*net);
  }
  RestoreLastActiveLayer();
  return via;
}

void Layout::DistributeVias(const geometry::Layer &via_layer,
                            const geometry::Point &start,
                            const geometry::Point &end,
                            const std::optional<std::string> &net) {
  const PhysicalPropertiesDatabase &db = physical_db_;
  const auto &rules = db.Rules(via_layer);

  // Assume vias are square, so take max side dimension:
  int64_t side = std::max(rules.via_width, rules.via_height);
  int64_t min_separation = rules.min_separation;

  int64_t nominal_pitch = side + min_separation;

  geometry::Line axis(start, end);
  int64_t length = axis.Length();
  // Rely on std::floor behaviour (truncating integer division).
  int64_t num_vias = length / nominal_pitch;
  int64_t spacing = (length - (num_vias * side)) / num_vias;

  double distance = spacing / 2 + side / 2;
  for (int i = 0; i < num_vias; ++i) {
    geometry::Point centre = axis.PointOnLineAtDistance(distance);
    MakeVia(via_layer, centre, net);
    distance += side + spacing;
  }
}

void Layout::StampVias(const geometry::Layer &layer,
                       const geometry::Point &start,
                       const geometry::Point &end,
                       int64_t pitch,
                       const std::optional<std::string> &net) {
  const PhysicalPropertiesDatabase &db = physical_db_;
  const auto &rules = db.Rules(layer);

  // Assume vias are square, so take max side dimension:
  int64_t side = std::max(rules.via_width, rules.via_height);

  geometry::Line axis(start, end);
  double length = axis.Length();

  double distance = (pitch - side) / 2;
  while (distance + side <= length) {
    geometry::Point centre = axis.PointOnLineAtDistance(distance + side / 2);
    MakeVia(layer, centre, net);
    distance += pitch;
  }
}

void Layout::MakePin(
    const std::string &net_name,
    const geometry::Point &centre,
    const std::string &layer_name) {
  int64_t layer = layer_name == "" ? 0 : physical_db_.GetLayer(layer_name);
  int64_t via_side = std::max(
      physical_db_.Rules(layer_name).via_height,
      physical_db_.Rules(layer_name).via_width);
  geometry::Port port = geometry::Port(
      centre, via_side, via_side, layer, net_name);

  ScopedLayer sl(this, layer);
  AddPort(port);
  AddRectangle(port);
}

void Layout::MakePort(
    const std::string &net_name,
    const geometry::Point &centre,
    const std::string &layer_name) {
  int64_t layer = layer_name == "" ? 0 : physical_db_.GetLayer(layer_name);
  int64_t via_side = layer_name == "" ?
    100 : std::max(
        physical_db_.Rules(layer_name).via_height,
        physical_db_.Rules(layer_name).via_width);
  geometry::Port port = geometry::Port(
      centre, via_side, via_side, layer, net_name);
  AddPort(port);
}

geometry::Group Layout::MakeVerticalSpineWithFingers(
    const std::string &spine_layer_name,
    const std::string &finger_layer_name,
    const std::string &net,
    const std::vector<geometry::Point> &connections,
    int64_t spine_x,
    int64_t spine_width,
    Layout *layout) {
  const PhysicalPropertiesDatabase &db = physical_db_;

  const auto &spine_layer = db.GetLayer(spine_layer_name);
  const auto &finger_layer = db.GetLayer(finger_layer_name);
  const auto &via_layer = db.GetViaLayerOrDie(spine_layer, finger_layer);

  const auto &spine_rules = db.Rules(spine_layer);
  const auto &finger_rules = db.Rules(finger_layer);

  const auto &via_rules = db.Rules(via_layer);
  const auto &spine_via_rules = db.Rules(spine_layer, via_layer);
  const auto &finger_via_rules = db.Rules(finger_layer, via_layer);

  geometry::Group created_shapes;

  // Sort points by y (the key) and remove duplicates by keeping either the
  // closest or the furthest point from spine_x.
  std::map<int64_t, geometry::Point> points;
  for (const geometry::Point &point : connections) {
    auto it = points.find(point.y());
    if (it != points.end()) {
      geometry::Point on_spine = {spine_x, point.y()};
      const geometry::Point &existing = it->second;
      // Keeps closest point:
      if (point.L1DistanceTo(on_spine) < existing.L1DistanceTo(on_spine)) {
        points[point.y()] = point;
      }
    } else {
      points[point.y()] = point;
    }
  }

  if (points.size() < 2) {
    return created_shapes;
  }

  // Draw spine.
  int64_t y_min = points.begin()->second.y();
  int64_t y_max = points.rbegin()->second.y();

  geometry::PolyLine spine_line({{spine_x, y_min}, {spine_x, y_max}});
  spine_line.SetWidth(
      std::max(spine_rules.min_width, spine_width));
  spine_line.set_min_separation(spine_rules.min_separation);
  spine_line.set_net(net);

  uint64_t via_side = std::max(via_rules.via_width, via_rules.via_height);
  uint64_t spine_bulge_width = 2 * spine_via_rules.via_overhang_wide + via_side;
  uint64_t spine_bulge_length = 2 * spine_via_rules.via_overhang + via_side;
  uint64_t finger_bulge_width =
      2 * finger_via_rules.via_overhang_wide + via_side;
  uint64_t finger_bulge_length = 2 * finger_via_rules.via_overhang + via_side;

  for (const auto &entry : points) {
    const geometry::Point &point = entry.second;
    if (point.x() == spine_x) {
      spine_line.InsertBulge(point, spine_bulge_width, spine_bulge_length);
      layout->MakeVia(via_layer, point, net);
      continue;
    }
    geometry::Point spine_via = {spine_x, point.y()};
    // Have to draw a finger!
    geometry::PolyLine finger({point, spine_via});

    finger.SetWidth(finger_rules.min_width);
    finger.set_min_separation(finger_rules.min_separation);
    finger.InsertBulge(spine_via, finger_bulge_width, finger_bulge_length);
    finger.set_net(net);

    {
      ScopedLayer sl(this, finger_layer);
      geometry::Polygon *finger_polygon = layout->AddPolyLine(finger);
      created_shapes.Add(finger_polygon);
    }

    geometry::Rectangle *via = layout->MakeVia(via_layer, spine_via, net);
    created_shapes.Add(via);

    spine_line.InsertBulge(spine_via, spine_bulge_width, spine_bulge_length);

    // TODO: do we worry about the via from the finger to the connection pin
    // here?
    // finger.InsertBulge(point, finger_bulge_width, finger_bulge_length);
  }

  ScopedLayer sl(this, spine_layer);
  geometry::Polygon *spine_metal_pour = layout->AddPolyLine(spine_line);
  created_shapes.Add(spine_metal_pour);

  return created_shapes;
}

void Layout::MakeAlternatingWire(
    const std::vector<geometry::Point> &points,
    const std::string &first_layer_name,
    const std::string &second_layer_name,
    const std::optional<std::string> &net) {
  if (points.size() < 2) {
    return;
  }

  const geometry::Layer via_layer = physical_db_.GetViaLayerOrDie(
      first_layer_name, second_layer_name);
  const geometry::Layer first_layer = physical_db_.GetLayer(first_layer_name);
  const geometry::Layer second_layer = physical_db_.GetLayer(second_layer_name);

  struct WireHopInfo {
    const geometry::Layer &layer;
    const ViaEncapInfo &encap_info;
    int64_t min_width;
    int64_t min_separation;
  };

  std::array<WireHopInfo, 2> hop_infos = {
    WireHopInfo {
      .layer = first_layer,
      .encap_info = physical_db_.TypicalViaEncap(first_layer, via_layer),
      .min_width = physical_db_.Rules(first_layer).min_width,
      .min_separation = physical_db_.Rules(first_layer).min_separation
    },
    WireHopInfo {
      .layer = second_layer,
      .encap_info = physical_db_.TypicalViaEncap(second_layer, via_layer),
      .min_width = physical_db_.Rules(second_layer).min_width,
      .min_separation = physical_db_.Rules(second_layer).min_separation
    }
  };

  size_t i = 0;
  for (auto next_it = points.begin() + 1; next_it != points.end(); ++next_it) {
    geometry::Point last = *(next_it - 1);
    geometry::Point next = *next_it;

    const WireHopInfo &hop = hop_infos[i % 2];
    ++i;  // So tempting to put this inline as i++. SO tempting.

    if (last == next) {
      ScopedLayer sl(this, hop.layer);
      // We have to put a minimum-sized metal pour here. The aspect ratio of
      // this pour will matter somewhat, but for now we don't know what it
      // should be (in the RoutingGrid we have assigned layers routing
      // directions, and that informs the aspect ratio, but here we have not).
      double min_area = physical_db_.Rules(hop.layer).min_area;
      int64_t width = std::max(hop.encap_info.width, hop.encap_info.length);
      int64_t length = std::ceil(min_area / static_cast<double>(width));
      geometry::Rectangle rectangle = geometry::Rectangle::CentredAt(
          last, width, length);
      if (net) {
        rectangle.set_net(*net);
      }
      AddRectangle(rectangle);
      continue;
    }

    ScopedLayer sl(this, hop.layer);
    geometry::PolyLine wire = geometry::PolyLine({last, next});
    wire.SetWidth(hop.min_width);
    wire.set_min_separation(hop.min_separation);
    wire.InsertBulge(last, hop.encap_info.width, hop.encap_info.length);
    wire.InsertBulge(next, hop.encap_info.width, hop.encap_info.length);
    if (net) {
      wire.set_net(*net);
    }
    AddPolyLine(wire);

    if ((next_it + 1) != points.end()) {
      // This is not a start or end hop, so we can add the via:
      MakeVia(via_layer, next, net);
    }
  }
}

geometry::Polygon *Layout::MakeWire(
    const std::vector<geometry::Point> &points,
    const std::string &wire_layer_name,
    const std::optional<std::string> &start_layer_name,
    const std::optional<std::string> &end_layer_name,
    bool start_pad_only,
    bool end_pad_only,
    const std::optional<std::string> &net) {
  std::vector<ViaToSomeLayer> vias;

  if (start_layer_name) {
    vias.push_back({
        .centre = points.front(),
        .layer_name = *start_layer_name,
        .pad_only = start_pad_only
    });
  }

  if (end_layer_name) {
    vias.push_back({
        .centre = points.back(),
        .layer_name = *end_layer_name,
        .pad_only = end_pad_only
    });
  }

  return MakeWire(points, wire_layer_name, vias, net);
}

geometry::Polygon *Layout::MakeWire(
    const std::vector<geometry::Point> &points,
    const std::string &wire_layer_name,
    const std::vector<ViaToSomeLayer> vias,
    const std::optional<std::string> &net) {
  LOG_IF(FATAL, points.empty())
      << "Why you wanna make a empty wire like that?";

  const geometry::Layer wire_layer = physical_db_.GetLayer(wire_layer_name);
  const auto &wire_rules = physical_db_.Rules(wire_layer);

  geometry::PolyLine wire(points);
  wire.SetWidth(wire_rules.min_width);
  wire.set_min_separation(wire_rules.min_separation);

  for (const auto &directive : vias) {
    const geometry::Layer destination_layer =
        physical_db_.GetLayer(directive.layer_name);
    const geometry::Layer via_layer = physical_db_.GetViaLayerOrDie(
        destination_layer, wire_layer);
    const ViaEncapInfo encap_info =
        physical_db_.TypicalViaEncap(wire_layer, via_layer);

    wire.InsertBulge(directive.centre, encap_info.width, encap_info.length);
    if (!directive.pad_only) {
      MakeVia(via_layer, directive.centre);
    }
  }

  ScopedLayer sl(this, wire_layer);
  geometry::Polygon *polygon = AddPolyLine(wire);
  if (net) {
    polygon->set_net(*net);
  }
  return polygon;
}

void Layout::Flatten(bool add_prefixes) {
  std::set<geometry::Instance*> instances_weak_copy;
  std::transform(
      instances_.begin(), instances_.end(),
      std::inserter(instances_weak_copy, instances_weak_copy.begin()),
      [](const std::unique_ptr<geometry::Instance> &uniq)
          -> geometry::Instance* {
        return uniq.get();
      });

  for (geometry::Instance *instance : instances_weak_copy) {
    Layout staging(physical_db_);

    std::string prefix = add_prefixes ? instance->name() : "";
    staging.AddLayout(*instance->template_layout(), prefix);

    instance->ApplyInstanceTransforms(&staging);

    // It is convenient to expand our tiling bounds automatically while doing
    // this:
    geometry::Rectangle::ExpandAccumulate(
        instance->GetTilingBounds(), &tiling_bounds_);

    AddLayout(staging);
  }

  // Remove old instances, noting that new instances could have been added as
  // part of the flattening.
  instances_.erase(
      std::remove_if(
          instances_.begin(), instances_.end(),
          [&](const std::unique_ptr<geometry::Instance> &uniq) {
            return instances_weak_copy.find(
                uniq.get()) != instances_weak_copy.end();
          }),
      instances_.end());
}

void Layout::LabelNet(const geometry::Point &point, const std::string &net) {
  LOG(FATAL) << "Not implemented.";
}

void Layout::CopyShapesOnLayer(const geometry::Layer &layer,
                               ShapeCollection *shapes) const {
  ShapeCollection *layer_shapes = GetShapeCollection(layer);
  if (layer_shapes) {
    shapes->Add(*layer_shapes);
  }
  for (const auto &instance : instances_) {
    instance->CopyShapesOnLayer(layer, shapes);
  }
}

void Layout::CopyNonConnectableShapesOnLayer(
    const geometry::Layer &layer,
    ShapeCollection *shapes,
    const std::optional<int64_t> &force_below_depth) const {
  ShapeCollection *layer_shapes = GetShapeCollection(layer);
  if (layer_shapes) {
    shapes->AddNonConnectableShapes(*layer_shapes);
  }

  if (force_below_depth && *force_below_depth == 0) {
    for (const auto &instance : instances_) {
      instance->CopyShapesOnLayer(layer, shapes);
    }
    return;
  }

  std::optional<int64_t> new_depth;
  if (force_below_depth) {
    new_depth = *force_below_depth - 1;
  }

  for (const auto &instance : instances_) {
    instance->CopyNonConnectableShapesOnLayer(layer, shapes, new_depth);
  }
}

ShapeCollection *Layout::GetShapeCollection(
    const geometry::Layer &layer) const {
  auto shapes_it = shapes_.find(layer);
  if (shapes_it != shapes_.end()) {
    return shapes_it->second.get();
  }
  return nullptr;
}

// Generally what we want from this function is to return all of the shapes we
// would *want* to connect to that aren't on the given nets, because those are
// the shapes we want to avoid blocking access to. In those cases we only care
// about connectable shapes in this Layout, and those that act as the interface
// to any instances herein. When we ask the instance for its ports, we don't
// want its instances' ports too.
//
// To track this through the recursive call loop, we decrement the max_depth
// counter every call. We define max_depth == 0 as being the shapes in this
// Layout and none of its instances.
//
// Layout::CopyConnectableShapesNotOnNets(max_depth=1)
//                    |
//                    V
// Instance::CopyConnectableShapesNotOnNets(max_depth=0)
//                    |
//                    V
// Instance::CopyAllShapes(max_depth=0)
//                    |
//                    V
// Layout::CopyAllShapes(max_depth=0)
//                    |
//                    X
// FIXME(aryap): Why does Instance::CopyConnectableShapesNotOnNets not just call
// Layout::CopyConnectableShapesNotOnNets?!
void Layout::CopyConnectableShapesNotOnNets(
    const EquivalentNets &nets,
    ShapeCollection *shapes,
    const std::optional<int64_t> &max_depth) const {
  for (const auto &entry : shapes_) {
    shapes->AddConnectableShapesNotOnNets(*entry.second, nets);
  }
  if (max_depth && *max_depth < 1) {
    return;
  }
  for (const auto &instance : instances_) {
    instance->CopyConnectableShapesNotOnNets(
        nets,
        shapes,
        // C++23 has std::optional::transform for this use case!
        max_depth ? *max_depth - 1 : max_depth);
  }
}

void Layout::CopyConnectableShapes(
    ShapeCollection *shapes,
    const std::optional<int64_t> &max_depth) const {
  for (const auto &entry : shapes_) {
    shapes->AddConnectableShapes(*entry.second);
  }
  for (const auto &instance : instances_) {
    instance->CopyConnectableShapes(
        shapes,
        max_depth ? *max_depth - 1 : max_depth,
        global_nets_);
  }
}

void Layout::CopyAllShapes(
    ShapeCollection *shapes,
    const std::optional<int64_t> &max_depth,
    const std::optional<std::set<std::string>> &no_prefix) const {
  for (const auto &entry : shapes_) {
    shapes->Add(*entry.second);
  }
  if (max_depth && *max_depth < 1) {
    return;
  }
  for (const auto &instance : instances_) {
    instance->CopyAllShapes(
        shapes,
        max_depth ? *max_depth - 1 : max_depth,
        no_prefix);
  }
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

void Layout::GetAllPorts(geometry::PortSet *ports) const {
  for (const auto &instance : instances_) {
    instance->GetInstancePorts(ports);
  }
}

void Layout::GetAllPortsExceptNamed(
    geometry::PortSet *ports, const std::string &named) const {
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

bool Layout::HasPort(const std::string &name) const {
  return ports_by_net_.find(name) != ports_by_net_.end();
}

bool Layout::ResolveEdgeSpec(
    const EdgeSpec &spec,
    geometry::Instance **from,
    std::vector<geometry::Port*> *from_ports,
    geometry::Instance **to,
    std::vector<geometry::Port*> *to_ports) const {
  auto extract_fn = [&](
      const EdgeSpec::Endpoint &endpoint,
      geometry::Instance *test_instance,
      geometry::Instance **matched_instance,
      std::vector<geometry::Port*> *matched_ports) -> bool {
    if (test_instance->name() != endpoint.instance_name) {
      return false;
    }
    *from = test_instance;
    size_t port_count = 0;
    for (const std::string &port_name : endpoint.port_names) {
      std::vector<geometry::Port*> ports;
      test_instance->GetInstancePorts(port_name, &ports);
      // C++23 would have append_range(...), which would be juicy.
      matched_ports->insert(matched_ports->end(), ports.begin(), ports.end());
      port_count += ports.size();
    }
    return port_count > 0;
  };

  for (auto &uniq : instances_) {
    bool matched_from = extract_fn(spec.from(), uniq.get(), from, from_ports);
    bool matched_to = extract_fn(spec.to(), uniq.get(), to, to_ports);
    if (matched_from && matched_to) {
      return true;
    }
  }
  return false;
}

const geometry::PortSet Layout::Ports() const {
  geometry::PortSet all_ports = geometry::Port::MakePortSet();
  for (const auto &entry : shapes_) {
    // How is this:
    std::transform(entry.second->ports().begin(),
                   entry.second->ports().end(),
                   std::inserter(all_ports, all_ports.begin()),
                   [](const std::unique_ptr<geometry::Port> &u) {
                     return u.get();
                   });
    // Nicer than this:
    //for (const auto &port : entry.second->ports) {
    //  all_ports.insert(port.get());
    //}
  }
  return all_ports;
}

void Layout::EraseLayerByName(const std::string &name) {
  EraseLayer(physical_db_.GetLayerInfo(name).internal_layer);
}

void Layout::EraseLayer(const geometry::Layer &layer) {
  shapes_.erase(layer);
}

void Layout::SavePoint(const std::string &name, const geometry::Point &point) {
  auto it = named_points_.find(name);
  LOG_IF(FATAL, name.empty())
      << "Saved point name cannot be empty (point: " << point << ")";
  LOG_IF(WARNING, it != named_points_.end())
      << "Saving " << name << " overrides an existing point " << it->second;
  named_points_[name] = point;
}

void Layout::SavePoints(std::map<const std::string, const Point> named_points) {
  for (const auto &entry : named_points) {
    SavePoint(entry.first, entry.second);
  }
};

geometry::Point Layout::GetPointOrDie(const std::string &name) const {
  auto point = GetPoint(name);
  LOG_IF(FATAL, !point) << "Point " << name << " not found";
  return *point;
}

std::optional<geometry::Point> Layout::GetPoint(const std::string &name) const {
  auto it = named_points_.find(name);
  if (it == named_points_.end()) {
    return std::nullopt;
  }
  return it->second;
}

}  // namespace bfg
