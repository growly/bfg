#ifndef ROUTING_VERTEX_H_
#define ROUTING_VERTEX_H_

#include <optional>
#include <set>
#include <vector>

#include "glog/logging.h"

#include "geometry/compass.h"
#include "geometry/layer.h"
#include "geometry/point.h"

namespace bfg {

class RoutingEdge;
class RoutingTrack;
class RoutingPath;

class RoutingVertex {
 public:
  static bool Compare(const RoutingVertex &lhs, const RoutingVertex &rhs);
  static bool Compare(RoutingVertex *lhs, RoutingVertex *rhs);

  static std::set<geometry::Layer> CommonLayers(
      const RoutingVertex &lhs, const RoutingVertex &rhs);

  RoutingVertex(const geometry::Point &centre)
      : net_(""),
        available_(true),
        in_edge_(nullptr),
        out_edge_(nullptr),
        cost_(0.0),
        horizontal_track_(nullptr),
        vertical_track_(nullptr),
        contextual_index_(-1),
        grid_position_x_(std::nullopt),
        grid_position_y_(std::nullopt),
        centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.insert(edge); }
  bool RemoveEdge(RoutingEdge *edge);

  //const std::set<RoutingEdge*> &edges() { return edges_; }

  int64_t L1DistanceTo(const geometry::Point &point);

  void AddConnectedLayer(const geometry::Layer &layer) {
    connected_layers_.insert(layer);
    LOG_IF(FATAL, connected_layers_.size() > 2)
        << "RoutingVertex can't connect more than 2 layers";
  }
  bool ConnectsLayer(const geometry::Layer &layer) const {
    return connected_layers_.find(layer) != connected_layers_.end();
  }
  void RemoveConnectedLayer(const geometry::Layer &layer) {
    connected_layers_.erase(layer);
  }
  std::optional<geometry::Layer> ConnectedLayerOtherThan(
      const geometry::Layer &layer) const;
  void set_connected_layers(const std::set<geometry::Layer> &connected_layers) {
    connected_layers_ = connected_layers;
  }
  const std::set<geometry::Layer> &connected_layers() const {
    return connected_layers_;
  }

  RoutingEdge *GetEdgeOnLayer(const geometry::Layer &layer) const;

