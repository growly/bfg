#ifndef ROUTING_LAYER_INFO_H_
#define ROUTING_LAYER_INFO_H_

#include <cstdint>

#include "physical_properties_database.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"

namespace bfg {

class RoutingLayerInfo {
 public:
  RoutingLayerInfo()
    : layer_(0),
      area_(geometry::Rectangle()),
      wire_width_(0),
      offset_(0),
      direction_(RoutingTrackDirection::kTrackHorizontal),
      pitch_(0),
      min_separation_(0) {}

  const geometry::Layer &layer() const { return layer_; }
  void set_layer(const geometry::Layer &layer) { layer_ = layer; }
  const std::optional<geometry::Layer> &pin_layer() const { return pin_layer_; }
  void set_pin_layer(const std::optional<geometry::Layer> &pin_layer) {
    pin_layer_ = pin_layer;
  }
  const geometry::Rectangle &area() const { return area_; }
  void set_area(const geometry::Rectangle &area) { area_ = area; }
  int64_t wire_width() const { return wire_width_; }
  void set_wire_width(int64_t wire_width) { wire_width_ = wire_width; }
  int64_t offset() const { return offset_; }
  void set_offset(int64_t offset) { offset_ = offset; }
  const RoutingTrackDirection &direction() const { return direction_; }
  void set_direction(const RoutingTrackDirection &direction) {
    direction_ = direction;
  }
  int64_t pitch() const { return pitch_; }
  void set_pitch(int64_t pitch) { pitch_ = pitch; }
  int64_t min_separation() const { return min_separation_; }
  void set_min_separation(int64_t min_separation) {
    min_separation_ = min_separation;
  }

 private:
  geometry::Layer layer_;
  std::optional<geometry::Layer> pin_layer_;
  geometry::Rectangle area_;
  int64_t wire_width_;
  int64_t offset_;
  RoutingTrackDirection direction_;
  int64_t pitch_;
  int64_t min_separation_;
};

}   // namespace bfg

#endif  // ROUTING_LAYER_INFO_H_
