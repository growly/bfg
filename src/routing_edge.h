#ifndef ROUTING_EDGE_H_
#define ROUTING_EDGE_H_

#include <optional>
#include <set>
#include <vector>

#include "equivalent_nets.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"
#include "physical_properties_database.h"

namespace bfg {

class RoutingTrack;
class RoutingVertex;

class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : in_use_by_net_(std::nullopt),
      temporarily_in_use_by_net_(std::nullopt),
      blocked_(false),
      temporarily_blocked_(false),
      track_(nullptr),
      layer_(std::nullopt),
      first_(first),
      second_(second),
      cost_(0.0) {
    ApproximateCost();
  }
  ~RoutingEdge() {}

  std::string Describe() const;

  bool TerminatesAt(const geometry::Point &point) const;

  bool IsBetween(const geometry::Point &lhs, const geometry::Point &rhs) const;

  void PrepareForRemoval();

  // Whether the edge is blocked, considering both permanent and temporary
  // blockages.
  bool Blocked() const;
  const std::optional<std::string> &EffectiveNet() const;
  const std::optional<std::string> &PermanentNet() const;

  std::vector<RoutingVertex*> SpannedVertices() const;

  RoutingVertex *OtherVertexThan(RoutingVertex *given) const;

  std::pair<int64_t, int64_t> ProjectOntoAxis() const;

  // Self-contained utility for imagining this edge as a rectilinear rectangle
  // of the given width.
  std::optional<geometry::Rectangle> AsRectangle(int64_t width) const;
  std::optional<geometry::Line> AsLine() const;

  void SetBlocked(bool blocked, bool temporary = false);
  void SetNet(
      const std::optional<std::string> &in_use_by_net, bool temporary = false);

  void SetPermanentlyBlocked(bool blocked) {
    SetBlocked(blocked, false);
  }
  void SetPermanentNet(
      const std::optional<std::string> &in_use_by_net) {
    SetNet(in_use_by_net, false);
  }

  bool Available() const { return !Blocked() && !EffectiveNet(); }
  bool AvailableForNets(const EquivalentNets &ok_nets) const;

  void ResetTemporaryStatus() {
    temporarily_in_use_by_net_ = std::nullopt;
    temporarily_blocked_ = false;
  }

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() const { return first_; }
  RoutingVertex *second() const { return second_; }

  void set_layer(const std::optional<geometry::Layer> &layer) {
    layer_ = layer;
  }
  const std::optional<geometry::Layer> &layer() const { return layer_; }

  const geometry::Layer EffectiveLayer() const;
  RoutingTrackDirection Direction() const;
  double Length() const;

  // Off-grid edges do not have tracks.
  void set_track(RoutingTrack *track);
  RoutingTrack *track() const { return track_; }

 private:
  void ApproximateCost();

  bool IsRectilinear() const;

  void ResetStatus() {
    in_use_by_net_ = std::nullopt;
    temporarily_in_use_by_net_ = std::nullopt;
    blocked_ = false;
    temporarily_blocked_ = false;
  }

  std::optional<std::string> in_use_by_net_;
  std::optional<std::string> temporarily_in_use_by_net_;
  bool blocked_;
  bool temporarily_blocked_;

  RoutingTrack *track_;
  std::optional<geometry::Layer> layer_;

  RoutingVertex *first_;
  RoutingVertex *second_;

  // Need some function of the distance between the two vertices (like of
  // length, sheet resistance). This also needs to be computed only once...
  double cost_;
};

std::ostream &operator<<(std::ostream &os, const RoutingEdge &edge);

}  // namespace bfg

#endif  // ROUTING_EDGE_H_
