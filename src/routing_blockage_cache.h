#ifndef ROUTING_BLOCKAGE_CACHE_H_
#define ROUTING_BLOCKAGE_CACHE_H_

#include <map>
#include <set>
#include <optional>

#include "routing_vertex.h"
#include "routing_track_direction.h"
#include "routing_grid_blockage.h"
#include "geometry/rectangle.h"
#include "geometry/polygon.h"

namespace bfg {

class RoutingGrid;

// This class provides a way to inspect the impact of blockages on a read-only
// RoutingGrid. It's goals are:
//  - to be usable in a multithreaded path-search;
//  - to provide fast blockage resolution;
//  - to provide fast blockage lookups; and
//  - to provide fast resolution of small blockage differences between
//  instances.
//
// The general approach is to memoise as much of the blockage resolution as
// possible, and then track the source of the blockages so that they can be
// removed piecemeal.
//
// The biggest benefit will be from caching hit-tests; mapping shapes to
// affected vertices and edges is costly, and it is often repeated. So for a
// given shape and padding we have to cache the affected vertices (including
// access directions) and edges. Vertices on the grid are efficiently searched
// because RoutingGridGeometry maps to their indices. Off-grid vertices are
// efficiently searched with a k-d tree.
//
// A typical use case for this class is that, when routing many nets, a single
// set of ports exists in the design and must be tested for blockage. However,
// in a given path search some subset of those ports are considered exceptions
// that should not cause blockages (these are usually the ports on the nets
// being routed). We store the blockages first, and the exceptions in child
// instances, which can be owned by each thread.
//
// The source of any blockage is a Shape, and the identity of that shape is its
// position in memory. All of the shapes referenced by these classes must be
// owned externally, typically by the caller of all RoutingGrid methods.
//
// The principal question this class shall answer is: is this thing blocked?
class RoutingBlockageCache {
 public:
  // This is similar to the identically-named function in RoutingGrid.
  //
  // Works for T = geometry::Group and T = geomtery::ShapeCollection, since the
  // code needed to use the interface looks the same. The interface is not
  // type-compatible because ShapeCollection contains std::unique_ptr and Group
  // contains straight pointers. It would be nicer to simply provide converting
  // accessors and use a standard interface. As is this is kind of gross, sorry.
  //
  // Be careful when adding connectable shapes (i.e. pins) as "blockages"!
  template<typename T>
  void AddBlockages(const T &shapes, int64_t padding = 0) {
    for (const auto &rectangle : shapes.rectangles()) {
      AddBlockage(*rectangle, padding);
    }
    for (const auto &polygon : shapes.polygons()) {
      AddBlockage(*polygon, padding);
    }
    for (const auto &port : shapes.ports()) {
      // Ports are Rectangles!
      AddBlockage(*port, padding);
    }
  }

  void AddBlockage(const geometry::Rectangle &rectangle,
                   int64_t padding);

  void AddBlockage(const geometry::Polygon &polygon,
                   int64_t padding);

  RoutingBlockageCache(const RoutingGrid &grid);

  RoutingBlockageCache(const RoutingGrid &grid,
                       const RoutingBlockageCache &parent)
      : grid_(grid),
        parent_(parent),
        search_window_margin_(0) {}

  bool IsEdgeBlocked(
      const RoutingEdge &edge,
      const EquivalentNets &for_nets) const;

  bool IsVertexBlocked(
      const RoutingVertex &vertex,
      const EquivalentNets &for_nets,
      const std::optional<RoutingTrackDirection> &direction_or_any,
      const std::optional<geometry::Layer> &layer_or_any) const;

  void set_search_window_margin(int64_t search_window_margin) {
    search_window_margin_ = search_window_margin;
  };

  int64_t search_window_margin() const {
    return search_window_margin_;
  }

 private:
  typedef std::variant<
      const RoutingGridBlockage<geometry::Rectangle>*,
      const RoutingGridBlockage<geometry::Polygon>*> SourceBlockage;

  // Vertices are blocked in the following interesting ways:
  //   - blockages on nets that are near enough to prevent a via being placed at
  //   the vertex, but might allow the vertex to be used to reach that net;
  //   - blockages that prevent the vertex for being used for anything but a
  //   given net (usually because they intersect).
  //
  // (This is also captured in RoutingVertex itself.)
  class VertexBlockages {
   public:
    template<typename T>
    void AddUser(const T& blockage) {
      const std::string &net = blockage.shape().net();
      SourceBlockage container = &blockage;
      const geometry::Layer &layer = blockage.shape().layer();
      users_[net][layer].insert(container);
    }

