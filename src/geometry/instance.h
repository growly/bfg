#ifndef GEOMETRY_INSTANCE_H_
#define GEOMETRY_INSTANCE_H_

#include <unordered_map>
#include <string>

#include <glog/logging.h>

#include "point.h"
#include "port.h"
#include "rectangle.h"
#include "shape_collection.h"

namespace bfg {

class Layout;

namespace geometry {

class Instance : public Manipulable {
 public:
  Instance(bfg::Layout *template_layout,
           const Point &lower_left)
      : ports_generated_(false),
        template_layout_(template_layout),
        lower_left_(lower_left),
        reflect_vertical_(false),
        rotation_degrees_ccw_(0) {}

  Instance(const Instance &other)
      : ports_generated_(false),
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

  const Rectangle GetBoundingBox() const;

  void GeneratePorts();

  // FIXME(aryap): The pointers to ports handed out here will be invalidated
  // the next time ports are regenerated or whenever the Instance object is
  // deleted; we should pass out copies of the Port or shared_ptrs.
  void GetInstancePorts(const std::string &name, std::set<Port*> *out) {
    if (!ports_generated_) GeneratePorts();
    auto it = instance_ports_.find(name);
    LOG_IF(FATAL, it == instance_ports_.end())
        << "No such instance port: " << name << " on instance "
        << name_;
    for (const auto &uniq : it->second) {
      out->insert(uniq.get());
    }
  }

  void GetShapesOnLayer(
      const geometry::Layer &layer, ShapeCollection *shapes) const;

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  bfg::Layout *template_layout() const { return template_layout_; }

  const Point &lower_left() const { return lower_left_; }

  void set_reflect_vertical(bool reflect) {
    reflect_vertical_ = reflect;
  }
  const bool reflect_vertical() const { return reflect_vertical_; }

  void set_rotation_degrees_ccw(int32_t degrees_ccw) {
    rotation_degrees_ccw_ = degrees_ccw;
    ports_generated_ = false;
  }
  const int32_t rotation_degrees_ccw() const { return
    rotation_degrees_ccw_;
  }

  const std::unordered_map<
      std::string, std::unique_ptr<Port>> InstancePorts() const;

 private:
  bool ports_generated_;

  std::string name_;

  // This is the template cell.
  bfg::Layout *const template_layout_;

  Point lower_left_;
  bool reflect_vertical_;
  // FIXME(growly): Store rotation anti-clockwise.
  int32_t rotation_degrees_ccw_;

  std::unordered_map<std::string, std::set<std::unique_ptr<Port>>>
      instance_ports_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_INSTANCE_H_
