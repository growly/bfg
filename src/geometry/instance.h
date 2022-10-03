#ifndef GEOMETRY_INSTANCE_H_
#define GEOMETRY_INSTANCE_H_

#include <unordered_map>
#include <string>

#include <glog/logging.h>

#include "point.h"
#include "port.h"
#include "rectangle.h"

namespace bfg {

class Layout;

namespace geometry {

class Instance : public Manipulable {
 public:
  Instance(bfg::Layout *template_layout,
           const Point &lower_left)
      : template_layout_(template_layout),
        lower_left_(lower_left),
        reflect_vertical_(false),
        rotation_clockwise_degrees_(0) {}

  Instance(const Instance &other)
      : name_(other.name_),
        template_layout_(other.template_layout_),
        lower_left_(other.lower_left_),
        reflect_vertical_(other.reflect_vertical_),
        rotation_clockwise_degrees_(other.rotation_clockwise_degrees_) {}

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;

  const Rectangle GetBoundingBox() const;

  void GeneratePorts();

  Port *GetInstancePort(const std::string &name) {
    auto it = instance_ports_.find(name);
    LOG_IF(FATAL, it == instance_ports_.end())
        << "No such instance port: " << name << " on instance "
        << name_;
    return it->second.get();
  }

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  bfg::Layout *template_layout() const { return template_layout_; }

  const Point &lower_left() const { return lower_left_; }

  void set_reflect_vertical(bool reflect) {
    reflect_vertical_ = reflect;
  }
  const bool reflect_vertical() const { return reflect_vertical_; }

  void set_rotation_clockwise_degrees(int32_t degrees) {
    rotation_clockwise_degrees_ = degrees;
  }
  const int32_t rotation_clockwise_degrees() const { return
    rotation_clockwise_degrees_;
  }

  const std::unordered_map<
      std::string, std::unique_ptr<Port>> InstancePorts() const;

 private:
  std::string name_;

  // This is the template cell.
  bfg::Layout *const template_layout_;

  Point lower_left_;
  bool reflect_vertical_;
  int32_t rotation_clockwise_degrees_;

  std::unordered_map<std::string, std::unique_ptr<Port>> instance_ports_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_INSTANCE_H_
