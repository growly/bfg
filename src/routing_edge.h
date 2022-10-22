#ifndef ROUTING_EDGE_H_
#define ROUTING_EDGE_H_

#include <set>
#include <vector>

#include "geometry/layer.h"

namespace bfg {

class RoutingTrack;
class RoutingVertex;

class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : available_(true),
      track_(nullptr),
      layer_(-1),
      first_(first),
      second_(second),
      cost_(-1.0) {
    ApproximateCost();
  }
  ~RoutingEdge() {}

  void PrepareForRemoval();

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() { return first_; }
  RoutingVertex *second() { return second_; }

  void AddSpannedVertex(RoutingVertex *vertex) { spans_.insert(vertex); }
  void RemoveSpannedVertex(RoutingVertex *vertex) { spans_.erase(vertex); }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_layer(const geometry::Layer &layer) { layer_ = layer; }

  const geometry::Layer &ExplicitOrTrackLayer() const;

  // Off-grid edges do not have tracks.
  void set_track(RoutingTrack *track);
  RoutingTrack *track() const { return track_; }

  std::vector<RoutingVertex*> VertexList() const;

 private:
  void ApproximateCost();

  bool available_;

  RoutingTrack *track_;
  geometry::Layer layer_;

  RoutingVertex *first_;
  RoutingVertex *second_;

  // Need some function of the distance between the two vertices (like of
  // length, sheet resistance). This also needs to be computed only once...
  double cost_;

  // The vertices spanned by this Edge.
  std::set<RoutingVertex*> spans_;
};

}  // namespace bfg

#endif  // ROUTING_EDGE_H_
