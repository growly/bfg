#include "routing_grid_geometry.h"

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include <glog/logging.h>

#include "geometry/layer.h"
#include "geometry/line.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_layer_info.h"
#include "routing_track_direction.h"

namespace bfg {

RoutingGridGeometry::RoutingGridGeometry()
    : x_offset_(0),
      x_pitch_(0),
      x_min_(0),
      x_max_(0),
      x_start_(0),
      max_column_index_(0),
      y_offset_(0),
      y_pitch_(0),
      y_min_(0),
      y_max_(0),
      y_start_(0),
      max_row_index_(0),
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

std::tuple<int64_t, int64_t, int64_t, int64_t>
RoutingGridGeometry::MapToBoundingGridIndices(const geometry::Point &point)
    const {
  int64_t column_lower = std::floor(static_cast<double>(
      point.x() - x_start_) / static_cast<double>(x_pitch_));
  int64_t row_lower = std::floor(static_cast<double>(
      point.y() - y_start_) / static_cast<double>(y_pitch_));
  int64_t column_upper = std::ceil(static_cast<double>(
      point.x() - x_start_) / static_cast<double>(x_pitch_));
  int64_t row_upper = std::ceil(static_cast<double>(
      point.y() - y_start_) / static_cast<double>(y_pitch_));

  return std::make_tuple(column_lower, column_upper, row_lower, row_upper);
}

std::tuple<int64_t, int64_t, int64_t, int64_t>
RoutingGridGeometry::MapToBoundingGridIndices(
    const geometry::Rectangle &rectangle) const {
  int64_t column_lower = std::floor(static_cast<double>(
      rectangle.lower_left().x() - x_start_) / static_cast<double>(x_pitch_));
  int64_t row_lower = std::floor(static_cast<double>(
      rectangle.lower_left().y() - y_start_) / static_cast<double>(y_pitch_));
  int64_t column_upper = std::ceil(static_cast<double>(
      rectangle.upper_right().x() - x_start_) / static_cast<double>(x_pitch_));
  int64_t row_upper = std::ceil(static_cast<double>(
      rectangle.upper_right().y() - y_start_) / static_cast<double>(y_pitch_));

  return std::make_tuple(column_lower, column_upper, row_lower, row_upper);
}

std::tuple<int64_t, int64_t, int64_t, int64_t>
RoutingGridGeometry::MapToBoundingGridIndices(
    const geometry::Polygon &polygon) const {
  geometry::Rectangle bounding_box = polygon.GetBoundingBox();
  return MapToBoundingGridIndices(bounding_box);
}

void RoutingGridGeometry::BoundGridIndices(int64_t num_concentric_layers,
                                        int64_t *column_lower,
                                           int64_t *column_upper,
                                           int64_t *row_lower,
                                           int64_t *row_upper) const {
  // Impose restriction of real bounded grid with indices [0, max]. Push out
  // limits by (num_concentric_layers - 1) to include (num_concentric_layers -
  // 1) additional layers of grid indices beyond the first encapsulating layer.
  --num_concentric_layers;

  *column_lower = std::max(std::min(
      (*column_lower - num_concentric_layers), max_column_index_), 0L);
  *column_upper = std::max(std::min(
      (*column_upper + num_concentric_layers), max_column_index_), 0L);
  *row_lower = std::max(std::min(
      (*row_lower - num_concentric_layers), max_row_index_), 0L);
  *row_upper = std::max(std::min(
      (*row_upper + num_concentric_layers), max_row_index_), 0L);
}

void RoutingGridGeometry::NearestTracks(
    const geometry::Point &point,
    std::set<RoutingTrack*> *horizontal,
    std::set<RoutingTrack*> *vertical) const {
  std::set<size_t> horizontal_indices;
  std::set<size_t> vertical_indices;
  NearestTrackIndices(point, &horizontal_indices, &vertical_indices);

  for (size_t index : horizontal_indices) {
    if (index >= horizontal_tracks_by_index_.size())
      continue;
    horizontal->insert(horizontal_tracks_by_index_[index]);
  }
  for (size_t index : vertical_indices) {
    if (index >= vertical_tracks_by_index_.size())
      continue;
    vertical->insert(vertical_tracks_by_index_[index]);
  }
}

void RoutingGridGeometry::NearestTrackIndices(
    const geometry::Point &point,
    std::set<size_t> *horizontal,
    std::set<size_t> *vertical,
    int64_t num_concentric_layers) const {
  // This departure from the style used elsewhere in the code is a personal
  // experiment to see if I like the sauce:
  auto [column_lower, column_upper, row_lower, row_upper] = 
      MapToBoundingGridIndices(point);
  // (I think I like the sauce.)

  BoundGridIndices(num_concentric_layers,
                   &column_lower,
                   &column_upper,
                   &row_lower,
                   &row_upper);

  // Columns (vertical tracks):
  vertical->insert(column_lower);
  vertical->insert(column_upper);

  // Rows (horizontal tracks):
  horizontal->insert(row_lower);
  horizontal->insert(row_upper);
}

void RoutingGridGeometry::ComputeForLayers(
    const RoutingLayerInfo &horizontal_info,
    const RoutingLayerInfo &vertical_info) {
  horizontal_layer_ = horizontal_info.layer();
  vertical_layer_ = vertical_info.layer();

  // Determine the area over which the grid is valid.
  geometry::Rectangle overlap =
      horizontal_info.area().OverlapWith(vertical_info.area());
  LOG(INFO) << "Layers " << horizontal_info.layer()
            << ", " << vertical_info.layer() << " overlap on " << overlap;
  
  // NOTE(aryap): We used to calculate 'offset' as a difference from the origin,
  // making the routing area a sort of mask that removes tracks outside the
  // defined bounds.  Then finding the start coordinate was a matter of finding
  // the first track position that would've landed within the masked area, as
  // follows:
  //
  //                        x_min   x_start
  //                            v   v
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //  origin   |      |      |  +   |      |
  //  O -----> | ---> | ---> | -+-> | ---> |
  //    x_offset x_pitch        ^
  //                            start of grid boundary
  //
  // x_start = x_min + (x_pitch - modulo(x_min - x_offset, x_pitch));
  //
  // But it turns out that this not at all intuitive, so instead we just treat
  // the offset as relative to the start of the routing area bounds. Clients of
  // this code should understand where their routing area is going to end up, so
  // setting offset in this way has a more direct relationship with where the
  // tracks end up.
  //
  //               x_min   x_start
  //                   v   v
  //         |      |  +   |      |
  //         |      |  +   |      |
  //         |      |  +   |      |
  //         |      |  +   |      |
  //         |      |  +   |      |
  //  O <--- | <--- | <+-- | ---> |
  //                   ^     ^ x_pitch
  //                   start of grid boundary
  //
  x_offset_ = vertical_info.offset();
  x_pitch_ = vertical_info.pitch();
  LOG_IF(FATAL, x_pitch_ == 0)
      << "Routing pitch for layer " << vertical_info.layer() << " is 0";
  x_min_ = overlap.lower_left().x();
  x_start_ = x_min_ + x_offset_;
  x_max_ = overlap.upper_right().x();
  max_column_index_ = (x_max_ - x_start_) / x_pitch_;
  
  y_offset_ = horizontal_info.offset();
  y_pitch_ = horizontal_info.pitch();
  LOG_IF(FATAL, y_pitch_ == 0)
      << "Routing pitch for layer " << horizontal_info.layer() << " is 0";
  y_min_ = overlap.lower_left().y();
  y_start_ = y_min_ + y_offset_;
  y_max_ = overlap.upper_right().y();
  max_row_index_ = (y_max_ - y_start_) / y_pitch_;

  vertices_by_grid_position_ = std::vector<std::vector<RoutingVertex*>>(
      max_column_index_ + 1, std::vector<RoutingVertex*>(
          max_row_index_ + 1, nullptr));
}

std::set<RoutingVertex*> RoutingGridGeometry::ConnectablePerimeter(
    const geometry::Polygon &polygon) const {
  std::set<RoutingVertex*> vertices;
  auto check_vertex = [&](int64_t i, int64_t j) {
    RoutingVertex *vertex = VertexAt(i, j);
    if (!vertex) {
      LOG(WARNING) << "There is no vertex at grid " << i << ", " << j;
      return false;
    }
    if (vertex->available() ||
        polygon.net() != "" && vertex->connectable_net() &&
            *vertex->connectable_net() == polygon.net()) {
      // This vertex can be used to connect to the shape, with a jog.
      VLOG(17) << "vertex at " << i << ", " << j << " can be used";
      vertices.insert(vertex);
      return true;
    }
    return false;
  };

  auto [i_lower, i_upper, j_lower, j_upper] = MapToBoundingGridIndices(polygon);
  // Iterate over columns:
  for (int64_t i = i_lower; i <= i_upper; ++i) {
    geometry::Line vertical_line = VerticalLineThrough(i);

    std::vector<geometry::PointPair> points =
        polygon.IntersectingPoints(vertical_line);

    // TODO(aryap): Test dealing with the problem of having two intersecting
    // point pairs that are very close. I *think* this deals with it.
    // TODO(aryap): The connection point could be half-way between the two
    // intersection points in the pair.

    for (size_t k = 0; k < points.size(); ++k) {
      int64_t lower_bound = 0;
      if (k > 0) {
        std::tie(std::ignore, std::ignore, std::ignore, lower_bound) =
            MapToBoundingGridIndices(points[k - 1].second);
      }

      auto &pair = points[k];
      int64_t row_lower;
      std::tie(std::ignore, std::ignore, row_lower, std::ignore) =
          MapToBoundingGridIndices(pair.first);
      // Check vertices from the low-side upper-limit down.
      for (int64_t j = row_lower; j >= lower_bound; --j) {
        if (check_vertex(i, j))
          break;
      }

      int64_t row_upper;
      std::tie(std::ignore, std::ignore, std::ignore, row_upper) =
          MapToBoundingGridIndices(pair.second);
      int64_t upper_bound = max_row_index_;
      if (k < points.size() - 1) {
        std::tie(std::ignore, std::ignore, upper_bound, std::ignore) =
            MapToBoundingGridIndices(points[k + 1].first);
      }
      for (int64_t j = row_upper; j <= upper_bound; ++j) {
        if (check_vertex(i, j))
          break;
      }
    }
  }
  // Iterate over rows:
  for (int64_t j = j_lower; j <= j_upper; ++j) {
    geometry::Line horizontal_line = HorizontalLineThrough(j);

    std::vector<geometry::PointPair> points =
        polygon.IntersectingPoints(horizontal_line);

    for (size_t k = 0; k < points.size(); ++k) {
      int64_t lower_bound = 0;
      if (k > 0) {
        std::tie(std::ignore, lower_bound, std::ignore, std::ignore) =
            MapToBoundingGridIndices(points[k - 1].second);
      }

      auto &pair = points[k];
      int64_t column_lower;
      std::tie(column_lower, std::ignore, std::ignore, std::ignore) =
          MapToBoundingGridIndices(pair.first);
      // Check vertices from the low-side upper-limit down.
      for (int64_t i = column_lower; i >= lower_bound; --i) {
        if (check_vertex(i, j))
          break;
      }

      int64_t column_upper;
      std::tie(std::ignore, column_upper, std::ignore, std::ignore) =
          MapToBoundingGridIndices(pair.second);
      int64_t upper_bound = max_column_index_;
      if (k < points.size() - 1) {
        std::tie(upper_bound, std::ignore, std::ignore, std::ignore) =
            MapToBoundingGridIndices(points[k + 1].first);
      }
      for (int64_t i = column_upper; i <= upper_bound; ++i) {
        if (check_vertex(i, j))
          break;
      }
    }
  }
  return vertices;
}

int64_t RoutingGridGeometry::ColumnCoordinate(size_t column_index) const {
  return x_start_ + x_pitch_ * column_index;
}

int64_t RoutingGridGeometry::RowCoordinate(size_t row_index) const {
  return y_start_ + y_pitch_ * row_index;
}

geometry::Point RoutingGridGeometry::PointAt(
    size_t column_index, size_t row_index) const {
  return {ColumnCoordinate(column_index), RowCoordinate(row_index)};
}

std::set<RoutingTrack*> RoutingGridGeometry::CrossedTracks(
    const geometry::Polygon &polygon) const {
  std::set<RoutingTrack*> tracks;
  auto [i_lower, i_upper, j_lower, j_upper] = MapToBoundingGridIndices(polygon);
  // Iterate over columns:
  for (int64_t i = i_lower; i <= i_upper; ++i) {
    RoutingTrack *track = vertical_tracks_by_index_[i];
    if (!track)
      continue;
    geometry::Line vertical_line = VerticalLineThrough(i);

    std::vector<geometry::PointPair> points =
        polygon.IntersectingPoints(vertical_line);
    if (!points.empty()) {
      tracks.insert(track);
    }
  }
  for (int64_t j = j_lower; j <= j_upper; ++j) {
    RoutingTrack *track = horizontal_tracks_by_index_[j];
    if (!track)
      continue;
    geometry::Line horizontal_line = HorizontalLineThrough(j);

    std::vector<geometry::PointPair> points =
        polygon.IntersectingPoints(horizontal_line);
    if (!points.empty())  {
      tracks.insert(track);
    }
  }
  return tracks;
}

void RoutingGridGeometry::EnvelopingVertexIndices(
    const geometry::Point &point,
    std::set<std::pair<size_t, size_t>> *vertices,
    int64_t padding,
    int64_t num_concentric_layers) const {
  if (padding != 0) {
    int64_t keep_out_width = 2 * padding;
    geometry::Rectangle keep_out = geometry::Rectangle(
        point - geometry::Point(padding, padding),
        keep_out_width,
        keep_out_width);
    return EnvelopingVertexIndices(keep_out, vertices, 0);
  }

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
  auto [i_lower, i_upper, j_lower, j_upper] = MapToBoundingGridIndices(point);

  // If the point ends up on a multiple of pitch exactly, there will be no
  // spread in one dimension. We explicitly widen the spread to include +/-1
  // pitch position.
  if (i_upper == i_lower) {
    i_lower = std::max(std::min(i_lower - 1, max_column_index_), 0L);
    i_upper = std::max(std::min(i_upper + 1, max_column_index_), 0L);
  }
  if (j_upper == j_lower) {
    j_lower = std::max(std::min(j_lower - 1, max_row_index_), 0L);
    j_upper = std::max(std::min(j_upper + 1, max_row_index_), 0L);
  }

  BoundGridIndices(
      num_concentric_layers, &i_lower, &i_upper, &j_lower, &j_upper);

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
  for (int64_t i = i_lower; i <= i_upper; ++i) {
    for (int64_t j = j_lower; j <= j_upper; ++j) {
      vertices->insert({i, j});
    }
  }
}

void RoutingGridGeometry::EnvelopingVertexIndices(
    const geometry::Rectangle &rectangle,
    std::set<std::pair<size_t, size_t>> *vertices,
    int64_t padding,
    int64_t num_concentric_layers) const {
  // Find the bounding corner indices of an infinite grid:
  int64_t i_lower = std::floor(
      static_cast<double>(rectangle.lower_left().x() - padding - x_start_) /
      static_cast<double>(x_pitch_));
  int64_t j_lower = std::floor(
      static_cast<double>(rectangle.lower_left().y() - padding - y_start_) /
      static_cast<double>(y_pitch_));

  int64_t i_upper = std::ceil(
      static_cast<double>(rectangle.upper_right().x() + padding - x_start_) /
      static_cast<double>(x_pitch_));
  int64_t j_upper = std::ceil(
      static_cast<double>(rectangle.upper_right().y() + padding - y_start_) /
      static_cast<double>(y_pitch_));

  BoundGridIndices(
      num_concentric_layers, &i_lower, &i_upper, &j_lower, &j_upper);

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
    std::set<std::pair<size_t, size_t>> *vertices,
    int64_t padding,
    int64_t num_concentric_layers) const {
  // There is the smart way to do this, and then there is this way.
  return EnvelopingVertexIndices(polygon.GetBoundingBox(), vertices, padding);

  // The smart way is to do a sort of raster scan along all of the rows which
  // the polygon's bounding box spans. That will at least remove areas inside
  // large concave parts of the polygon.
}

void RoutingGridGeometry::VerticesAt(
    const std::set<std::pair<size_t, size_t>> &indices,
    std::set<RoutingVertex*> *vertices) const {
  for (const auto index_pair : indices) {
    RoutingVertex *vertex = VertexAt(index_pair.first, index_pair.second);
    if (!vertex)
      continue;
    vertices->insert(vertex);
  }
}

geometry::Line RoutingGridGeometry::HorizontalLineThrough(size_t row_index)
  const {
  int64_t y = RowCoordinate(row_index);
  return geometry::Line({x_min_, y}, {x_max_, y});
}

geometry::Line RoutingGridGeometry::VerticalLineThrough(size_t column_index)
  const {
  int64_t x = ColumnCoordinate(column_index);
  return geometry::Line({x, y_min_}, {x, y_max_});
}

void RoutingGridGeometry::AssignVertexAt(
    size_t column_index, size_t row_index, RoutingVertex *vertex) {
  LOG_IF(FATAL, column_index > max_column_index_)
    << "column_index (" << column_index << ") out of bounds (max: "
    << max_column_index_ << ")";
  LOG_IF(FATAL, row_index > max_row_index_)
    << "row_index (" << row_index << ") out of bounds (max: "
    << max_row_index_ << ")";
  vertices_by_grid_position_[column_index][row_index] = vertex;
}

RoutingVertex *RoutingGridGeometry::VertexAt(const geometry::Point &point)
    const {
  auto [column_lower, column_upper, row_lower, row_upper] = 
      MapToBoundingGridIndices(point);
  if (column_lower != column_upper) {
    return nullptr;
  }
  if (row_lower != row_upper) {
    return nullptr;
  }
  return VertexAt(column_lower, row_lower);
}

RoutingVertex *RoutingGridGeometry::VertexAt(
    size_t column_index, size_t row_index) const {
  if (column_index > max_column_index_ ||
      row_index > max_row_index_) {
    return nullptr;
  }
  RoutingVertex *vertex = vertices_by_grid_position_[column_index][row_index];
  return vertex;
}

}   // namespace bfg
