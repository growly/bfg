#include "routing_grid_blockage.h"

#include <glog/logging.h>

#include "geometry/polygon.h"
#include "geometry/rectangle.h"
#include "routing_grid.h"
#include "routing_track.h"

namespace bfg {

template<>
bool RoutingGridBlockage<geometry::Rectangle>::IntersectsPoint(
    const geometry::Point &point, int64_t margin) const {
  return shape_.Intersects(point, margin);
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::IntersectsPoint(
    const geometry::Point &point, int64_t margin) const {
  return shape_.Intersects(point, margin);
  return false;
}

// We have a specialisation for {Rectangle, Polygon} X {Vertex, Edge}.
//
// Since these methods test for intersection, or that the two geometric objects
// overlap, we do not need to consider the case where same-net shapes are too
// close for min_separation rules (which wouldn't apply if they touched).
//
// Blockages come with a padding that we consider to be a necessary minimum
// spacing between two shapes. If the intersection occurs for padding == 0,
// i.e. the shapes touch, and we have defined exceptional nets that match for
// both shapes, then there is no blockage.
template<>
bool RoutingGridBlockage<geometry::Rectangle>::Blocks(
    const RoutingVertex &vertex,
    int64_t padding,
    const std::optional<EquivalentNets> &exceptional_nets,
    const std::optional<RoutingTrackDirection> &access_direction) const {
  // Check if there's an intersection within the default padding region:
  bool intersects = routing_grid_.ViaWouldIntersect(
      vertex, shape_, padding, access_direction);
  // If so, and if exceptional nets are defined and match, then the
  // intersection is permissible if the shapes are touching (i.e. intersection
  // with padding = 0). If we just checked that because padding == 0 already,
  // shortcut the response.
  if (intersects &&
      exceptional_nets && exceptional_nets->Contains(shape_.net())) {
    if (padding == 0) {
      return false;
    }
    return !routing_grid_.ViaWouldIntersect(
        vertex, shape_, 0, access_direction);
  }
  return intersects;
}

template<>
bool RoutingGridBlockage<geometry::Rectangle>::Blocks(
    const RoutingEdge &edge,
    int64_t padding,
    const std::optional<EquivalentNets> &exceptional_nets) const {
  bool intersects = routing_grid_.WireWouldIntersect(edge, shape_, padding); 
  if (intersects &&
      exceptional_nets && exceptional_nets->Contains(shape_.net())) {
    if (padding == 0) {
      return false;
    }
    return !routing_grid_.WireWouldIntersect(edge, shape_, 0);
  }
  return intersects;
}

template<typename T>
bool RoutingGridBlockage<T>::Blocks(
    const geometry::Rectangle &footprint,
    int64_t padding,
    const std::optional<EquivalentNets> &exceptional_nets) const {
  // T should have a .Overlaps(const geometry::Rectangle &footprint).
  bool intersects = shape_.Overlaps(
      footprint.WithPadding(std::max(padding - 1, 0L)));
  if (intersects &&
      exceptional_nets && exceptional_nets->Contains(shape_.net())) {
    if (padding == 0) {
      return false;
    }
    return !shape_.Overlaps(footprint);
  }
  return intersects;
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::Blocks(
    const RoutingVertex &vertex,
    int64_t padding,
    const std::optional<EquivalentNets> &exceptional_nets,
    const std::optional<RoutingTrackDirection> &access_direction) const {
  bool intersects = routing_grid_.ViaWouldIntersect(
      vertex, shape_, padding, access_direction);
  if (intersects &&
      exceptional_nets &&
      exceptional_nets->Contains(shape_.net())) {
    if (padding == 0) {
      return false;
    }
    return !routing_grid_.ViaWouldIntersect(
      vertex, shape_, 0, access_direction);
  }
  return intersects;
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::Blocks(
    const RoutingEdge &edge,
    int64_t padding,
    const std::optional<EquivalentNets> &exceptional_nets) const {
  bool intersects = routing_grid_.WireWouldIntersect(edge, shape_, padding); 
  if (intersects &&
      exceptional_nets &&
      exceptional_nets->Contains(shape_.net())) {
    if (padding == 0) {
      return false;
    }
    return !routing_grid_.WireWouldIntersect(edge, shape_, 0);
  }
  return intersects;
}

template<typename T>
void RoutingGridBlockage<T>::AddChildTrackBlockage(
    RoutingTrack *track, RoutingTrackBlockage *blockage) {
  child_track_blockages_.emplace_back(
      track, 
      std::unique_ptr<RoutingTrackBlockage>(blockage));
}

template<typename T>
void RoutingGridBlockage<T>::ClearChildTrackBlockages() {
  for (auto &entry : child_track_blockages_) {
    RoutingTrack *track = entry.first;
    RoutingTrackBlockage *blockage = entry.second.get();
    // NOTE(aryap): It is conceivable that RoutingGridBlockage would want to
    // store 'child' blockages which aren't temporary, but this is not the
    // case today.
    track->RemoveTemporaryBlockage(blockage);
  }
}

template<typename T>
RoutingGridBlockage<T>::~RoutingGridBlockage() {
  ClearChildTrackBlockages();
  // Objects destroyed.
}

// Explicit instantiation for template classes. Google used to mandate that
// template definitions like the those in the bulk of this file go into an
// -inl.h header, but that is no longer true. Putting the definitions in the .h
// file means using classes that were otherwise forward-declared (like
// RoutingGrid), which results in a circular header dependency mess. Using the
// -inl.h files would've solved this, mostly. But anyway now we do it this way:
template class RoutingGridBlockage<geometry::Rectangle>;
template class RoutingGridBlockage<geometry::Polygon>;

}  // namespace bfg
