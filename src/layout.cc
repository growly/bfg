#include "layout.h"

#include <sstream>
#include <absl/strings/str_cat.h>

#include "cell.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

using geometry::Point;

const std::string &Layout::NameOrParentName() const {
  if (name_ == "" && parent_cell_ != nullptr) {
    return parent_cell_->name();
  }
  return name_;
}

void Layout::MirrorY() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorY(); }
  for (const auto &polygon : polygons_) { polygon->MirrorY(); }
  for (const auto &port : ports_) { port->MirrorY(); }
  for (const auto &instance : instances_) { instance->MirrorY(); }
  for (auto &entry : named_points_) { entry.second.MirrorY(); }
}

void Layout::MirrorX() {
  for (const auto &rectangle : rectangles_) { rectangle->MirrorX(); }
  for (const auto &polygon : polygons_) { polygon->MirrorX(); }
  for (const auto &port : ports_) { port->MirrorX(); }
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
  for (const auto &rectangle : rectangles_) { rectangle->Translate(offset); }
  for (const auto &polygon : polygons_) { polygon->Translate(offset); }
  for (const auto &port : ports_) { port->Translate(offset); }
  for (const auto &instance : instances_) { instance->Translate(offset); }
  for (auto &entry : named_points_) { entry.second.Translate(offset); }
}

void Layout::ResetOrigin() {
  geometry::Rectangle bounding_box = GetBoundingBox();
  Translate(-bounding_box.lower_left());
}

const geometry::Rectangle Layout::GetBoundingBox() const {
  Point start;
  if (!rectangles_.empty()) {
    start = rectangles_.front()->lower_left();
  } else if (!polygons_.empty()) {
    start = polygons_.front()->GetBoundingBox().lower_left();
  } else if (!instances_.empty()) {
    start = instances_.front()->GetBoundingBox().lower_left();
  } else {
    // Layout is empty.
    return geometry::Rectangle(Point(0, 0), Point(0, 0));
  }

  int64_t min_x = start.x();
  int64_t max_x = start.x();
  int64_t min_y = start.y();
  int64_t max_y = start.y();

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

  for (const auto &instance : instances_) {
    LOG(INFO) << "computing bounding box for " << instance->name();
    geometry::Rectangle bounding_box = instance->GetBoundingBox();
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

  // TODO(growly): Include saved points in bounds? No...?

  return geometry::Rectangle(Point(min_x, min_y), Point(max_x, max_y));
}

std::string Layout::Describe() const {
  std::stringstream ss;

  ss << "layout: " << rectangles_.size() << " rectangles, "
     << polygons_.size() << " polygons, "
     << std::endl;
  for (const auto &rectangle : rectangles_) {
    ss << "rect " << rectangle->lower_left().x() << " "
       << rectangle->lower_left().y() << " "
       << rectangle->upper_right().x() << " "
       << rectangle->upper_right().y() << std::endl;
  }

  for (const auto &poly : polygons_) {
    ss << "polygon ";
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

  return ss.str();
}

void Layout::SetActiveLayerByName(const std::string &name) {
  set_active_layer(physical_db_.GetLayerInfo(name).internal_layer);
}

::vlsir::raw::Layout Layout::ToVLSIRLayout() const {
  ::vlsir::raw::Layout layout_pb;

  std::map<geometry::Layer, ::vlsir::raw::LayerShapes*> shapes;

  layout_pb.set_name(parent_cell_->name());

  // Collect shapes by layer.
  for (const auto &rect : rectangles_) {
    ::vlsir::raw::LayerShapes *layer_shapes_pb =
        GetOrInsertLayerShapes(rect->layer(), &shapes);
    ::vlsir::raw::Rectangle *rect_pb = layer_shapes_pb->add_rectangles();
    rect_pb->mutable_lower_left()->set_x(rect->lower_left().x());
    rect_pb->mutable_lower_left()->set_y(rect->lower_left().y());
    rect_pb->set_width(rect->upper_right().x() - rect->lower_left().x());
    rect_pb->set_height(rect->upper_right().y() - rect->lower_left().y());
  }
  for (const auto &poly : polygons_) {
    ::vlsir::raw::LayerShapes *layer_shapes_pb =
        GetOrInsertLayerShapes(poly->layer(), &shapes);
    ::vlsir::raw::Polygon *poly_pb = layer_shapes_pb->add_polygons();
    for (const auto &point : poly->vertices()) {
      ::vlsir::raw::Point *point_pb = poly_pb->add_vertices();
      point_pb->set_x(point.x());
      point_pb->set_y(point.y());
    }
  }
  LOG_IF(WARNING, !ports_.empty()) << "vlsir does not support ports yet";
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
    instance_pb->set_rotation_clockwise_degrees(
        instance->rotation_clockwise_degrees());
  }

  for (const auto pair : shapes) {
    *layout_pb.add_shapes() = *pair.second;
    delete pair.second;
  }

  return layout_pb;
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

void Layout::AddLayout(const Layout &other, const std::string &name_prefix) {
  // To be able to support this we'd need to make a temporary copy of all the
  // containers:
  LOG_IF(FATAL, this == &other) << "Can't add layout to itself.";
  for (const auto &rectangle : other.rectangles_) {
    rectangles_.emplace_back(new geometry::Rectangle(*rectangle));
  }
  for (const auto &polygon : other.polygons_) {
    polygons_.emplace_back(new geometry::Polygon(*polygon));
  }
  for (const auto &port : other.ports_) {
    ports_.emplace_back(new geometry::Port(*port));
  }
  for (const auto &instance : other.instances_) {
    instances_.emplace_back(new geometry::Instance(*instance));
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
