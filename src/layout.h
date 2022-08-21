#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <string>
#include <vector>

#include "physical_properties_database.h"
#include "geometry/instance.h"
#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/polygon.h"
#include "geometry/port.h"
#include "geometry/rectangle.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

class Cell;

class Layout {
 public:
  Layout() = delete;
  Layout(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db),
        active_layer_(0) {
  }

  void AddRectangle(const geometry::Rectangle &rectangle) {
    rectangles_.push_back(rectangle);
    rectangles_.back().set_layer(active_layer_);
  }
  void AddPolygon(const geometry::Polygon &polygon) {
    polygons_.push_back(polygon);
    polygons_.back().set_layer(active_layer_);
  }
  void AddInstance(const geometry::Instance &instance) {
    instances_.push_back(instance);
  }
  void AddPort(const geometry::Port &port) {
    ports_.push_back(port);
    ports_.back().set_layer(active_layer_);
  }

  std::string Describe() const;

  ::vlsir::raw::Layout ToVLSIRLayout() const;

  const geometry::Rectangle GetBoundingBox() const;

  void set_parent_cell(bfg::Cell *cell) { parent_cell_ = cell; }
  bfg::Cell *parent_cell() const { return parent_cell_; }

  void SetActiveLayerByName(const std::string &name);
  void set_active_layer(const geometry::Layer &active_layer) {
    active_layer_ = active_layer;
  };
  const geometry::Layer &active_layer() const { return active_layer_; }

  const std::vector<geometry::Rectangle> &rectangles() const {
    return rectangles_;
  }
  const std::vector<geometry::Polygon> &polygons() const { return polygons_; }
  const std::vector<geometry::Instance> &instances() const {
    return instances_;
  }
  const std::vector<geometry::Port> &ports() const { return ports_; }

 private:
  bfg::Cell *parent_cell_;

  ::vlsir::raw::LayerShapes *GetOrInsertLayerShapes(
      const geometry::Layer &layer,
      std::map<geometry::Layer, ::vlsir::raw::LayerShapes*> *shapes) const;

  const PhysicalPropertiesDatabase &physical_db_;

  geometry::Layer active_layer_;
  std::vector<geometry::Rectangle> rectangles_;
  std::vector<geometry::Polygon> polygons_;
  std::vector<geometry::Port> ports_;

  std::vector<geometry::Instance> instances_;
};

}  // namespace bfg

#endif  // LAYOUT_H_
