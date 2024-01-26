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
    connected_layers_.push_back(layer);
    LOG_IF(FATAL, connected_layers_.size() > 2)
        << "RoutingVertex can't connect more than 2 layers";
  }
  const std::vector<geometry::Layer> &connected_layers() const {
    return connected_layers_;
  }

  // This is the cost of connecting through this vertex (i.e. a via).
  double cost() const { return cost_; }
  void set_cost(const double cost) { cost_ = cost; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::set<RoutingEdge*> edges() const { return edges_; }

  const std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths() const {
    return installed_in_paths_;
  }
  std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths() {
    return installed_in_paths_;
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

  void set_horizontal_track(RoutingTrack *track) { horizontal_track_ = track; }
  RoutingTrack *horizontal_track() const { return horizontal_track_; }
  void set_vertical_track(RoutingTrack *track) { vertical_track_ = track; }
  RoutingTrack *vertical_track() const { return vertical_track_; }

  void AddNeighbour(const geometry::Compass &position, RoutingVertex *vertex);
  std::set<RoutingVertex*> GetNeighbours(
      const geometry::Compass &position) const;

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

 private:
  struct NeighbouringVertex {
    geometry::Compass position;
    RoutingVertex *vertex;
  };

  // TODO(aryap): 

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

  geometry::Point centre_;

  // NOTE: A vertex can only ever connect 2 layers. A second vertex in the same
  // position is needed to connect to another layer.
  std::vector<geometry::Layer> connected_layers_;

  std::set<RoutingEdge*> edges_;
};

std::ostream &operator<<(std::ostream &os, const RoutingVertex &vertex);

}  // namespace bfg

#endif  // ROUTING_VERTEX_H_
