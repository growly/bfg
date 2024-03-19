#include <algorithm>
#include <map>
#include <vector>

#include <glog/logging.h>

#include "geometry/layer.h"
#include "routing_via_info.h"

namespace bfg {

const std::vector<geometry::Layer> RoutingViaInfo::ConnectedLayers() const {
  std::vector<geometry::Layer> connected_layers;
  for (const auto &entry : connected_layers_) {
    connected_layers.push_back(entry.first);
  }
  return connected_layers;
}

const RoutingViaEncapInfo &RoutingViaInfo::Layer(const geometry::Layer &layer)
    const {
  auto it = connected_layers_.find(layer);
  LOG_IF(FATAL, it == connected_layers_.end())
      << "RoutingViaInfo doesn't connect this layer: " << layer;
  return it->second;
}

int64_t RoutingViaInfo::MinMinArea() const {
  int64_t minimum_value = 0;
  for (const auto &entry : connected_layers_) {
    minimum_value = std::min(minimum_value, entry.second.min_area);
  }
  return minimum_value;
}

int64_t RoutingViaInfo::MaxOverhangLength() const {
  int64_t maximum_value = 0;
  for (const auto &entry : connected_layers_) {
    maximum_value = std::max(maximum_value, entry.second.overhang_length);
  }
  return maximum_value;
}

int64_t RoutingViaInfo::MaxOverhangWidth() const {
  int64_t maximum_value = 0;
  for (const auto &entry : connected_layers_) {
    maximum_value = std::max(maximum_value, entry.second.overhang_width);
  }
  return maximum_value;
}

int64_t RoutingViaInfo::MaxOverhang() const {
  return std::max(MaxOverhangLength(), MaxOverhangWidth());
}

int64_t RoutingViaInfo::MaxViaSide() const {
  return std::max(width_, height_);
}

int64_t RoutingViaInfo::MaxEncapLength() const {
  return MaxViaSide() + 2 * MaxOverhangLength();
}

int64_t RoutingViaInfo::MaxEncapWidth() const {
  return MaxViaSide() + 2 * MaxOverhangWidth();
}

int64_t RoutingViaInfo::MaxEncapSide() const {
  return MaxViaSide() + 2 * MaxOverhang();
}

int64_t RoutingViaInfo::EncapLength(const geometry::Layer &layer) const {
  const RoutingViaEncapInfo &encap_info = Layer(layer);
  return MaxViaSide() + encap_info.overhang_length;
}

int64_t RoutingViaInfo::EncapWidth(const geometry::Layer &layer) const {
  const RoutingViaEncapInfo &encap_info = Layer(layer);
  return MaxViaSide() + encap_info.overhang_width;
}
  
}   // namespace bfg
