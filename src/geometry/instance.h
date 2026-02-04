#ifndef GEOMETRY_INSTANCE_H_
#define GEOMETRY_INSTANCE_H_

#include <unordered_map>
#include <string>

#include <absl/strings/str_cat.h>

#include <glog/logging.h>

#include "../equivalent_nets.h"
#include "point.h"
#include "port.h"
#include "rectangle.h"
#include "shape_collection.h"

namespace bfg {

class Layout;

namespace circuit {
class Instance;
}  // namespace circuit

namespace geometry {

class Instance : public Manipulable {
 public:
  typedef bool (*UniquePortCompare)(
      const std::unique_ptr<Port>&, const std::unique_ptr<Port>&);
  typedef std::set<std::unique_ptr<Port>, UniquePortCompare> InternalPortSet;

  // TODO(aryap): We should only need a const &Layout here!
  Instance(bfg::Layout *template_layout,
           const Point &lower_left)
      : ports_generated_(false),
        circuit_instance_(nullptr),
        template_layout_(template_layout),
        lower_left_(lower_left),
        reflect_vertical_(false),
        rotation_degrees_ccw_(0) {}

  Instance(bfg::Layout *template_layout)
      : Instance(template_layout, { 0, 0 }) {}

  Instance(const Instance &other)
      : ports_generated_(false),
        circuit_instance_(nullptr),
        name_(other.name_),
        template_layout_(other.template_layout_),
        lower_left_(other.lower_left_),
        reflect_vertical_(other.reflect_vertical_),
        rotation_degrees_ccw_(other.rotation_degrees_ccw_) {}

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;
  void AlignPoints(const Point &our_point, const Point &align_to);

  void ApplyInstanceTransforms(Layout *layout) const;

  // Apply the transforms from the template layout to the instance layout.
  void ApplyInstanceTransforms(ShapeCollection *shape_collection) const {
    shape_collection->Rotate(rotation_degrees_ccw_);
    shape_collection->Translate(lower_left_);
  }
  void ApplyInstanceTransforms(Point *point) const {
    point->Rotate(rotation_degrees_ccw_);
    point->Translate(lower_left_);
  }

  Point ApplyInstanceTransforms(const Point &point) const {
    Point mutated = point;
    ApplyInstanceTransforms(&mutated);
    return mutated;
  }

  // Apply rotation in a way that keeps the lower_left_ point the same as it
  // currently is.
  void RotatePreservingLowerLeft(int32_t rotation_degrees_ccw);

  uint64_t Height() const;
  uint64_t Width() const;

  uint64_t TilingHeight() const;
  uint64_t TilingWidth() const;
  Point TilingLowerLeft() const;

  // Align the lower-left point of the tiling bounds to the given point.
  void MoveTilingLowerLeft(const Point &new_lower_left);

  // Effective tiling bounds after all transformations.
  const Rectangle GetTilingBounds() const;

  const Rectangle GetBoundingBox() const;

  bool HasPort(const std::string &name) const;

  void GeneratePorts();

  std::string InstancePortName(const std::string &master_port_name) const {
    return absl::StrCat(name_, ".", master_port_name);
  }

  // FIXME(aryap): The pointers to ports handed out here will be invalidated
  // the next time ports are regenerated or whenever the Instance object is
  // deleted; we should pass out copies of the Port or shared_ptrs.
  //
  // Find the port named 'name', without the instance name prefix.
  //
  // TODO(aryap): Replace or supplement this with
  // PortSet GetPorts(const std::string &name);
  void GetInstancePorts(const std::string &name, std::vector<Port*> *out) {
    PortSet ports = Port::MakePortSet();
    GetInstancePorts(name, &ports);
    out->insert(out->end(), ports.begin(), ports.end());
  }

  void GetInstancePorts(PortSet *out) {
    if (!ports_generated_) GeneratePorts();
    for (auto &entry : instance_ports_) {
      for (auto &uniq : entry.second) {
        out->insert(uniq.get());
      }
    }
  }

  Port *GetNearestPortNamed(const Port &to_port, const std::string &name);
  Port *GetNearestPortNamed(const Point &to_point, const std::string &name);
  Port *GetFirstPortNamed(const std::string &name);

