#ifndef BLOCKAGE_PROJECTOR_H_
#define BLOCKAGE_PROJECTOR_H_

namespace bfg {

// This class provides a way to inspect the impact of blockages on a read-only
// RoutingGrid. It's goals are:
//  - to be usable in a multithreaded path-search;
//  - to provide fast blockage resolution;
//  - to provide fast blockage lookups; and
//  - to provide fast resolution of small blockage differences.
//
// The general approach is to memoise as much of the blockage resolution as
// possible, and then track the source of the blockages so that they can be
// removed piecemeal.
//
// Biggest benefit will be from caching hit-tests; mapping shapes to affected
// vertices and edges is costly, and it is often repeated. So for a given shape
// and padding we have to cache the affected vertices (including access
// directions) and edges. Vertices on the grid are efficiently searched because
// RoutingGridGeometry maps to their indices. Off-grid vertices are efficiently
// searched with a k-d tree.
class BlockageProjector {
 public:
  struct BlockedVertex {
    RoutingVertex *vertex;

    // Need a collection of the blockages that led to this blockage.
    // RoutingGridBlockage contains the shape that caused it.
    std::set<RoutingGridBlockage*> sources;
  };

  struct BlockedEdge {
    RoutingEdge *edge;
    std::set<RoutingGridBlockage*> sources;
  }

  BlockageProjector(
      const RoutingGrid &grid)
      : grid_(grid) {}

 private:
  const RoutingGrid &grid_;

  // A regular list of blocked vertices.
  // A regular list of blocked edges.
  // A k-d tree to track off-grid blockages.

  // A master list of all blockages we know about.

  // RoutingGridBlockage pairs a shape of either type Rectangle or Polygon with
  // some padding and tests for intersection.
};

}   // namespace bfg

#endif    // BLOCKAGE_PROJECTOR_H_
