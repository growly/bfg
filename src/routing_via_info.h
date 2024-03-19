#ifndef ROUTING_VIA_INFO_H_
#define ROUTING_VIA_INFO_H_

#include <map>
#include <vector>

#include <glog/logging.h>

#include "geometry/layer.h"

namespace bfg {

struct RoutingViaEncapInfo {
  // These are the via encapsulations in the axis of the bulge and the
  // transverse axis respectively.
  int64_t overhang_length;
  int64_t overhang_width;

  int64_t min_area;
};

class RoutingViaInfo {
 public:
  const std::vector<geometry::Layer> ConnectedLayers() const;

  void AddRoutingViaEncapInfo(
      const geometry::Layer &layer, const RoutingViaEncapInfo &info) {
    connected_layers_[layer] = info;
    LOG_IF(FATAL, connected_layers_.size() > 2)
        << "RoutingViaInfo should always connect 2 layers. This one "
        << " now connects " << connected_layers_.size();
  }

  const RoutingViaEncapInfo &Layer(const geometry::Layer &layer) const;

  int64_t MinMinArea() const;
  int64_t MaxOverhangLength() const;
  int64_t MaxOverhangWidth() const;
  int64_t MaxOverhang() const;
  int64_t MaxViaSide() const;
  int64_t MaxEncapLength() const;
  int64_t MaxEncapWidth() const;
  int64_t MaxEncapSide() const;

  int64_t EncapLength(const geometry::Layer &layer) const;
  int64_t EncapWidth(const geometry::Layer &layer) const;

  void set_layer(const geometry::Layer &layer) { layer_ = layer; }
  const geometry::Layer &layer() const { return layer_; }

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  void set_width(int64_t width) { width_ = width; }
  int64_t width() const { return width_; }

  void set_height(int64_t height) { height_ = height; }
  int64_t height() const { return height_; }

 private:
  // Vias have their own layer.
  geometry::Layer layer_;
  // Need some measure of cost for connecting between these two layers. Maybe
  // a function that describes the cost based on something (like length,
  // sheet resistance).
  double cost_;
  int64_t width_;
  int64_t height_;

  std::map<geometry::Layer, RoutingViaEncapInfo> connected_layers_;
};

}   // namespace bfg

#endif  // ROUTING_VIA_INFO_H_
