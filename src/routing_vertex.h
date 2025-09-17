#ifndef ROUTING_VERTEX_H_
#define ROUTING_VERTEX_H_

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <variant>
#include <vector>

#include "glog/logging.h"

#include "geometry/compass.h"
#include "geometry/layer.h"
#include "geometry/point.h"

#include "equivalent_nets.h"

namespace bfg {

namespace geometry {
class Polygon;
class Rectangle;
}

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
      : update_tracks_on_blockage_(false),
        cost_(0.0),
        horizontal_track_(nullptr),
        vertical_track_(nullptr),
        contextual_index_(-1),
        grid_position_x_(std::nullopt),
        grid_position_y_(std::nullopt),
        centre_(centre) {
    UpdateCachedStatus();
  }

  typedef std::map<std::string, std::set<geometry::Layer>> NetToLayersMap;

  struct NetWithLayers {
    std::string net;
    std::set<geometry::Layer> layers;
  };

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
  std::optional<geometry::Layer> ConnectableLayerTo(
      const std::function<bool(const geometry::Layer&, const geometry::Layer&)>
          &connectable_by_via_fn,
      const geometry::Layer &layer) const;
  void set_connected_layers(const std::set<geometry::Layer> &connected_layers) {
    connected_layers_ = connected_layers;
  }
  const std::set<geometry::Layer> &connected_layers() const {
    return connected_layers_;
  }

  std::optional<std::set<geometry::Layer>> GetUsingNetLayers(
      const std::string &net) const {
    return GetNetLayers(in_use_by_nets_, net);
  }
  std::optional<std::set<geometry::Layer>> GetBlockingNearbyNetLayers(
      const std::string &net) const {
    return GetNetLayers(blocked_by_nearby_nets_, net);
  }

  // If the vertex was available, the given net is marked as reachable from it
  // (setting connectable_net_) and available_ is set false. If the vertex was
  // unavailable already and connectable_net_ is set for a different net, the
  // connectable_net_ is cleared to avoid indicating that any net can be
  // reached.
  void AddUsingNet(
      const std::string &net,
      bool temporary,
      std::optional<geometry::Layer> layer = std::nullopt,
      std::optional<const geometry::Rectangle*>
          blocking_rectangle = std::nullopt,
      std::optional<const geometry::Polygon*> blocking_polygon = std::nullopt);
  void AddBlockingNet(
      const std::string &net,
      bool temporary,
      std::optional<geometry::Layer> layer = std::nullopt,
      std::optional<const geometry::Rectangle*>
          blocking_rectangle = std::nullopt,
      std::optional<const geometry::Polygon*> blocking_polygon = std::nullopt);

  // Returns true iff the vertex is blocked given the equivalent nets `for_nets`
  // and on layer `layer_or_any`. If `layer_or_any` is std::nullopt, a blockage
  // on _any_ given `for_nets` will return true. This is inverse of checking
  // that the vertex is available for all layers on the given nets. Since we the
  // vertex itself doesn't track blockages, we cannot reason about directions
  // here. (A quick note to self because I have been staring at this too long:
  // checking if ALL things are true should bethe same as negating whether ANY
  // are false.)
  bool IsBlocked(
      const EquivalentNets &for_nets,
      const std::optional<geometry::Layer> &layer_or_any) const;

  void SetForcedBlocked(
      bool blocked,
      bool temporary,
      const std::optional<geometry::Layer> &layer = std::nullopt);

  bool ForcedBlocked(
      const std::optional<geometry::Layer> &layer = std::nullopt) const;

  void ResetTemporaryStatus();

  std::optional<NetWithLayers> InUseBySingleNet(
      const std::optional<geometry::Layer> &layer = std::nullopt) const {
    return PickSingleNetOrNone(UsingNets(layer));
  }

  std::optional<NetWithLayers> BlockedBySingleNearbyNet(
      const std::optional<geometry::Layer> &layer = std::nullopt) const {
    return PickSingleNetOrNone(BlockingNets(layer));
  }

  bool Available() const { return totally_available_; }

  // Check if the vertex is available for a specific net, or ALL given nets, on
  // a specific layer, or ALL layers. If no nets are given, we check that vertex
  // is free of any nets using or blocking it. Two nullopt arguments tests if
  // the vertex is completely available for any net on EVERY layer.
  //
  // The `on_layer` argument is passed to UsingNets(...) and BlockingNets(...),
  // where std::nullopt as the layer argument returns the union of all nets on
  // all layers.
  bool AvailableForAll(
      const std::optional<
          std::reference_wrapper<const EquivalentNets>> &for_nets =
          std::nullopt,
      const std::optional<geometry::Layer> &on_layer = std::nullopt) const;

