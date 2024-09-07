#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <string>
#include <vector>
#include <stack>
#include <optional>

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
  geometry::Rectangle *AddRectangleAsPort(const geometry::Rectangle &rectangle,
                                          const std::string &net,
                                          const std::string &net_prefix = "");
  geometry::Rectangle *AddSquare(
      const geometry::Point &centre, uint64_t side_width) {
    return AddRectangle(
        geometry::Rectangle::CentredAt(centre, side_width, side_width));
  }
  geometry::Rectangle *AddSquareAsPort(
      const geometry::Point &centre,
      uint64_t side_width,
      const std::string &net,
      const std::string &net_prefix = "");

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
  geometry::Polygon *AddPolyLine(const geometry::PolyLine &line);
  void AddPort(const geometry::Port &port,
               const std::string &net_prefix = "");
  void GetPorts(const std::string &net_name, std::set<geometry::Port*> *out)
      const;

  // Copies the objects in other layout into this one. If a name prefix is
  // given, it is apply to named points and net names copied from the other
  // Layout.
  void AddLayout(const Layout &other, const std::string &name_prefix = "");

  // Translate the Layout such that the given reference point, assumed to be in
  // the coordinate space of this Layout, ends up at the target point.
  void AlignPointTo(
      const geometry::Point &reference, const geometry::Point &target);

  geometry::Rectangle *MakeVia(
      const std::string &layer_name,
      const geometry::Point &centre,
      const std::optional<std::string> &net = std::nullopt);
  void MakePort(const std::string &net_name,
                const geometry::Point &centre,
                const std::string &layer_name = "");

  // TODO(aryap): Every shape electrically (passively) connected to 'point' is
  // given the net 'net'.
  //
  // This is a convenience function to save having to explicitly label all
  // shapes known to be connected when drawing the layout. However, it requires
  // knowledge of which layer shapes can connect electrically. For example,
  // polysilicon does not passively label an underlying diffusion region with
  // its net, since it is the gate to a transistor. Wire and via layers,
  // however, do colour each other with their net when they touch.
  void LabelNet(const geometry::Point &point, const std::string &net);

  std::string Describe() const;

  // Report the rectangular area covered by this Layout. (This is the area of
  // the bounding box.)
  // std::string Area() const;

  ::vlsir::raw::Abstract ToVLSIRAbstract(
      std::optional<geometry::Layer> top_layer = std::nullopt) const;
  ::vlsir::raw::Layout ToVLSIRLayout() const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;

  // Shifts the layout so that the lower-left-most point of the bounding box
  // lies at the origin.
  void ResetOrigin() override;

  void Translate(const geometry::Point &offset) override;

  // Shifts the layout so that the left-most point of the bounding box
  // lies at the origin.
  void ResetX();

  // Shifts the layout so that the lowest point of the bounding box
  // lies at the origin.
  void ResetY();

  // Shifts the layout so that the lower-left-most point of the tiling bounds
  // lies at the origin.
  void ResetToTilingBounds();

  void MoveTo(const geometry::Point &lower_left) {
    // TODO(aryap): Surely this just takes one translation after you compute a
    // vector:
    ResetOrigin();
    Translate(lower_left);
  }

  uint64_t Height() const {
    return GetBoundingBox().Height();
  }
  uint64_t Width() const {
    return GetBoundingBox().Width();
  }

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
  void SavePoints(
      std::map<const std::string, const geometry::Point> named_points);
  geometry::Point GetPointOrDie(const std::string &name) const;
  std::optional<geometry::Point> GetPoint(const std::string &name) const;

  // NOTE(aryap): It might be justified to create a "ShadowShapeCollection" or
  // something that merely collects pointers to shapes guaranteed to outlive
  // the ShadowShapeCollection.
  void CopyShapesOnLayer(
      const geometry::Layer &layer, ShapeCollection *shapes) const;
  ShapeCollection *GetShapeCollection(const geometry::Layer &layer) const;

  void CopyNonConnectableShapesOnLayer(
      const geometry::Layer &layer, ShapeCollection *shapes) const;

  void CopyConnectableShapesNotOnNets(
      const EquivalentNets &nets, ShapeCollection *shapes) const;

  void CopyConnectableShapes(ShapeCollection *shapes) const;

  void CopyAllShapes(ShapeCollection *shapes) const;

  void GetInstancesByName(
      std::unordered_map<std::string, geometry::Instance *const> *mapping)
      const;

  void GetAllPorts(
      std::set<geometry::Port*> *ports) const;

  void GetAllPortsExceptNamed(
      std::set<geometry::Port*> *ports,
      const std::string &named) const;

  bool HasPort(const std::string &name) const;

  const std::string &NameOrParentName() const;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  void set_parent_cell(bfg::Cell *cell) { parent_cell_ = cell; }
  bfg::Cell *parent_cell() const { return parent_cell_; }

  void SetActiveLayerByName(const std::string &name);
  void RestoreLastActiveLayer() {
    if (previous_layers_.empty())
      return;
    geometry::Layer last = previous_layers_.top();
    previous_layers_.pop();
    active_layer_ = last;
  }
  void set_active_layer(const geometry::Layer &active_layer) {
    previous_layers_.push(active_layer_);
    active_layer_ = active_layer;
  };
  const geometry::Layer &active_layer() const { return active_layer_; }

  const std::unordered_map<std::string, std::set<geometry::Port*>>
      &ports_by_net() const { return ports_by_net_; }
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
  std::stack<geometry::Layer> previous_layers_;

  std::map<geometry::Layer, std::unique_ptr<ShapeCollection>> shapes_;

  std::unordered_map<std::string, geometry::Point> named_points_;
};

}  // namespace bfg

#endif  // LAYOUT_H_
