#ifndef ROUTING_VERTEX_KD_TREE_H_
#define ROUTING_VERTEX_KD_TREE_H_

#include <kdtree++/kdtree.hpp>
#include "routing_vertex.h"
#include "geometry/point.h"

namespace bfg {

// A key for something like a KD tree.
class RoutingVertexKDNode {
 public:
  typedef int64_t value_type;

  // Convert an L2 distance (euclidean distance) to the internal distance
  // measure.
  static value_type L2DistanceToInternal(int64_t l2_distance) {
    return l2_distance * l2_distance;
  }

  RoutingVertexKDNode(RoutingVertex *vertex) : vertex_(vertex) {}

  inline value_type operator[](const size_t n) const {
    switch (n) {
      case 0:
        return vertex_->centre().x();
      case 1:
        return vertex_->centre().y();
      default:
        // For any higher dimensions, use the pointer itself.
        return reinterpret_cast<value_type>(vertex_);
    }
  }

  // KDTree will use this function to check distance.
  inline value_type distance(const RoutingVertexKDNode &other) const {
    // L2 squared distance saves a sqrt.
    // L1 (Manhattan) distance would be even faster.
    // Unfortunately neither of these is intuitive to use. If speed is
    // imperative the implications for the radius value in FindNearby(...)
    // should be clearly communicated.
    const geometry::Point &lhs = vertex_->centre();
    const geometry::Point &rhs = other.vertex_->centre();
    return lhs.L2DistanceTo(rhs);
  }

  RoutingVertex *vertex() const { return vertex_; }

 private:
  RoutingVertex *vertex_;
};

// Manages a collection of RoutingVertex pointers (we do not take ownership!)
// using a KDTree. Wraps KDTree search functions to automatically batch calls
// to KDTree::optimise.
//
// The appropriate usage of KDTree::optimise() is described on the library
// GitHub page here:
//    <I can't find it>
class RoutingVertexKDTree {
 public:
  RoutingVertexKDTree()
      : needs_optimise_(false) {}

  void Add(RoutingVertex *vertex);
  void Erase(RoutingVertex *vertex);

  std::vector<RoutingVertex*> FindNearby(
      const geometry::Point &reference, int64_t radius) const;

  size_t Size() const {
    return tree_.size();
  }

 private:
  inline void Optimise() const;

  // These are only 'mutable' by Optimise(), which needs to call
  // KDTree::optimise(). Otherwise it is a bit stupid that the only two data
  // members are mutable.
  mutable bool needs_optimise_;
  mutable KDTree::KDTree<2, RoutingVertexKDNode> tree_;
};

}  // namespace bfg

#endif  // ROUTING_VERTEX_KD_TREE_H_