  // Returns true if the vertex is available for the given nets on any (at least
  // 1) of its connected layers.
  bool AvailableForNetsOnAnyLayer(const EquivalentNets &nets) const {
    LOG_IF(WARNING, connected_layers_.empty())
        << "There are no connected layers on this vertex so this call will "
        << "always fail.";
    for (const geometry::Layer &layer : connected_layers_) {
      if (AvailableForAll(nets, layer)) {
        return true;
      }
    }
    return false;
  }

  // TODO(aryap): This is easier to understand:
  //bool TraversableByNets(const EquivalentNets &nets) const;
  //bool LandableByNets(const EquivalentNets &nets) const;
  // We call a vertex Available if it is either of these things.

  // Returns the edge into or out of this vertex on the given layer, if there is
  // only one. If there are none or there are more than 1, returns nullptr.
  RoutingEdge *GetOnlyEdgeOnLayer(const geometry::Layer &layer) const;

  std::vector<RoutingEdge*> GetEdgesOnLayer(const geometry::Layer &layer) const;

  bool IsOffGrid() const {
    return !horizontal_track_ || !vertical_track_;
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

  bool ChangesEdge() const;

  // Returns the layers switched between by an ingress and egress edge at this
  // vertex. This can be the result of an (in, out) edge pair switching layers,
  // or a path starting/ending here.
  //
  // TODO(aryap): In the event of the latter, one layer comes from the
  // ingress/egress edge and the other comes from the layers connected by the
  // vertex (in connected_layers_). This raises a dilemma. We allow edges to
  // terminate at vertices that don't connect their layers and rely on a via
  // stack later on reach them. This presumes that we can connect to one of the
  // layers in connected_layers_ from the edge layer. To find that layer we need
  // layer connectivity graph, which is heinous. This is part of the physical
  // information stored by the RoutingGrid so maybe we just pass that in? In the
  // meantime we just pass in a functor that will tell us if two layers are
  // directly connectable by a via.
  std::optional<std::pair<geometry::Layer, geometry::Layer>>
  ChangedEdgeAndLayers(
      const std::function<bool(const geometry::Layer&, const geometry::Layer&)>
          &connecable_by_via_fn) const;

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
      const geometry::Layer &layer) const {
    auto it = forced_encap_directions_.find(layer);
    if (it == forced_encap_directions_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::optional<RoutingTrackDirection> GetEncapDirection(
      const geometry::Layer &layer) const;

  // This is the cost of connecting through this vertex (i.e. a via).
  void set_cost(const double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  void set_update_tracks_on_blockage(bool update_tracks_on_blockage) {
    update_tracks_on_blockage_ = update_tracks_on_blockage;
  }

  const std::set<RoutingEdge*> edges() const { return edges_; }

  const std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths()
      const {
    return installed_in_paths_;
  }
  std::map<RoutingPath*, std::set<RoutingEdge*>> &installed_in_paths() {
    return installed_in_paths_;
  }

  bool explicit_net_layer_requires_encap() const {
    return explicit_net_layer_requires_encap_;
  }
  void set_explicit_net_layer_requires_encap(
      bool explicit_net_layer_requires_encap) {
    explicit_net_layer_requires_encap_ = explicit_net_layer_requires_encap;
  }

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

  const geometry::Point &centre() const { return centre_; }

  void AddEdges(RoutingEdge *in, RoutingEdge *out);

  const std::set<std::pair<RoutingEdge*, RoutingEdge*>> &in_out_edges() {
    return in_out_edges_;
  }

  std::map<geometry::Layer, RoutingTrackDirection> &forced_encap_direction() {
    return forced_encap_directions_;
  }

  void set_hosts_port(const std::optional<std::string> port_name) {
    hosts_port_ = port_name;
  }

  const std::optional<std::string> hosts_port() const {
    return hosts_port_;
  }

 private:
  struct NeighbouringVertex {
    geometry::Compass position;
    RoutingVertex *vertex;
  };

  // TODO(aryap): I think we need to store the layers on which the nets are
  // connectible here. It might be possible that a vertex can be used to connect
  // to different nets on different layers. Right now I'm not sure how else to
  // solve the problem of connecting to a net when there's a choice of layers to
  // connect on. Usually it creates a hazard.
  //
  // Not sure about this structure.
  struct NetHazardInfo {
    bool is_temporary;

    // An unspecified layer indicates that the hazard applies to ALL layers.
    std::optional<geometry::Layer> layer;

    // Almost used a union!
    //
    // A std::nullopt Rectangle-type blockage indicates a forced blockage with
    // no particular shape to blame.
    std::variant<
      std::optional<const geometry::Rectangle*>,
      std::optional<const geometry::Polygon*>> blockage;
  };

  void RemoveTemporaryHazardsFrom(
      std::map<std::string, std::vector<NetHazardInfo>> *container);
  std::optional<std::set<geometry::Layer>> GetNetLayers(
      const std::map<std::string, std::vector<NetHazardInfo>> &container,
      const std::string &net) const;

  // Updates totally_blocked_ and totally_available_ based on the using and
  // blocking nets, permanent and temporary.
  void UpdateCachedStatus();

  // Returns all of the nets using this vertex in a map whose index is the net
  // and whose entry is a set of all the layers on which the usage occurs.
  //
  // If the `layer` argument is specified, only the given layer is searched. If
  // it is std::nullopt all layers are considered.
  NetToLayersMap UsingNets(
      const std::optional<geometry::Layer> &layer = std::nullopt) const {
    return SummariseNets(in_use_by_nets_, layer);
  }

  // Returns all of the nets blocking this vertex in a map whose index is the
  // net and whose entry is a set of all the layers on which the usage occurs.
  //
  // If the `layer` argument is specified, only the given layer is searched. If
  // it is std::nullopt all layers are considered.
  NetToLayersMap BlockingNets(
      const std::optional<geometry::Layer> &layer = std::nullopt) const {
    return SummariseNets(blocked_by_nearby_nets_, layer);
  }

  std::optional<NetWithLayers> PickSingleNetOrNone(
      const NetToLayersMap &source) const;

  NetToLayersMap SummariseNets(
      const std::map<std::string, std::vector<NetHazardInfo>> &source,
      const std::optional<geometry::Layer> &layer = std::nullopt) const;

  bool update_tracks_on_blockage_;

  // FIXME(aryap): I think this can be entirely replaced by installed_in_paths_,
  // since it tracks the path using the vertex and a set of edges. The only
  // thing that doesn't track is the directionality of the edges within the
  // path, i.e. which one is in and which is out. But does that matter? As is we
  // have duplicated the bookeeping, and I don't think this is even that
  // important?
  //
  // One entry per path that crosses the vertex.
  std::set<std::pair<RoutingEdge*, RoutingEdge*>> in_out_edges_;

  // The availability of the RoutingVertex for use in a route depends on the net
  // the route is to be used for and whether there is any existing use of or
  // proximity to the vertex.
  //
  // If there are no using nets and no blocking nearby nets, and the vertex is
  // not forced blocked, then it is available for use by any net.
  //
  // If there are conflicting using and blocking nearby nets, or if there are
  // multiple of each, or if it permanently or temporarily forced blocked, then
  // it is not available for any net.
  //
  // If there is only a single using or blocking nearby net, or one of each and
  // they match, then the vertex is available for that single net.
  //
  // We cache availability into totally_available_ since the check is performed
  // by the RoutingGrid very often.
  bool totally_available_;

  std::set<geometry::Layer> forced_blockages_;
  std::set<geometry::Layer> temporary_forced_blockages_;

  // Map of using/blocking net name to whether NetHazardInfo structure that
  // tracks principally whether the usage is permanent or temporary, but also
  // what layer, source blockage it has. Resolution of multiple active hazards
  // is done elsewhere.
  //
  // Permanent usage/blockage trumps temporary usage/blockage.
  std::map<std::string, std::vector<NetHazardInfo>> in_use_by_nets_;
  std::map<std::string, std::vector<NetHazardInfo>> blocked_by_nearby_nets_;

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

  // If set to a string, a port is expected to be placed at the top of the
  // vertex connecting to the host_port_;
  std::optional<std::string> hosts_port_;

  friend std::ostream &operator<<(
      std::ostream &os, const RoutingVertex &vertex);
};

std::ostream &operator<<(std::ostream &os, const RoutingVertex &vertex);

}  // namespace bfg

#endif  // ROUTING_VERTEX_H_