  void CopyShapesOnLayer(
      const geometry::Layer &layer,
      ShapeCollection *shapes,
      const std::optional<std::set<std::string>> &no_prefix =
          std::nullopt) const;

  void CopyNonConnectableShapesOnLayer(
      const geometry::Layer &layer,
      ShapeCollection *shapes,
      const std::optional<int64_t> &force_below_depth = 1) const;

  void CopyConnectableShapesNotOnNets(
      const EquivalentNets &nets,
      ShapeCollection *shapes,
      const std::optional<int64_t> &max_depth = std::nullopt,
      const std::optional<std::set<std::string>> &no_prefix =
          std::nullopt) const;

  void CopyConnectableShapesOnNets(
      const EquivalentNets &nets,
      ShapeCollection *shapes,
      const std::optional<int64_t> &max_depth = std::nullopt,
      const std::optional<std::set<std::string>> &no_prefix =
          std::nullopt) const;

  void CopyConnectableShapes(
      ShapeCollection *shapes,
      const std::optional<int64_t> &max_depth = std::nullopt,
      const std::optional<std::set<std::string>> &no_prefix =
          std::nullopt) const;

  void CopyAllShapes(
      ShapeCollection *shapes,
      const std::optional<int64_t> &max_depth = std::nullopt,
      const std::optional<std::set<std::string>> &no_prefix =
          std::nullopt) const;

  // Get a named point (from the template_layout_) and translate it to where it
  // is in this instance.
  Point GetPointOrDie(const std::string &name) const;
  std::optional<Point> GetPoint(const std::string &name) const;

  void set_circuit_instance(circuit::Instance *circuit_instance) {
    circuit_instance_ = circuit_instance;
  }
  circuit::Instance *circuit_instance() {
    return circuit_instance_;
  }

  void set_name(const std::string &name) {
    name_ = name;
    ports_generated_ = false;
  }
  const std::string &name() const { return name_; }

  bfg::Layout *template_layout() const { return template_layout_; }

  void set_lower_left(const Point &lower_left) {
    lower_left_ = lower_left;
    ports_generated_ = false;
  }
  const Point &lower_left() const { return lower_left_; }

  void set_reflect_vertical(bool reflect) {
    reflect_vertical_ = reflect;
    ports_generated_ = false;
  }
  const bool reflect_vertical() const { return reflect_vertical_; }

  void set_rotation_degrees_ccw(int32_t degrees_ccw) {
    rotation_degrees_ccw_ = degrees_ccw;
    ports_generated_ = false;
  }
  const int32_t rotation_degrees_ccw() const { return
    rotation_degrees_ccw_;
  }

  const std::unordered_map<std::string, InternalPortSet> &instance_ports()
      const {
    return instance_ports_;
  }

 private:
  void AddNamedInstancePort(
      const std::string &name, Port *instance_port);

  void GetInstancePorts(const std::string &name, PortSet *out) {
    if (!ports_generated_) GeneratePorts();
    const std::string actual_name = InstancePortName(name);
    auto it = instance_ports_.find(actual_name);
    if (it == instance_ports_.end()) {
      return;
    }
    for (const auto &uniq : it->second) {
      out->insert(uniq.get());
    }
  }

  bool ports_generated_;

  std::string name_;

  circuit::Instance *circuit_instance_;

  // This is the template cell.
  bfg::Layout *const template_layout_;

  // FIXME(aryap): This is confusing because "lower_left_" actually stores the
  // origin of the cell, not the "lower left" point. This should be renamed to
  // origin for instances.
  //
  // If you want the effecitve lower left point of the instance after all
  // transformations, use GetBoundingBox().lower_left().
  Point lower_left_;
  // This is mirroring in the X axis.
  bool reflect_vertical_;
  // FIXME(growly): Store rotation anti-clockwise.
  int32_t rotation_degrees_ccw_;

  std::unordered_map<std::string, InternalPortSet> instance_ports_;
};

}  // namespace geometry

std::ostream &operator<<(
    std::ostream &os,
    const geometry::Instance &instance);
}  // namespace bfg

#endif  // GEOMETRY_INSTANCE_H_