  // This is the cost of connecting through this vertex (i.e. a via).
  double cost() const { return cost_; }
  void set_cost(const double cost) { cost_ = cost; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::set<RoutingEdge*> edges() const { return edges_; }

  const std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths()
      const {
    return installed_in_paths_;
  }
  std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths() {
    return installed_in_paths_;
  }

  const std::optional<geometry::Layer> &explicit_net_layer() const {
    return explicit_net_layer_;
  }
  void set_explicit_net_layer(
      const std::optional<geometry::Layer> &explicit_net_layer) {
    explicit_net_layer_ = explicit_net_layer;
  }

  bool explicit_net_layer_requires_encap() const {
    return explicit_net_layer_requires_encap_;
  }
  void set_explicit_net_layer_requires_encap(
      bool explicit_net_layer_requires_encap) {
    explicit_net_layer_requires_encap_ = explicit_net_layer_requires_encap;
  }

  bool IsOffGrid() const {
    return !horizontal_track_ || !vertical_track_;
  }

  const geometry::Point &centre() const { return centre_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() const { return net_; }

  void set_available(bool available) { available_ = available; }
  bool available() const { return available_; }

  void set_in_edge(RoutingEdge *edge) { in_edge_ = edge; }
  RoutingEdge *in_edge() const { return in_edge_; }
  void set_out_edge(RoutingEdge *edge) { out_edge_ = edge; }
  RoutingEdge *out_edge() const { return out_edge_; }

  void set_connectable_net(const std::optional<std::string> &connectable_net) {
    connectable_net_ = connectable_net;
  }
  const std::optional<std::string> &connectable_net() const {
    return connectable_net_;
  }

  std::vector<RoutingTrack*> Tracks() const;
  std::vector<RoutingTrack*> TracksOnLayer(const geometry::Layer &layer) const;
  std::vector<RoutingTrack*> TracksInDirection(
      const RoutingTrackDirection &direction) const;

  void set_horizontal_track(RoutingTrack *track) { horizontal_track_ = track; }
  RoutingTrack *horizontal_track() const { return horizontal_track_; }
  void set_vertical_track(RoutingTrack *track) { vertical_track_ = track; }
  RoutingTrack *vertical_track() const { return vertical_track_; }

  void AddNeighbour(const geometry::Compass &position, RoutingVertex *vertex);
  std::set<RoutingVertex*> GetNeighbours(
      const geometry::Compass &position) const;
  std::set<RoutingVertex*> GetNeighbours() const;

  bool ChangesEdge() const { return in_edge_ != out_edge_; }

  const std::optional<size_t> &grid_position_x() const {
    return grid_position_x_;
  }
  void set_grid_position_x(const std::optional<size_t> &grid_position_x) {
    grid_position_x_ = grid_position_x;
  }

  const std::optional<size_t> &grid_position_y() const {
    return grid_position_y_;
  }
  void set_grid_position_y(const std::optional<size_t> &grid_position_y) {
    grid_position_y_ = grid_position_y;
  }

  void ClearAllForcedEncapDirections() {
    forced_encap_directions_.clear();
  }
  void ClearForcedEncapDirection(const geometry::Layer &layer) {
    forced_encap_directions_.erase(layer);
  }
  void SetForcedEncapDirection(
      const geometry::Layer &layer, const RoutingTrackDirection &direction) {
    forced_encap_directions_[layer] = direction;
  }
  std::optional<RoutingTrackDirection> GetForcedEncapDirection(
      const geometry::Layer &layer) {
    auto it = forced_encap_directions_.find(layer);
    if (it == forced_encap_directions_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::map<geometry::Layer, RoutingTrackDirection> &forced_encap_direction() {
    return forced_encap_directions_;
  }

 private:
  struct NeighbouringVertex {
    geometry::Compass position;
    RoutingVertex *vertex;
  };

  // FIXME(aryap): I think we need to store the layers on which the nets are
  // connectible here. It might be possible that a vertex can be used to connect
  // to different nets on different layers. Right now I'm not sure how else to
  // solve the problem of connecting to a net when there's a choice of layers to
  // connect on. Usually it creates a hazard.

  // If the vertex is in use by some route, the name of the net should be here,
  // available_ should be false. in_edge and out_edge should point to the
  // incoming and outgoing edges used for the route through this vertex. If the
  // edge spans this vertex, in_edge_ == out_edge_ == that edge.
  std::string net_;
  bool available_;
  RoutingEdge *in_edge_;
  RoutingEdge *out_edge_;

  // This is the cost of changing layer at this vertex.
  double cost_;

  std::optional<std::string> connectable_net_;

  RoutingTrack *horizontal_track_;
  RoutingTrack *vertical_track_;

  // There are up to 8 neighbouring vertices. 
  std::vector<NeighbouringVertex> neighbours_;

  // This is a minor optimisation to avoid having to key things by pointer.
  // This index should be unique within the RoutingGrid that owns this
  // RoutingVertex for the duration of whatever process requires it.
  size_t contextual_index_;

  // Likewise, these are indices to track the vertex on a grid between two
  // layers. Vertices only actually connect two layers.
  std::optional<size_t> grid_position_x_;
  std::optional<size_t> grid_position_y_;

  // The paths in which this vertex participates. This is every spanned vertex
  // in the path. When multiple paths join, the shared vertex will have mutliple
  // installed paths here.
  std::map<RoutingPath*, std::set<RoutingEdge*>> installed_in_paths_;

  // If defined, this identifies the layer on which the RoutingVertex is known
  // (or expected to be) connected to the assigned net_ label.
  // TODO(aryap): Not handled: being able to connect to the net on multiple
  // layers; needing an encap on some of them.
  std::optional<geometry::Layer> explicit_net_layer_;
  bool explicit_net_layer_requires_encap_;

  geometry::Point centre_;

  // NOTE: A vertex can only ever connect at most 2 layers. A second vertex in
  // the same position is needed to connect to another layer.
  //
  // TODO(aryap): In practice we cheat and simply make the layers jump multiple
  // vias if necessary, and figure it out later. This is mostly taken care of in
  // RoutingPath.
  std::set<geometry::Layer> connected_layers_;

  std::set<RoutingEdge*> edges_;

  std::map<geometry::Layer, RoutingTrackDirection> forced_encap_directions_;
};

std::ostream &operator<<(std::ostream &os, const RoutingVertex &vertex);

}  // namespace bfg

#endif  // ROUTING_VERTEX_H_
