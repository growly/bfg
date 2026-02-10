#ifndef ROUTING_GRID_BLOCKAGE_H_
#define ROUTING_GRID_BLOCKAGE_H_

#include <memory>
#include <optional>
#include <vector>

#include "equivalent_nets.h"
#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_edge.h"
#include "routing_track_blockage.h"
#include "routing_track_direction.h"
#include "routing_vertex.h"

namespace bfg {

// Forward declarations
class RoutingGrid;
class RoutingTrack;

template<typename T>
class RoutingGridBlockage {
 public:
  RoutingGridBlockage(
      const RoutingGrid &routing_grid, const T& shape, int64_t padding)
      : routing_grid_(routing_grid),
        shape_(shape),
        padding_(padding),
        blockage_layers_({shape.layer()}) {}

  RoutingGridBlockage(
      const RoutingGrid &routing_grid, 
      const T& shape,
      const std::set<geometry::Layer> &blockage_layers,
      int64_t padding)
      : routing_grid_(routing_grid),
        shape_(shape),
        padding_(padding),
        blockage_layers_(blockage_layers.begin(), blockage_layers.end()) {}

  ~RoutingGridBlockage();

  // Tests intersection of the blockage with a given point. This differs from
  // the RoutingVertex and the RoutingEdge tests because no footprint is
  // assumed: the point is either in the shape or not. (Margin is applied to the
  // shape before testing and can be negative, if the point needs to be "more
  // inside" to be considered a hit.)
  bool IntersectsPoint(const geometry::Point &point, int64_t margin) const;

  bool BlocksWithoutPadding(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets =
          std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
    return Blocks(vertex, 0, exceptional_nets, access_direction);
  }
  bool BlocksWithoutPadding(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &exceptional_nets = std::nullopt)
      const {
    return Blocks(edge, 0, exceptional_nets);
  }

  bool Blocks(
      const RoutingVertex &vertex,
      const std::optional<EquivalentNets> &exceptional_nets =
          std::nullopt,
      const std::optional<RoutingTrackDirection> &access_direction =
          std::nullopt)
      const {
    return Blocks(vertex, padding_, exceptional_nets, access_direction);
  }
  bool Blocks(
      const RoutingEdge &edge,
      const std::optional<EquivalentNets> &exceptional_nets) const {
    return Blocks(edge, padding_, exceptional_nets);
  }
  bool Blocks(
      const geometry::Rectangle &footprint,
      const std::optional<EquivalentNets> &exceptional_nets) const {
    return Blocks(footprint, padding_, exceptional_nets);
  }

  // Takes ownership of the given RoutingTrackBlockage. Store the RoutingTrack
  // so that we can remove the blockage from the track if we need do.
  void AddChildTrackBlockage(
      RoutingTrack *track, RoutingTrackBlockage *blockage);

  void ClearChildTrackBlockages();

  const std::set<geometry::Layer> &blockage_layers() const {
    return blockage_layers_;
  }

  const T& shape() const { return shape_; }
  const int64_t &padding() const { return padding_; }

 private:
  bool Blocks(
      const RoutingVertex &vertex,
      int64_t padding,
      const std::optional<EquivalentNets> &exceptional_nets,
      const std::optional<RoutingTrackDirection> &access_direction) const;
  bool Blocks(
      const RoutingEdge &edge,
      int64_t padding,
      const std::optional<EquivalentNets> &exceptional_nets) const;
  bool Blocks(
      const geometry::Rectangle &footprint,
      int64_t padding,
      const std::optional<EquivalentNets> &exceptional_nets) const;

  const RoutingGrid &routing_grid_;
  // We store a copy of the shape. We can't store a reference because callers
  // can do cowboy shit.
  //
  // TODO(aryap): YIKES. Can we avoid this?! This requires us to store a copy of
  // every shape on the grid and also makes looking up existing shapes much
  // harder!
  const T shape_;
  int64_t padding_;

  std::set<geometry::Layer> blockage_layers_;

  std::vector<std::pair<RoutingTrack*, std::unique_ptr<RoutingTrackBlockage>>>
      child_track_blockages_;
};

}  // namespace bfg

#endif  // ROUTING_GRID_BLOCKAGE_H_
