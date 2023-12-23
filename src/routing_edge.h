#ifndef ROUTING_EDGE_H_
#define ROUTING_EDGE_H_

#include <optional>
#include <set>
#include <vector>

#include "geometry/layer.h"
#include "geometry/rectangle.h"

namespace bfg {

class RoutingTrack;
class RoutingVertex;

class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : in_use_by_net_(std::nullopt),
      blocked_(false),
      track_(nullptr),
      layer_(-1),
      first_(first),
      second_(second),
      cost_(0.0) {
    ApproximateCost();
  }
  ~RoutingEdge() {}

  void PrepareForRemoval();

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() const { return first_; }
  RoutingVertex *second() const { return second_; }

  // Self-contained utility for imagining this edge as a rectilinear rectangle
  // of the given width.
  std::optional<geometry::Rectangle> AsRectangle(int64_t width) const;

  void set_in_use_by_net(const std::optional<std::string> &in_use_by_net) {
    in_use_by_net_ = in_use_by_net;
  }
  const std::optional<std::string> &in_use_by_net() const {
    return in_use_by_net_;
  }
  void set_blocked(bool blocked) { blocked_ = blocked; }
  bool blocked() const { return blocked_; }

  bool Available() const { return !blocked_ && !in_use_by_net_; }
  void ResetStatus() {
    in_use_by_net_ = std::nullopt;
    blocked_ = false;
  }

  void set_layer(const geometry::Layer &layer) { layer_ = layer; }
  const geometry::Layer &layer() const { return layer_; }

  const geometry::Layer &ExplicitOrTrackLayer() const;

  // Off-grid edges do not have tracks.
  void set_track(RoutingTrack *track);
  RoutingTrack *track() const { return track_; }

  std::vector<RoutingVertex*> VertexList() const;

 private:
  void ApproximateCost();

  std::optional<std::string> in_use_by_net_;
  bool blocked_;

  RoutingTrack *track_;
  geometry::Layer layer_;

  RoutingVertex *first_;
  RoutingVertex *second_;

  // Need some function of the distance between the two vertices (like of
  // length, sheet resistance). This also needs to be computed only once...
  double cost_;
};

std::ostream &operator<<(std::ostream &os, const RoutingEdge &edge);

}  // namespace bfg

#endif  // ROUTING_EDGE_H_
