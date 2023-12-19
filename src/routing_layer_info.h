#ifndef ROUTING_LAYER_INFO_H_
#define ROUTING_LAYER_INFO_H_

#include <cstdint>

#include "physical_properties_database.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"

namespace bfg {

struct RoutingLayerInfo {
  geometry::Layer layer;
  geometry::Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingTrackDirection direction;
  int64_t pitch;
  int64_t min_separation;
};

}   // namespace bfg

#endif  // ROUTING_LAYER_INFO_H_
