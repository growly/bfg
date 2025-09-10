#include "routing_blockage_cache.h"

#include <map>
#include <set>
#include <optional>

#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_track_direction.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"

namespace bfg {

// TODO(aryap): We have to factor out of the ApplyBlockage et. al. functions
// all the stuff that needs to be checked to determine:
//  - if the blockage overlaps a vertex (so the vertex can be used to
//  connect to it directly)
//  - if the blokkage inhibits a vertex, meaning it comes too close for the
//  vertex to accomodate a via, and in what directions.
template<typename T>
void RoutingBlockageCache::ApplyBlockageToOneVertex(
    const RoutingGridBlockage<T> blockage,
    const RoutingVertex *vertex,
    std::optional<RoutingTrackDirection> access_direction) {
  const geometry::Layer &layer = blockage.shape().layer();

  // Check if the blockage overlaps the vertex completely:
  if (blockage.IntersectsPoint(vertex->centre(), 0)) {
    blocked_vertices_[vertex].AddUser(blockage);
    //VLOG(16) << "Blockage: " << blockage.shape()
    //         << " intersects " << vertex->centre()
    //         << " with margin " << 0;
  }

  // If it doesn't, check if there are viable directions the vertex can
  // still be used in:
  static const std::set<RoutingTrackDirection> kAllDirections = {
      RoutingTrackDirection::kTrackHorizontal,
      RoutingTrackDirection::kTrackVertical};

  // Yikes. This will make a copy of kAllDirections.
  std::set<RoutingTrackDirection> test_directions = access_direction ?
      std::set<RoutingTrackDirection>{*access_direction} : kAllDirections;

  const std::string &net = blockage.shape().net();
  for (const auto &direction : test_directions) {
    // We use the RoutingGridBlockage to do a hit test; set
    // exceptional_nets = nullopt so that no exception is made.
    if (blockage.Blocks(*vertex, std::nullopt, direction)) {
      blocked_vertices_[vertex].AddInhibitor(direction, blockage);
      //VLOG(16) << "Blockage: " << blockage.shape()
      //         << " blocks " << vertex->centre()
      //         << " with padding=" << blockage.padding() << " in "
      //         << direction << " direction";
      continue;
    }
  }
}

void RoutingBlockageCache::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding) {
  // Find possibly-affected vertices.
  std::vector<const RoutingVertex*> vertices =
      DeterminePossiblyAffectedVertices(rectangle, padding);

  int64_t min_separation = grid_.GetMinSeparation(rectangle.layer());

  //RoutingGridBlockage<geometry::Rectangle> *blockage =
  //    new RoutingGridBlockage<geometry::Rectangle>(
  //        grid_, rectangle, padding + min_separation);
  //rectangle_blockages_.emplace_back(blockage);

  for (const RoutingVertex *vertex : vertices) {
    //ApplyBlockageToOneVertex(*blockage, vertex, std::nullopt);
  }

  // Find possibly-affected edges.
  // TODO(aryap): We need to ask RoutingTracks to tell us this?
}

std::vector<const RoutingVertex*>
RoutingBlockageCache::DeterminePossiblyAffectedVertices(
    const geometry::Rectangle &rectangle,
    int64_t padding) const {
  const geometry::Layer &layer = rectangle.layer();

  std::set<RoutingVertex*> targets;

  // Check on-grid vertices for each RoutingGridGeometry in which the shape is
  // involved:
  for (const auto &outer : grid_.grid_geometry_by_layers()) {
    for (const auto &inner : outer.second) {
      const RoutingGridGeometry &grid_geometry = inner.second;
      if (outer.first == layer || inner.first == layer) {
        std::set<RoutingVertex*> enveloping_vertices;
        grid_geometry.EnvelopingVertices(
            rectangle,
            &enveloping_vertices,
            padding + search_window_margin_,
            1);   // Number of concentric vertex layers.
        targets.insert(enveloping_vertices.begin(),
                       enveloping_vertices.end());
      }
    }
  }

  // Check off-grid vertices:
  int64_t radius = std::max(
      rectangle.Width(), rectangle.Height()) + padding + search_window_margin_;
  std::vector<RoutingVertex*> nearby_off_grid =
      grid_.off_grid_vertices().FindNearby(rectangle.centre(), radius);

  // Yikes cv-fuckery.
  std::vector<const RoutingVertex*> const_targets(targets.size());
  for (RoutingVertex *vertex : targets) {
    const_targets.push_back(vertex);
  }
  for (RoutingVertex *vertex : nearby_off_grid) {
    const_targets.push_back(vertex);
  }

  return const_targets;
}

}   // namespace bfg