    template<typename T>
    void AddInhibitor(const RoutingTrackDirection &blocked_direction,
                      const T &blockage) {
      SourceBlockage container = &blockage;
      const geometry::Layer &layer = blockage.shape().layer();
      inhibitors_[blocked_direction][layer].insert(container);
    }

    // Any specified parameters narrow the space for checked blockages, so if
    // exceptional_nets is empty and no layer is specified, ANY blockage counts
    // return true.
    bool IsBlockedByUsers(
        const EquivalentNets &exceptional_nets,
        const std::optional<geometry::Layer> &layer_or_any) const;

    // Similarly, if a parameter is std::nullopt, any direction (respectively
    // layer) is checked for a blockage, and any of those will cause this to
    // return true.
    bool IsInhibitedInDirection(
        const std::optional<RoutingTrackDirection> &direction_or_any,
        const std::optional<geometry::Layer> &layer_or_any) const;

    // An "inhibitor" entry indicates the direction and layer in which a vertex
    // cannot accomodate a via, and set of blockages which cause this.
    const std::map<RoutingTrackDirection,
        std::map<geometry::Layer, std::set<SourceBlockage>>> &inhibitors()
        const {
      return inhibitors_;
    }

    // A "user" entry tracks sets of blockages that intersect with a vertex on a
    // given (net, layer) pair.
    const std::map<std::string,
        std::map<geometry::Layer, std::set<SourceBlockage>>> &users() const {
      return users_;
    }

   private:
    // Maps the direction which is disallowed to the set of blockages causing
    // the inhibition.
    std::map<RoutingTrackDirection,
        std::map<geometry::Layer, std::set<SourceBlockage>>> inhibitors_;

    // Overlapping blockages and their nets.
    std::map<std::string,
        std::map<geometry::Layer, std::set<SourceBlockage>>> users_;
  };

  struct EdgeBlockages {
    // If a single blockage with a blocks the edge, the edge can act as a
    // connector to that blockage and inherits the net itself. Otherwise, it is
    // not usable. All the blockages without a net ("") will end up under the
    // same entry.
    std::map<std::string, std::set<SourceBlockage>> sources;
  };

  bool IsEdgeBlocked(
      const RoutingEdge &edge,
      const EquivalentNets &for_nets,
      const std::optional<std::vector<SourceBlockage>>
          &more_cancelled_blockages_) const;

  template<typename T>
  void ApplyBlockageToOneVertex(
      const RoutingGridBlockage<T> &blockage,
      const RoutingVertex *vertex,
      std::optional<RoutingTrackDirection> access_direction = std::nullopt);

  std::vector<const RoutingVertex*> DeterminePossiblyAffectedVertices(
      const geometry::Rectangle &rectangle,
      int64_t padding) const;

  std::vector<const RoutingVertex*> DeterminePossiblyAffectedVertices(
      const geometry::Polygon &polygon,
      int64_t padding) const {
    geometry::Rectangle bounding_box = polygon.GetBoundingBox();
    return DeterminePossiblyAffectedVertices(bounding_box, padding);
  }

  template<typename T>
  std::vector<const RoutingEdge*> DetermineAffectedOnGridEdges(
      const T &shape, int64_t padding) const;

  template<typename T>
  std::vector<const RoutingEdge*> DetermineAffectedEdges(
      const T &rectangle,
      int64_t padding) const;

  const RoutingGrid &grid_;

  // To speed things up we will limit the vertices we check for blockages to
  // those within this margin of any blockage + padding:
  int64_t search_window_margin_;

  // If available, queries are forwarded to a parent RoutingBlockageCache.
  std::optional<std::reference_wrapper<const RoutingBlockageCache>> parent_;

  // A regular list of blocked vertices.
  std::map<const RoutingVertex*, VertexBlockages> blocked_vertices_;

  // A regular list of blocked edges.
  std::map<const RoutingEdge*, EdgeBlockages> blocked_edges_;

  // Cancelled blockages should be treated as non-existent.
  // FIXME(aryap): How to identify shapes?
  std::vector<SourceBlockage> cancelled_blockages_;

  // A master list of all blockages we know about.
  //
  // RoutingGridBlockage pairs a shape of either type Rectangle or Polygon with
  // some padding and tests for intersection.
  std::vector<std::unique_ptr<RoutingGridBlockage<geometry::Polygon>>>
      polygon_blockages_;
  std::vector<std::unique_ptr<RoutingGridBlockage<geometry::Rectangle>>>
      rectangle_blockages_;
};

}   // namespace bfg

#endif  // ROUTING_BLOCKAGE_CACHE_H_
