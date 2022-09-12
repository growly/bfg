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

#include "vlsir/layout/raw.pb.h"

namespace bfg {

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
    rectangles_.emplace_back(copy);
    copy->set_layer(active_layer_);
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
    polygons_.emplace_back(copy);
    return copy;
  }
  void AddInstance(const geometry::Instance &instance) {
    geometry::Instance *copy = new geometry::Instance(instance);
    instances_.emplace_back(copy);
  }
  void AddPort(const geometry::Port &port) {
    geometry::Port *copy = new geometry::Port(port);
    ports_.emplace_back(copy);
    copy->set_layer(active_layer_);
  }
  void AddLayout(const Layout &other);

  std::string Describe() const;

  ::vlsir::raw::Layout ToVLSIRLayout() const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void ResetOrigin() override;
  void Translate(const geometry::Point &offset) override;

  const geometry::Rectangle GetBoundingBox() const;

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

  const std::vector<std::unique_ptr<geometry::Rectangle>> &rectangles() const {
    return rectangles_;
  }
  const std::vector<std::unique_ptr<geometry::Polygon>> &polygons() const { return polygons_; }
  const std::vector<std::unique_ptr<geometry::Instance>> &instances() const {
    return instances_;
  }
  const std::vector<std::unique_ptr<geometry::Port>> &ports() const { return ports_; }

 private:
  bfg::Cell *parent_cell_;

  std::string name_;

  ::vlsir::raw::LayerShapes *GetOrInsertLayerShapes(
      const geometry::Layer &layer,
      std::map<geometry::Layer, ::vlsir::raw::LayerShapes*> *shapes) const;

  const PhysicalPropertiesDatabase &physical_db_;

  geometry::Layer active_layer_;
  std::vector<std::unique_ptr<geometry::Rectangle>> rectangles_;
  std::vector<std::unique_ptr<geometry::Polygon>> polygons_;
  std::vector<std::unique_ptr<geometry::Port>> ports_;

  std::vector<std::unique_ptr<geometry::Instance>> instances_;
};

}  // namespace bfg

#endif  // LAYOUT_H_
