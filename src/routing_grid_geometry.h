#ifndef ROUTING_GRID_GEOMETRY_H_
#define ROUTING_GRID_GEOMETRY_H_

#include <cstdint>
#include <utility>
#include <set>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"
#include "routing_layer_info.h"

namespace bfg {

class RoutingGridGeometry {
 public:
  RoutingGridGeometry();

  void ComputeForLayers(
      const RoutingLayerInfo &horizontal_info,
      const RoutingLayerInfo &vertical_info);

  void EnvelopingVertexIndices(
      const geometry::Point &point,
      std::set<std::pair<size_t, size_t>> *vertices) const;

  void EnvelopingVertexIndices(
      const geometry::Rectangle &rectangle,
      std::set<std::pair<size_t, size_t>> *vertices) const;

  void EnvelopingVertexIndices(
      const geometry::Polygon &polygon,
      std::set<std::pair<size_t, size_t>> *vertices) const;

  int64_t x_offset() const {
    return x_offset_;
  }
  void set_x_offset(int64_t x_offset) {
    x_offset_ = x_offset;
  }
  int64_t x_pitch() const {
    return x_pitch_;
  }
  void set_x_pitch(int64_t x_pitch) {
    x_pitch_ = x_pitch;
  }
  int64_t x_min() const {
    return x_min_;
  }
  void set_x_min(int64_t x_min) {
    x_min_ = x_min;
  }
  int64_t x_max() const {
    return x_max_;
  }
  void set_x_max(int64_t x_max) {
    x_max_ = x_max;
  }
  int64_t x_start() const {
    return x_start_;
  }
  void set_x_start(int64_t x_start) {
    x_start_ = x_start;
  }

  int64_t y_offset() const {
    return y_offset_;
  }
  void set_y_offset(int64_t y_offset) {
    y_offset_ = y_offset;
  }
  int64_t y_pitch() const {
    return y_pitch_;
  }
  void set_y_pitch(int64_t y_pitch) {
    y_pitch_ = y_pitch;
  }
  int64_t y_min() const {
    return y_min_;
  }
  void set_y_min(int64_t y_min) {
    y_min_ = y_min;
  }
  int64_t y_max() const {
    return y_max_;
  }
  void set_y_max(int64_t y_max) {
    y_max_ = y_max;
  }
  int64_t y_start() const {
    return y_start_;
  }
  void set_y_start(int64_t y_start) {
    y_start_ = y_start;
  }

 private:
  int64_t x_offset_;
  int64_t x_pitch_;
  int64_t x_min_;
  int64_t x_max_;
  int64_t x_start_;

  int64_t max_column_index_;

  int64_t y_offset_;
  int64_t y_pitch_;
  int64_t y_min_;
  int64_t y_max_;
  int64_t y_start_;

  int64_t max_row_index_;

  geometry::Layer horizontal_layer_;
  geometry::Layer vertical_layer_;
};

}   // namespace bfg

#endif  // ROUTING_GRID_GEOMETRY_H_