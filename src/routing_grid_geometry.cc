#include "routing_grid_geometry.h"

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include <glog/logging.h>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_layer_info.h"

namespace bfg {

RoutingGridGeometry::RoutingGridGeometry()
    : x_offset_(0),
      x_pitch_(0),
      x_min_(0),
      x_max_(0),
      x_start_(0),
      y_offset_(0),
      y_pitch_(0),
      y_min_(0),
      y_max_(0),
      y_start_(0),
      horizontal_layer_(0),
      vertical_layer_(0) {}

namespace {

// C++ modulo is more 'remainder' than 'modulo' because of how negative numbers
// are handled:
//    mod(-3, 5) = 2
//    rem(-3, 5) = -3 (since -3 / 5 = 0)
// So we have to do this:
int64_t modulo(int64_t a, int64_t b) {
  int64_t remainder = a % b;
  return remainder < 0 ? remainder + b : remainder;
}

}   // namespace

void RoutingGridGeometry::ComputeForLayers(
    const RoutingLayerInfo &horizontal_info,
    const RoutingLayerInfo &vertical_info) {
  // Determine the area over which the grid is valid.
  geometry::Rectangle overlap =
      horizontal_info.area.OverlapWith(vertical_info.area);
  LOG(INFO) << "Layers " << horizontal_info.layer
            << ", " << vertical_info.layer << " overlap on " << overlap;
  
  //                      x_min v   v x_start
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //  origin   |      |      |  +   |      |
  //  O -----> | ---> | ---> | -+-> | ---> |
  //    offset   pitch          ^ start of grid boundary
  //
  x_offset_ = vertical_info.offset;
  x_pitch_ = vertical_info.pitch;
  LOG_IF(FATAL, x_pitch_ == 0)
      << "Routing pitch for layer " << vertical_info.layer << " is 0";
  x_min_ = overlap.lower_left().x();
  x_start_ = x_min_ + (x_pitch_ - modulo(x_min_ - x_offset_, x_pitch_));
  x_max_ = overlap.upper_right().x();
  max_column_index_ = (x_max_ - x_min_) / x_pitch_;
  
  y_offset_ = horizontal_info.offset;
  y_pitch_ = horizontal_info.pitch;
  LOG_IF(FATAL, y_pitch_ == 0)
      << "Routing pitch for layer " << horizontal_info.layer << " is 0";
  y_min_ = overlap.lower_left().y();
  y_start_ = y_min_ + (y_pitch_ - modulo(y_min_ - y_offset_, y_pitch_));
  y_max_ = overlap.upper_right().y();
  max_row_index_ = (y_max_ - y_min_) / y_pitch_;
}

void RoutingGridGeometry::EnvelopingVertexIndices(
    const geometry::Point &point,
    std::set<std::pair<size_t, size_t>> *vertices) const {
  // If we continued the grid infinitely in the cartesian plane, all points
  // would land in a rectangle defined by four grid points closest to the point.
  // The lower left, upper left, lower right, upper right. When the grid isn't
  // infinite and the point falls outside its boundaries, some of these are
  // missing. Since the grid area is also convex we always have 1, 2 or 4 nearby
  // points.
  //
  // Returned coordinates indices are in the form (i, j) where i is the column
  // index (corresponding to x position) and j is the row index (corresponding
  // to x-position), increasing up and right.

  // Find the bounding corner positions of an infinite grid:
  int64_t i_lower = std::floor(static_cast<double>(
      point.x() - x_offset_) / static_cast<double>(x_pitch_));
  int64_t i_upper = std::ceil(static_cast<double>(
      point.x() - x_offset_) / static_cast<double>(x_pitch_));
  int64_t j_lower = std::floor(static_cast<double>(
      point.y() - y_offset_) / static_cast<double>(y_pitch_));

  int64_t j_upper = std::ceil(static_cast<double>(
      point.y() - y_offset_) / static_cast<double>(y_pitch_));

  // Now impose restriction of real bounded grid with indices [0, max]:
  i_lower = std::max(std::min(i_lower, max_column_index_), 0L);
  i_upper = std::max(std::min(i_upper, max_column_index_), 0L);
  j_lower = std::max(std::min(j_lower, max_row_index_), 0L);
  j_upper = std::max(std::min(j_upper, max_row_index_), 0L);

  VLOG(13) << point << ": "
           << i_lower << " <= i <= " << i_upper << "; "
           << j_lower << " <= j <= " << j_upper;

  // We use the set to de-dupe index pairs that have been pushed to the same
  // values, which happens when the point is outside the grid or exactly on one
  // of the grid lines or vertices.
  // std::set<std::pair<int64_t, int64_t>> corners = {
  //     {i_lower, j_lower},
  //     {i_lower, j_upper},
  //     {i_upper, j_lower},
  //     {i_upper, j_upper}
  // };
  // vertices->insert(corners.begin(), corners.end());
  vertices->insert({i_lower, j_lower});
  vertices->insert({i_lower, j_upper});
  vertices->insert({i_upper, j_lower});
  vertices->insert({i_upper, j_upper});
}

void RoutingGridGeometry::EnvelopingVertexIndices(
    const geometry::Rectangle &rectangle,
    std::set<std::pair<size_t, size_t>> *vertices) const {
  // Find the bounding corner positions of an infinite grid:
  int64_t i_lower = std::floor(static_cast<double>(
      rectangle.lower_left().x() - x_offset_) / static_cast<double>(x_pitch_));
  int64_t i_upper = std::ceil(static_cast<double>(
      rectangle.upper_right().x() - x_offset_) / static_cast<double>(x_pitch_));

  int64_t j_lower = std::floor(static_cast<double>(
      rectangle.lower_left().y() - y_offset_) / static_cast<double>(y_pitch_));

  int64_t j_upper = std::ceil(static_cast<double>(
      rectangle.upper_right().y() - y_offset_) / static_cast<double>(y_pitch_));

  // Now impose restriction of real bounded grid with indices [0, max]:
  i_lower = std::max(std::min(i_lower, max_column_index_), 0L);
  i_upper = std::max(std::min(i_upper, max_column_index_), 0L);
  j_lower = std::max(std::min(j_lower, max_row_index_), 0L);
  j_upper = std::max(std::min(j_upper, max_row_index_), 0L);

  VLOG(13) << rectangle << ": "
           << i_lower << " <= i <= " << i_upper << "; "
           << j_lower << " <= j <= " << j_upper;

  for (size_t i = i_lower; i <= i_upper; ++i) {
    for (size_t j = j_lower; j <= j_upper; ++j) {
      vertices->insert({i, j});
    }
  }
}

void RoutingGridGeometry::EnvelopingVertexIndices(
    const geometry::Polygon &polygon,
    std::set<std::pair<size_t, size_t>> *vertices) const {
  // There is a smart way to do this, and then there is this way.
  return EnvelopingVertexIndices(polygon.GetBoundingBox(), vertices);

  // The smart way is to do a sort of raster scan along all of the rows which
  // the polygon's bounding box spans. That will at least remove areas inside
  // large concave parts of the polygon.
}

}   // namespace bfg
