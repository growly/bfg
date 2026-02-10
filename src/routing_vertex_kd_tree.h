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
//
// So I think libkdtree++ k-d trees work only with manhattan distance (the L1
// norm), even though I initially thought I could define my own distance
// function.
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
