#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <string>
#include <vector>

#include "physical_properties_database.h"
#include "geometry/instance.h"
#include "geometry/layer.h"
#include "geometry/manipulable.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"
#include "geometry/shape_collection.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

using geometry::ShapeCollection;

class Cell;

// TODO(growly): Do we want this facility?
struct LayoutPadding {
  int64_t left;
  int64_t top;
  int64_t bottom;
  int64_t right;
};

class Layout : public geometry::Manipulable {
 public:

  Layout() = delete;
  Layout(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db),
        active_layer_(0) {
  }

  geometry::Rectangle *AddRectangle(const geometry::Rectangle &rectangle) {
    geometry::Rectangle *copy = new geometry::Rectangle(rectangle);
    copy->set_layer(active_layer_);
    ShapeCollection *shape_collection = GetOrInsertLayerShapes(active_layer_);
    shape_collection->rectangles().emplace_back(copy);
    return copy;
  }
  geometry::Rectangle *AddSquare(
      const geometry::Point &centre, uint64_t side_width) {
    return AddRectangle(geometry::Rectangle(
        geometry::Point(centre.x() - side_width / 2,
                        centre.y() - side_width / 2),
        side_width,
        side_width));
  }
  geometry::Polygon *AddPolygon(const geometry::Polygon &polygon) {
    geometry::Polygon *copy = new geometry::Polygon(polygon);
    copy->set_layer(active_layer_);
    ShapeCollection *shape_collection = GetOrInsertLayerShapes(active_layer_);
    shape_collection->polygons().emplace_back(copy);
    return copy;
  }
  geometry::Instance *AddInstance(const geometry::Instance &instance) {
    geometry::Instance *copy = new geometry::Instance(instance);
    instances_.emplace_back(copy);
    return copy;
  }
  void AddPort(const geometry::Port &port, const std::string &net_prefix = "");
  void GetPorts(const std::string &net_name, std::set<geometry::Port*> *out) const;
  void AddLayout(const Layout &other, const std::string &name_prefix = "");

  void MakeVia(const std::string &layer_name, const geometry::Point &centre);

  std::string Describe() const;

  ::vlsir::raw::Layout ToVLSIRLayout() const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void ResetOrigin() override;
  void Translate(const geometry::Point &offset) override;

  const geometry::Rectangle GetBoundingBox() const;
  const geometry::Rectangle GetTilingBounds() const {
    if (tiling_bounds_) {
      return *tiling_bounds_;
    } else {
      return GetBoundingBox();
    }
  }

  void SetTilingBounds(const geometry::Rectangle &rectangle) {
    tiling_bounds_.reset(new geometry::Rectangle(rectangle));
  }
  void UnsetTilingBounds() {
    tiling_bounds_.reset();
  }

  void SavePoint(const std::string &name, const geometry::Point &point);
  geometry::Point GetPoint(const std::string &name) const;

  void GetShapesOnLayer(
      const geometry::Layer &layer, ShapeCollection *shapes) const;
  ShapeCollection *GetShapeCollection(const geometry::Layer &layer) const;

  const std::string &NameOrParentName() const;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  void set_parent_cell(bfg::Cell *cell) { parent_cell_ = cell; }
  bfg::Cell *parent_cell() const { return parent_cell_; }

  void SetActiveLayerByName(const std::string &name);
  void set_active_layer(const geometry::Layer &active_layer) {
    active_layer_ = active_layer;
  };
  const geometry::Layer &active_layer() const { return active_layer_; }

  const std::unordered_map<std::string, std::set<geometry::Port*>>
      &ports_by_net() const { return ports_by_net_; }
  //const std::vector<std::unique_ptr<geometry::Rectangle>> &rectangles() const {
  //  return rectangles_;
  //}
  //const std::vector<std::unique_ptr<geometry::Polygon>> &polygons() const { return polygons_; }
  //const std::vector<std::unique_ptr<geometry::Port>> &ports() const { return ports_; }
  const std::set<geometry::Port*> Ports() const;
  const std::vector<std::unique_ptr<geometry::Instance>> &instances() const {
    return instances_;
  }

 private:
  bfg::Cell *parent_cell_;

  std::string name_;

  ShapeCollection *GetOrInsertLayerShapes(const geometry::Layer &layer);

  const PhysicalPropertiesDatabase &physical_db_;

  std::unique_ptr<geometry::Rectangle> tiling_bounds_;

  std::vector<std::unique_ptr<geometry::Instance>> instances_;

  std::unordered_map<std::string, std::set<geometry::Port*>> ports_by_net_;

  geometry::Layer active_layer_;
  std::map<geometry::Layer, std::unique_ptr<ShapeCollection>> shapes_;

  std::unordered_map<std::string, geometry::Point> named_points_;
};

}  // namespace bfg

#endif  // LAYOUT_H_
