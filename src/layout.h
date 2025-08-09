#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <string>
#include <vector>
#include <stack>
#include <optional>

#include "physical_properties_database.h"
#include "scoped_layer.h"
#include "geometry/group.h"
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

// TODO(growly): Do we want this facility? Ok we do, but it's more convenient
// to just use one argument per parameter instead of this struct. TODO(aryap):
// Delete.
struct LayoutPadding {
  int64_t left;
  int64_t top;
  int64_t bottom;
  int64_t right;
};

class Layout : public geometry::Manipulable {
 public:
  struct ViaToSomeLayer {
    geometry::Point centre;
    std::string layer_name;
    // Indicate that the surrounding metal should be sized for a via here but
    // the via should not be inserted.
    bool pad_only;
  };

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

  // Add a port using the given port as a template. The port's layer is
  // overridden with the currently active layer.
  void AddPort(const geometry::Port &port,
               const std::string &net_prefix = "");
  void GetPorts(const std::string &net_name, geometry::PortSet *out)
      const;

  // Deletes all ports on the given net (name). Does NOT delete any other shapes
  // associated with the port, such as the rectangle on the pin layer.
  void DeletePorts(const std::string &name);

  // Take ownership of all the shapes in the other Layout, deleting them from
  // that Layout as we go. If a name_prefix is given, moved shapes will be
  // renamed with the given prefix at the start.
  void ConsumeLayout(Layout *other, const std::string &name_prefix = "");

  // Copies the objects in other layout into this one. If a name prefix is
  // given, it is apply to named points and net names copied from the other
  // Layout.
  void AddLayout(const Layout &other,
                 const std::string &name_prefix = "",
                 bool include_ports = true);

  // Translate the Layout such that the given reference point, assumed to be in
  // the coordinate space of this Layout, ends up at the target point.
  void AlignPointTo(
      const geometry::Point &reference, const geometry::Point &target);

  // TODO(aryap): Doubling all these functions for both the string and textual
  // representation of a layer is annoying. Any of these might be better:
  // 1) Make all API calls exclusively use the human-readable string version.
  // 2) Make a fancy LayerRef that can contain either a number or a string and
  // use that to reference layers everywhere.
  // 3) That could just become the layer class, actually. Except that having a
  // simple integer representation internally is a worthy thing to keep.
  geometry::Rectangle *MakeVia(
      const std::string &layer_name,
      const geometry::Point &centre,
      const std::optional<std::string> &net = std::nullopt) {
    return MakeVia(physical_db_.GetLayer(layer_name), centre, net);
  }
  geometry::Rectangle *MakeVia(
      const geometry::Layer &layer,
      const geometry::Point &centre,
      const std::optional<std::string> &net = std::nullopt);

  // TODO(aryap): It would be handy to generalise the "connection" from just a
  // point to a struct that allows the client to specify a via to be inserted
  // (or just a bulge to be created) at the given point.
  geometry::Group MakeVerticalSpineWithFingers(
      const std::string &spine_layer_name,
      const std::string &finger_layer_name,
      const std::string &net,
      const std::vector<geometry::Point> &connections,
      int64_t spine_x,
      int64_t spine_width,
      Layout *layout);

  void DistributeVias(
      const std::string &via_layer,
      const geometry::Point &start,
      const geometry::Point &end,
      const std::optional<std::string> &net = std::nullopt) {
    return DistributeVias(physical_db_.GetLayer(via_layer), start, end, net);
  }
  void DistributeVias(
      const geometry::Layer &via_layer,
      const geometry::Point &start,
      const geometry::Point &end,
      const std::optional<std::string> &net = std::nullopt);

  // Stamps out vias every given pitch, starting half that pitch in from the
  // starting point, until the end.
  void StampVias(
      const std::string &via_layer,
      const geometry::Point &start,
      const geometry::Point &end,
      int64_t pitch,
      const std::optional<std::string> &net = std::nullopt) {
    return StampVias(physical_db_.GetLayer(via_layer), start, end, pitch, net);
  }
  void StampVias(
      const geometry::Layer &layer,
      const geometry::Point &start,
      const geometry::Point &end,
      int64_t pitch,
      const std::optional<std::string> &net = std::nullopt);

