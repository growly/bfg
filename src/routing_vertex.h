#ifndef ROUTING_VERTEX_H_
#define ROUTING_VERTEX_H_

#include <set>
#include <vector>

#include "geometry/layer.h"
#include "geometry/point.h"

namespace bfg {

class RoutingEdge;
class RoutingTrack;

class RoutingVertex {
 public:
  RoutingVertex(const geometry::Point &centre)
      : net_(""),
        available_(true),
        in_edge_(nullptr),
        out_edge_(nullptr),
        horizontal_track_(nullptr),
        vertical_track_(nullptr),
        contextual_index_(-1),
        centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.insert(edge); }
  bool RemoveEdge(RoutingEdge *edge);

  //const std::set<RoutingEdge*> &edges() { return edges_; }

  uint64_t L1DistanceTo(const geometry::Point &point);

  void AddConnectedLayer(const geometry::Layer &layer) {
    connected_layers_.push_back(layer);
  }
  const std::vector<geometry::Layer> &connected_layers() { return connected_layers_; }

  // This is the cost of connecting through this vertex (i.e. a via).
  double cost() const { return 1.0; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::set<RoutingEdge*> edges() const { return edges_; }

  const geometry::Point &centre() const { return centre_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_in_edge(RoutingEdge *edge) { in_edge_ = edge; }
  RoutingEdge *in_edge() const { return in_edge_; }
  void set_out_edge(RoutingEdge *edge) { out_edge_ = edge; }
  RoutingEdge *out_edge() const { return out_edge_; }

  void set_horizontal_track(RoutingTrack *track) { horizontal_track_ = track; }
  RoutingTrack *horizontal_track() const { return horizontal_track_; }
  void set_vertical_track(RoutingTrack *track) { vertical_track_ = track; }
  RoutingTrack *vertical_track() const { return vertical_track_; }

 private:
  // If the vertex is in use by some route, the name of the net should be here,
  // available_ should be false. in_edge and out_edge should point to the
  // incoming and outgoing edges used for the route through this vertex.
  std::string net_;
  bool available_;
  RoutingEdge *in_edge_;
  RoutingEdge *out_edge_;

  RoutingTrack *horizontal_track_;
  RoutingTrack *vertical_track_;

  // This is a minor optimisation to avoid having to key things by pointer.
  // This index should be unique within the RoutingGrid that owns this
  // RoutingVertex for the duration of whatever process requires it.
  size_t contextual_index_;

  geometry::Point centre_;
  std::vector<geometry::Layer> connected_layers_;
  std::set<RoutingEdge*> edges_;
};


}  // namespace bfg

#endif  // ROUTING_VERTEX_H_