  // TODO(aryap): I am confused. I made "Port" an abstract thing, though it is a
  // shape sized according to the rules of the via layer you give it. But it
  // does not appear as a shape. A "Pin" is a real shape that is emited in the
  // layout description and corresponds to an abstract "Port". "Port"s are used
  // internally to determine connection points, and "Pin"s are layout features
  // that communicate this to legacy tools. So until that's cleaned up, have a
  // second way to do it, which adds a pin shape on a pin layer and also makes
  // it a port:
  void MakePin(const std::string &net_name,
               const geometry::Point &centre,
               const std::string &pin_layer);

  void MakePort(const std::string &net_name,
                const geometry::Point &centre,
                const std::string &layer_name = "");

  // Draw that alternates between two layers with every subsequent edge.
  void MakeAlternatingWire(
      const std::vector<geometry::Point> &points,
      const std::string &first_layer_name,
      const std::string &second_layer_name,
      const std::optional<std::string> &net = std::nullopt);

  geometry::Polygon *MakeWire(
      const std::vector<geometry::Point> &points,
      const std::string &wire_layer_name,
      const std::optional<std::string> &start_layer_name = std::nullopt,
      const std::optional<std::string> &end_layer_name = std::nullopt,
      bool start_pad_only = false,
      bool end_pad_only = false,
      const std::optional<std::string> &net = std::nullopt);

  geometry::Polygon *MakeWire(
      const std::vector<geometry::Point> &points,
      const std::string &wire_layer_name,
      const std::vector<ViaToSomeLayer> vias,
      const std::optional<std::string> &net = std::nullopt);

  // Add the layout of every instance to the this layout directly, removing one
  // layer of hierarchy. To flatten completely, this must be called repeatedly
  // until no more instances_ remain.
  void Flatten();

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

  void AddGlobalNet(const std::string &net) {
    global_nets_.insert(net);
  }
  bool HasGlobalNet(const std::string &net) const {
    return global_nets_.find(net) != global_nets_.end();
  }

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

  void Rotate(int32_t degrees_ccw) override;
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

  const geometry::Rectangle GetBoundingBoxByNameOrDie(
      const std::string &layer_name) const;
  const geometry::Rectangle GetBoundingBoxOrDie(const geometry::Layer &layer)
      const;

  void SetTilingBounds(const geometry::Rectangle &rectangle) {
    tiling_bounds_ = rectangle;
  }
  void UnsetTilingBounds() {
    tiling_bounds_.reset();
  }

  // TODO(aryap): Why did I differentiate the names of functions doing lookups
  // by name with "ByName"? Function overloading solves this problem. Silly
  // goose.
  void EraseLayerByName(const std::string &name);
  void EraseLayer(const geometry::Layer &layer);

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

  void GetAllPorts(geometry::PortSet *ports) const;

  void GetAllPortsExceptNamed(
      geometry::PortSet *ports,
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

  const std::unordered_map<std::string, geometry::PortSet>
      &ports_by_net() const { return ports_by_net_; }
  const geometry::PortSet Ports() const;
  const std::vector<std::unique_ptr<geometry::Instance>> &instances() const {
    return instances_;
  }

  const std::unordered_map<std::string, geometry::Point> &named_points() const {
    return named_points_;
  }

  const std::set<std::string> &global_nets() const {
    return global_nets_;
  }

 private:
  bfg::Cell *parent_cell_;

  std::string name_;

  void AddPortByNet(const std::string &name, geometry::Port *port);

  ShapeCollection *GetOrInsertLayerShapes(const geometry::Layer &layer);

  const PhysicalPropertiesDatabase &physical_db_;

  std::optional<geometry::Rectangle> tiling_bounds_;

  std::vector<std::unique_ptr<geometry::Instance>> instances_;

  std::unordered_map<std::string, geometry::PortSet> ports_by_net_;

  geometry::Layer active_layer_;
  std::stack<geometry::Layer> previous_layers_;

  std::map<geometry::Layer, std::unique_ptr<ShapeCollection>> shapes_;

  std::unordered_map<std::string, geometry::Point> named_points_;

  // Shapes with these net labelles do not have prefixes applied to them when
  // this Layout is added to another Layout.
  std::set<std::string> global_nets_;
};

}  // namespace bfg

#endif  // LAYOUT_H_
