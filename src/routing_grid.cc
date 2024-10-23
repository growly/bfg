#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <utility>
#include <vector>

#include <absl/cleanup/cleanup.h>
#include <absl/status/status.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "equivalent_nets.h"
#include "geometry/compass.h"
#include "geometry/poly_line.h"
#include "geometry/rectangle.h"
#include "physical_properties_database.h"
#include "poly_line_cell.h"
#include "poly_line_inflator.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_path.h"
#include "routing_track.h"
#include "routing_track_blockage.h"
#include "routing_vertex.h"
#include "routing_via_info.h"

// TODO(aryap):
//  1) What does RoutingGrid::available_vertices_by_layer_ actually do?

// A RoutingGrid manages a multi-layer rectilinear wire grid for for
// connections between points.
//
// The grid comprises RoutingTracks, which own a set of RoutingVertexs and
// RoutingEdges. It also owns a separate collection of RoutingVertexs and
// RoutingEdges that do not fall onto specific tracks.
//
// NOTE(aryap): An optimisation for the router that prefers single-layer
// routing: maintaining the strict vertical/horizontal constraint on layers
// tends to create sub-optimal routes in congested areas because changing layers
// incurs forces other paths to go way around the long way. It might be better
// to assume single-layer routing, and then as a post-processing step find all
// intersections of used edges and elevate one of the conflicting edges to a
// different layer. But even if we assume that vertices must be able to
// accomodate a via at all times the post-processing step might backtrack us
// into an unroutable state.

using bfg::geometry::Compass;

namespace bfg {

// These partial specialisations need to occur before any use because dem's the
// rulz.

template<>
bool RoutingGridBlockage<geometry::Rectangle>::IntersectsPoint(
    const geometry::Point &point, int64_t margin) const {
  VLOG(20) << "Unimplemented!";
  return false;
}

template<>
bool RoutingGridBlockage<geometry::Polygon>::IntersectsPoint(
    const geometry::Point &point, int64_t margin) const {
  VLOG(20) << "Unimplemented!";
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
bool RoutingGridBlockage<geometry::Rectangle>::Blocks(
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

template<typename T>
void RoutingGrid::ApplyBlockage(
    const RoutingGridBlockage<T> &blockage,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices) {
  const geometry::Layer &layer = blockage.shape().layer();
  // Find any possibly-blocked vertices and make them unavailable:
  std::vector<RoutingGridGeometry*> grid_geometries =
      FindRoutingGridGeometriesUsingLayer(layer);
  for (RoutingGridGeometry *grid_geometry : grid_geometries) {
    std::set<RoutingVertex*> vertices;
    grid_geometry->EnvelopingVertices(blockage.shape(), &vertices);
    for (RoutingVertex *vertex : vertices) {
      if (!vertex->available())
        continue;

      std::set<RoutingTrackDirection> access_directions = {
          RoutingTrackDirection::kTrackHorizontal,
          RoutingTrackDirection::kTrackVertical};

      const std::string &net = blockage.shape().net();
      bool any_access = false;
      for (const auto &direction : access_directions) {
        bool blocked_at_all = false;
        // We use the RoutingGridBlockage to do a hit test; set
        // exceptional_nets = nullopt so that no exception is made.
        if (blockage.IntersectsPoint(vertex->centre(), 0)) {
          vertex->set_net(blockage.shape().net());
          VLOG(16) << "Blockage: " << blockage.shape()
                   << " intersects " << vertex->centre()
                   << " with margin " << 0
                   << direction << " direction";
        } else if (blockage.Blocks(*vertex, std::nullopt, direction)) {
          blocked_at_all = true;
          if (net != "") {
            vertex->set_connectable_net(net);
          }
          VLOG(16) << "Blockage: " << blockage.shape()
                   << " blocks " << vertex->centre()
                   << " with padding=" << blockage.padding() << " in "
                   << direction << " direction";
        }
        any_access = !blocked_at_all || any_access;
      }

      if (!any_access) {
        vertex->set_available(false);
        if (blocked_vertices) {
          blocked_vertices->insert(vertex);
        }
      }
    }

    // TODO(aryap): Do we need a facility to roll back off-grid vertices for
    // shapes on nets that are temporary blockages? Practically this includes
    // via footprints for ports!
    if (!is_temporary && !blockage.shape().net().empty()) {
      AddOffGridVerticesForBlockage(*grid_geometry, blockage);
    }
  }
}

template<>
void RoutingGrid::ForgetBlockage(
    RoutingGridBlockage<geometry::Rectangle> *blockage) {
  auto predicate = [&](
      const std::unique_ptr<RoutingGridBlockage<geometry::Rectangle>> &entry) {
      return entry.get() == blockage;
  };
  auto it = std::find_if(
      rectangle_blockages_.begin(), rectangle_blockages_.end(), predicate);
  if (it == rectangle_blockages_.end())
    return;
  rectangle_blockages_.erase(it);
}

template<>
void RoutingGrid::ForgetBlockage(
    RoutingGridBlockage<geometry::Polygon> *blockage) {
  auto predicate = [&](
      const std::unique_ptr<RoutingGridBlockage<geometry::Polygon>> &entry) {
      return entry.get() == blockage;
  };
  auto it = std::find_if(
      polygon_blockages_.begin(), polygon_blockages_.end(), predicate);
  if (it == polygon_blockages_.end())
    return;
  polygon_blockages_.erase(it);
}

// We rely on the RoutingGridBlockage to generate candidate positions and
// because it can efficiently determine which tracks the polygon intersects,
// since it can relate the bounding box of the given object to the
// possibly-implicated tracks.
template<typename T>
void RoutingGrid::AddOffGridVerticesForBlockage(
    const RoutingGridGeometry &grid_geometry,
    const RoutingGridBlockage<T> &blockage) {
  auto tracks_and_positions =
      grid_geometry.CandidateVertexPositionsOnCrossedTracks(
          blockage.shape());
  
  for (auto entry : tracks_and_positions) {
    RoutingTrack *track = entry.first;
    const geometry::Layer &other_layer =
        track->layer() == grid_geometry.horizontal_layer() ?
            grid_geometry.vertical_layer() : grid_geometry.horizontal_layer();

    for (const geometry::Point &point : entry.second) {
      RoutingVertex *new_vertex =
          track->CreateNewVertexAndConnect(*this,
                                           point,
                                           other_layer,
                                           blockage.shape().net());
      if (!new_vertex) {
        continue;
      }
      new_vertex->set_available(false);
      new_vertex->set_net(blockage.shape().net());
      new_vertex->set_explicit_net_layer(blockage.shape().layer());
      // TODO(aryap): This actually requires a test on the blockage shape
      // accommodating the encap rules as-is, which we could do, but which would
      // require me to be less lazy.
      new_vertex->set_explicit_net_layer_requires_encap(true);
      AddVertex(new_vertex);
    }
  }
}

absl::Status RoutingGrid::ValidAgainstKnownBlockages(
    const RoutingEdge &edge,
    const std::optional<EquivalentNets> &exceptional_nets) const {
  // *snicker* Cute opportunity for std::any_of here:
  for (const auto &blockage : rectangle_blockages_) {
    if (blockage->Blocks(edge, exceptional_nets)) {
      return absl::UnavailableError(
          absl::StrCat("Blocked by ", blockage->shape().Describe()));
    }
  }
  for (const auto &blockage : polygon_blockages_) {
    if (blockage->Blocks(edge, exceptional_nets)) {
      return absl::UnavailableError(
          absl::StrCat("Blocked by ", blockage->shape().Describe()));
    }
  }
  return absl::OkStatus();
}

absl::Status RoutingGrid::ValidAgainstKnownBlockages(
    const RoutingVertex &vertex,
    const std::optional<EquivalentNets> &exceptional_nets,
    const std::optional<RoutingTrackDirection> &access_direction) const {
  // *snicker* Cute opportunity for std::any_of here:
  for (const auto &blockage : rectangle_blockages_) {
    if (blockage->Blocks(vertex, exceptional_nets, access_direction)) {
      return absl::UnavailableError(
          absl::StrCat("Blocked by ", blockage->shape().Describe()));
    }
  }
  for (const auto &blockage : polygon_blockages_) {
    if (blockage->Blocks(vertex, exceptional_nets, access_direction)) {
      return absl::UnavailableError(
          absl::StrCat("Blocked by ", blockage->shape().Describe()));
    }
  }
  return absl::OkStatus();
}

absl::Status RoutingGrid::ValidAgainstInstalledPaths(
    const RoutingEdge &edge,
    const std::optional<EquivalentNets> &for_nets) const {
  auto edge_footprint = EdgeWireFootprint(edge);
  if (!edge_footprint) {
    // No way to check.
    return absl::UnavailableError(
        "Could not get footprint of edge to check its validity");
  }
  return ValidAgainstInstalledPaths(*edge_footprint, for_nets);
}

absl::Status RoutingGrid::ValidAgainstInstalledPaths(
    const RoutingVertex &vertex,
    const std::optional<EquivalentNets> &for_nets,
    const std::optional<RoutingTrackDirection> &access_direction) const {
  // In this case we have to do labourious check for proximity to all used paths
  // and vertices.
  std::vector<std::string> errors;
  for (const geometry::Layer &candidate_layer : vertex.connected_layers()) {
    std::optional<geometry::Rectangle> via_encap_footprint = ViaFootprint(
        vertex, candidate_layer, 0, access_direction);
    if (!via_encap_footprint) {
      // ViaFootprint will return nullopt if there is no other layer to connect
      // to at the given vertex, which happens if the vertex represents a
      // connection on the same layer only. This is not a problem, and there is
      // no footprint to measure here, since there is no via to cover.
      continue;
    }
    auto valid = ValidAgainstInstalledPaths(*via_encap_footprint, for_nets);
    if (!valid.ok()) {
      errors.push_back(
          std::string(valid.message().begin(),
                      valid.message().end()));
    }
  }
  if (errors.empty()) {
    return absl::OkStatus();
  }
  std::string error_str = absl::StrJoin(errors, "; ");
  LOG(INFO) << error_str;
  return absl::UnavailableError(error_str);
}

absl::Status RoutingGrid::ValidAgainstInstalledPaths(
    const geometry::Rectangle &footprint,
    const std::optional<EquivalentNets> &for_nets) const {
  const geometry::Layer &footprint_layer = footprint.layer();
  // In this case we have to do labourious check for proximity to all used paths
  // and vertices.
  std::set<const RoutingEdge*> used_edges;
  std::vector<std::pair<const RoutingVertex*, const RoutingTrackDirection>>
      used_vertices_and_directions;
  for (const RoutingPath *path : paths_) {
    for (const RoutingEdge *edge : path->edges()) {
      if (edge->EffectiveLayer() != footprint_layer) {
        continue;
      }
      used_edges.insert(edge);
      const RoutingTrackDirection &direction = edge->Direction();
      used_vertices_and_directions.push_back(
          {edge->first(), direction});
      used_vertices_and_directions.push_back(
          {edge->second(), direction});
    }
  }

  // TODO(aryap): We have fragmented sources for this information. Some
  // places I've used the PhysicalPropertiesDatabase, others the copies of
  // the data in the RoutingLayerInfo etc structures. Gross!
  //
  // Also, the RoutingGrid needs to be aware of some details of layers not
  // explicitly used for routing, but used for connection and via checking. For
  // example, vertices which connect to li1.drawing, beneath the grid, are not
  // in error just because we can't find connectivity info for them.
  // auto routing_layer_info = GetRoutingLayerInfo(footprint_layer);
  // if (!routing_layer_info) {
  //   return absl::NotFoundError(
  //       absl::StrCat("No routing layer info for footprint layer ",
  //                    footprint_layer));
  // }
  // int64_t min_separation = routing_layer_info->get().min_separation();
  int64_t min_separation = physical_db_.Rules(footprint_layer).min_separation;

  // Check proximity to all installed edges:
  for (const RoutingEdge *used : used_edges) {
    if (used->EffectiveLayer() != footprint_layer)
      continue;
    auto existing_footprint = EdgeWireFootprint(*used);
    if (!existing_footprint)
      continue;
    int64_t distance = static_cast<int64_t>(
        std::ceil(existing_footprint->ClosestDistanceTo(footprint)));
    if (distance == 0 && for_nets &&
        used->EffectiveNet() &&
        for_nets->Contains(*used->EffectiveNet())) {
      // Touching footprints are ok if they share the same net. Footprints which
      // share the same net but which do not touch, and instead violate
      // min_separation, are not ok.
      continue;
    } else if (distance < min_separation) {
      return absl::UnavailableError(
          absl::StrCat("Footprint is too close to existing edge: ",
                       footprint.Describe(),
                       " to ",
                       used->Describe()));
    }
  }

  for (const auto &pair : used_vertices_and_directions) {
    const RoutingVertex *const other = pair.first;
    const RoutingTrackDirection &access_direction = pair.second;

    // Get the other vertices' footprints on the layer footprint layer we're
    // dealing with, skipping if they don't have one.
    std::optional<geometry::Rectangle> other_via_encap = ViaFootprint(
       *other, footprint_layer, 0, access_direction);
    if (!other_via_encap) {
      // An empty footprint indicates that the via doesn't connect to a layer
      // other than 'footprint_layer'.
      continue;
    }

    int64_t distance = static_cast<int64_t>(
        std::ceil(footprint.ClosestDistanceTo(*other_via_encap)));
    if (distance == 0 && for_nets &&
        other->connectable_net() &&
        for_nets->Contains(*other->connectable_net())) {
      // The shapes touch and they're on the same net, so no problem.
      // NOTE(aryap): This is the same as checking
      // via_encap->Overlaps(*other_via_encap).
      continue;
    } else if (distance < min_separation) {
      std::stringstream ss;
      ss << "Footprint " << footprint 
         << " is too close to " << other->centre() << " on layer "
         << footprint_layer << " (distance " << distance <<
         " < min separation " << min_separation << ")";
      VLOG(12) << ss.str();
      return absl::UnavailableError(ss.str());
    } else if (VLOG_IS_ON(16)) {
      VLOG(13) << "Footprint " << footprint
               << " is ok with " << other->centre() << " on layer "
               << footprint_layer << " (distance " << distance
               << " >= min separation " << min_separation << ")";
    }
  }
  return absl::OkStatus();
}

RoutingGrid::~RoutingGrid()  {
  // NOTE(aryap): The problem with doing this in ~RoutingGrid() explicitly is
  // that we can no longer rely on the ordered unwind of RoutingGrid's fields
  // to delete objects which depend on these ones first. In particular,
  // RoutingGridBlockages can depend on RoutingTracks, so we have to make sure
  // we delete the RoutingGridBlockages first.
  //
  // The reason we have to do it explicitly is because we're using raw
  // pointers, that we own. Oof.
  ClearAllBlockages();

  for (auto entry : tracks_by_layer_) {
    for (RoutingTrack *track : entry.second) {
      delete track;
    }
  }
  for (RoutingPath *path : paths_) { delete path; }
  for (RoutingEdge *edge : off_grid_edges_) { delete edge; }
  for (RoutingVertex *vertex : vertices_) { delete vertex; }
}

// Return the (horizontal, vertical) routing infos.
// TODO(aryap): UGH using make_pair makes a copy of the references?
// I have to use std::reference_wrapper<const RoutingLayerInfo>?
// Why don't I just use pointers, exactly?
//    UGH compare this to the last signature:
//
//  std::pair<RoutingLayerInfo*, RoutingLayerInfo*>
//      RoutingGrid::PickHorizontalAndVertical(
//          const Layer &lhs, const Layer &rhs) const;
//
// And I could even use std::make_pair because it wasn't so smart:
//  https://stackoverflow.com/questions/53483124/reference-wrapper-make-pair-vs-class-template-argument-deduction-ctad
std::pair<std::reference_wrapper<const RoutingLayerInfo>,
          std::reference_wrapper<const RoutingLayerInfo>>
    RoutingGrid::PickHorizontalAndVertical(
        const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  const RoutingLayerInfo &lhs_info = GetRoutingLayerInfoOrDie(lhs);
  const RoutingLayerInfo &rhs_info = GetRoutingLayerInfoOrDie(rhs);
  if (lhs_info.direction() == RoutingTrackDirection::kTrackHorizontal &&
      rhs_info.direction() == RoutingTrackDirection::kTrackVertical) {
    return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
        lhs_info, rhs_info);
  } else if (lhs_info.direction() == RoutingTrackDirection::kTrackVertical &&
             rhs_info.direction() == RoutingTrackDirection::kTrackHorizontal) {
    return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
        rhs_info, lhs_info);
  }
  LOG(FATAL) << "Exactly one of each layer must be horizontal and one must be"
             << "vertical: " << lhs << ", " << rhs;
  return std::pair<const RoutingLayerInfo&, const RoutingLayerInfo&>(
      lhs_info, rhs_info);
}

absl::StatusOr<RoutingGrid::VertexWithLayer> RoutingGrid::ConnectToGrid(
    const geometry::Port &port,
    const EquivalentNets &connectable_nets) {
  auto try_add_access_vertices = AddAccessVerticesForPoint(
      port.centre(), port.layer(), connectable_nets);
  if (try_add_access_vertices.ok()) {
    return *try_add_access_vertices;
  }
  // Fall back to slower, possibly broken method.
  auto try_nearest_available =
      ConnectToNearestAvailableVertex(port, connectable_nets);
  if (try_nearest_available.ok()) {
    return *try_nearest_available;
  }

  std::stringstream ss;
  ss << "Could not connect to grid: "
     << "(1) " << try_add_access_vertices.status().message()
     << "; (2) " << try_nearest_available.status().message();

  return absl::NotFoundError(ss.str());
}

// Using the given RoutingGridGeometry, find the tracks which surround
// *off_grid and attempt to create vertices on each of those tracks for edges
// from *off_grid to land on.
//
// NOTE(aryap): Does not rollback changes to *off_grid on error, so assume the
// caller will just give up on the object and delete it from the grid.
absl::Status RoutingGrid::ConnectToSurroundingTracks(
    const RoutingGridGeometry &grid_geometry,
    const geometry::Layer &access_layer,
    const EquivalentNets &connectable_nets,
    const std::optional<
        std::reference_wrapper<
            const std::set<RoutingTrackDirection>>> &directions,
    RoutingVertex *off_grid) {
  // Number of layers of tracks to connect to, outwards, from the given off-grid
  // vertex.
  static int64_t kTrackRadius = 2;

  std::set<RoutingTrack*> nearest_tracks;
  grid_geometry.NearestTracks(
      off_grid->centre(), &nearest_tracks, &nearest_tracks, kTrackRadius);

  auto ok_access_direction = [&](const RoutingTrackDirection &candidate) {
    if (directions) {
      const auto &access_directions = directions->get();
      return access_directions.find(candidate) != access_directions.end();
    }
    return true;
  };

  std::vector<std::string> errors;
  std::vector<RoutingEdge*> new_edges;

  bool any_success = false;
  for (RoutingTrack *track : nearest_tracks) {
    RoutingVertex *bridging_vertex = nullptr;
    bool bridging_vertex_is_new = false;
    bool off_grid_already_exists = false;
    bool success = track->CreateNearestVertexAndConnect(
        *this,
        off_grid,
        access_layer,
        connectable_nets,
        &bridging_vertex,
        &bridging_vertex_is_new,
        &off_grid_already_exists);

    if (!success) {
      continue;
    } else if (bridging_vertex == off_grid) {

      if (bridging_vertex_is_new) {
        errors.push_back(
            "Doesn't make sense for bridging_vertex == target "
            "and bridging_vertex_is_new to both be true");
        continue;
      }

      // Since our off_grid vertex has landed on the track, the access direction
      // to the off_grid point is just the track direction. If this is not an
      // allowable direction, give up now:
      if (!ok_access_direction(track->direction())) {
        continue;
      }

      any_success = true || any_success;
      continue;
    }

    if (off_grid_already_exists) {
      std::stringstream ss;
      ss << *track << " already has a vertex at the position of off_grid "
         << off_grid->centre();
      errors.push_back(ss.str());
      continue;
    }

    if (bridging_vertex_is_new) {
      // Need to check if this new vertex is valid against all known blockages:
      auto validity = ValidAgainstKnownBlockages(
          *bridging_vertex, connectable_nets, track->direction());
      if (!validity.ok()) {
        track->RemoveVertex(bridging_vertex);
        delete bridging_vertex;
        continue;
      }

      AddVertex(bridging_vertex);
    }

    // At this point, the bridging vertex needs to be connected to off_grid.
    // Any condition that precludes that should have been handled already.

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid);
    edge->set_layer(access_layer);

    // Since we're creating a jog edge to connect to the off_grid point, we have
    // to check that its direction is permissible as an access direction.
    if (!ok_access_direction(edge->Direction())) {
      delete edge;
      continue;
    }

    // We do not check ValidAgainstInstalledPaths because that is slow. We hope
    // that by now the other rules have prevented such a possibility. Fingers
    // crossed....
    if (!ValidAgainstKnownBlockages(*edge, connectable_nets).ok()) {
      VLOG(100) << "Invalid off grid edge between "
                << bridging_vertex->centre()
                << " and " << off_grid->centre();
      // Rollback extra hard!
      RemoveVertex(bridging_vertex, true);  // and delete!
      delete edge;
      continue;
    }

    bridging_vertex->AddEdge(edge);
    off_grid->AddEdge(edge);
    new_edges.push_back(edge);
    any_success = true || any_success;
  }

  for (RoutingEdge *edge : new_edges) {
    AddOffGridEdge(edge);
  }

  std::string message = absl::StrJoin(errors, "; ");
  return any_success ?
      absl::Status(absl::StatusCode::kOk, message) :
      absl::NotFoundError(message);
}

absl::StatusOr<RoutingGrid::VertexWithLayer>
RoutingGrid::AddAccessVerticesForPoint(const geometry::Point &point,
                                       const geometry::Layer &layer,
                                       const EquivalentNets &for_nets) {
  // Add each of the possible on-grid access vertices for a given off-grid
  // point to the RoutingGrid. For example, given an arbitrary point O, we must
  // find the four nearest on-grid points A, B, C, D:
  //
  //        (A)
  //     X   +       X           X
  //      (2)| <-(1)
  //  (B)+---O-------+
  //         |  ^   (D)
  //         |  (4)
  //      (3)|
  //     X   +       X           X
  //        (C)
  //
  // If O lands on a grid column and/or row, we do not need to find a bridging
  // vertex on that column/row.
  //
  // For each vertex (A, B, C, D) we create, we also have to add a bridging edge
  // to the off-grid vertex (1, 2, 3, 4, respectively).
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      layer_access = physical_db_.FindReachableLayersByPinLayer(layer);

  LOG_IF(WARNING, layer_access.empty())
      << "Pin layer access was empty; is this a pin layer? " << layer;
  if (layer_access.empty()) {
    // If the given layer does not provide access to other layers, use the layer
    // itself.
    layer_access.push_back({layer, {layer}});

    // TODO(aryap): More generally use any layer that we can reach with a via?
    //layer_access = physical_db_.FindLayersReachableThroughOneViaFrom(layer);
  }

  struct AccessOption {
    RoutingGridGeometry *grid_geometry;
    geometry::Layer target_layer;
    geometry::Layer access_layer;
    double total_via_cost;
  };

  std::vector<AccessOption> access_options;

  // Find usable RoutingGridGeometries (grids):
  for (const auto &entry : layer_access) {
    const geometry::Layer &target_layer = entry.first;
    for (const geometry::Layer &access_layer : entry.second) {
      std::vector<RoutingGridGeometry*> layer_grid_geometries =
          FindRoutingGridGeometriesUsingLayer(access_layer);

      auto cost = FindViaStackCost(target_layer, access_layer);
      if (!cost) {
        // No via stack.
        VLOG(15) << "Cannot connect layers (" << target_layer
                 << ", " << access_layer << ")";
        continue;
      }

      for (RoutingGridGeometry *grid_geometry : layer_grid_geometries) {
        access_options.push_back(AccessOption {
            .grid_geometry = grid_geometry,
            .target_layer = target_layer,
            .access_layer = access_layer,
            .total_via_cost = *cost});
      }
    }
  }

  auto comparator = [](const AccessOption &lhs, const AccessOption &rhs) {
    return lhs.total_via_cost < rhs.total_via_cost;
  };
  std::sort(access_options.begin(), access_options.end(), comparator);

  // Now that our options are sorted by the via cost they would incur, iterate
  // in increasing cost order until one of the options can accommodate the
  // target point.
  for (AccessOption &option : access_options) {
    const geometry::Layer &target_layer = option.target_layer;
    const geometry::Layer &access_layer = option.access_layer;
    RoutingGridGeometry *grid_geometry = option.grid_geometry;

    LOG(INFO) << "Access to " << point << " (layer " << target_layer
              << ") from layer " << access_layer
              << " possible through grid geometry " << grid_geometry
              << " with via cost " << option.total_via_cost;

    // FIXME: Should check if off_grid position is an existing on-grid vertex!
    RoutingVertex *existing = grid_geometry->VertexAt(point);
    if (existing) {
      return {{existing, target_layer}};
    }

    std::unique_ptr<RoutingVertex> off_grid(new RoutingVertex(point));
    off_grid->AddConnectedLayer(target_layer);
    off_grid->AddConnectedLayer(access_layer);

    std::set<RoutingTrackDirection> access_directions =
        ValidAccessDirectionsForVertex(*off_grid, for_nets);
    if (access_directions.empty()) {
      VLOG(15) << "Invalid off grid candidate at " << off_grid->centre();
      continue;
    }

    // If ConnectToSurroundingTracks has any success, we move ownership of the
    // off_grid vertex to the parent RoutingGrid.
    if (!ConnectToSurroundingTracks(*grid_geometry,
                                    access_layer,
                                    for_nets,
                                    access_directions,
                                    off_grid.get()).ok()) {
      // TODO(aryap): Accumulate errors?
      // The off-grid vertex could not be connected to any surrounding tracks.
      continue;
    }

    RoutingVertex *vertex = off_grid.release();
    AddVertex(vertex);
    return {{vertex, target_layer}};
  }

  return absl::NotFoundError("No workable options");
}

absl::StatusOr<RoutingGrid::VertexWithLayer>
RoutingGrid::ConnectToNearestAvailableVertex(
    const geometry::Port &port, const EquivalentNets &connectable_nets) {
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      layer_access = physical_db_.FindReachableLayersByPinLayer(port.layer());
  for (const auto &entry : layer_access) {
    for (const geometry::Layer &layer : entry.second) {
      LOG(INFO) << "checking for grid vertex on layer " << layer;
      auto vertex = ConnectToNearestAvailableVertex(
          port.centre(), layer, connectable_nets);
      if (vertex.ok()) {
        return {VertexWithLayer{.vertex = *vertex, .layer = layer}};
      }
      return vertex.status();
    }
  }
  return absl::NotFoundError(
      absl::StrCat("Couldn't find nearest available vertex for port ",
                   port.Describe()));
}

absl::StatusOr<RoutingVertex*> RoutingGrid::ConnectToNearestAvailableVertex(
    const geometry::Point &point,
    const geometry::Layer &target_layer,
    const EquivalentNets &for_nets) {
  // If constrained to one or two layers on a fixed grid, we can determine the
  // nearest vertices quickly by shortlisting those vertices whose positions
  // would correspond to the given point by construction (since we also
  // construct the grid).
  //
  // The more general solution, of finding the nearest vertex across any number
  // of layers, requires us to sort all available vertices by their proximity
  // to the position. This can be quite expensive. Also, there remains the
  // question of whether the vertex we find can be routed to.
  //
  // We have to check for available vertices on all the layers we might be able
  // to route to (i.e. insert a via to), and pick the cheapest.
  //
  // The first cut of this algorithm is to just find the closest of all the
  // available vertices on the given layer.

  // We need a copy to manipulate the layer:
  geometry::Point target_point = point;
  target_point.set_layer(target_layer);

  std::map<geometry::Layer, std::unique_ptr<RoutingVertex>>
      off_grid_candidate_by_layer;
  std::vector<CostedVertex> costed_vertices;
  for (const auto &entry : available_vertices_by_layer_) {
    const geometry::Layer &vertex_layer = entry.first;

    std::unique_ptr<RoutingVertex> off_grid(new RoutingVertex(target_point));
    off_grid->AddConnectedLayer(target_layer);

    // Is this layer reachable from the target?
    std::optional<std::pair<geometry::Layer, double>> needs_via;
    if (vertex_layer != target_layer) {
      off_grid->AddConnectedLayer(vertex_layer);

      needs_via = ViaLayerAndCost(vertex_layer, target_layer);
      if (!needs_via) {
        // This is a failure, since vertex_layer != target_layer.
        continue;
      }
      LOG(INFO) << "layer " << physical_db_.DescribeLayer(target_layer)
                << " is accessible for routing via layer "
                << physical_db_.DescribeLayer(needs_via->first);
    }

    // FIXME: Need to check if RoutingVertex and RoutingEdges we create off grid
    // go too close to in-use edges and vertices!
    if (!ValidAgainstKnownBlockages(*off_grid, for_nets).ok() ||
        !ValidAgainstInstalledPaths(*off_grid, for_nets).ok()) {
      VLOG(15) << "Invalid off grid candidate at " << off_grid->centre()
               << " layers " << vertex_layer << ", " << target_layer;
      continue;
    }

    off_grid_candidate_by_layer[vertex_layer] = std::move(off_grid);

    for (RoutingVertex *vertex : entry.second) {
      // Do not consider unavailable vertices! Unless they have connectable
      // nets!
      if (!vertex->available() && !(
          vertex->connectable_net() &&
          for_nets.Contains(*vertex->connectable_net())))
        continue;
      uint64_t vertex_cost = static_cast<uint64_t>(
          vertex->L1DistanceTo(target_point));
      if (needs_via) {
        // FIXME(aryap): Use via cost!
        vertex_cost += (10.0 * needs_via->second);
      }
      costed_vertices.emplace_back(CostedVertex {
          .cost = vertex_cost,
          .layer = vertex_layer,
          .vertex = vertex
      });
    }
  }

  // Should sort automatically based on operator< for first and second entries
  // in pairs.
  //
  // This inequality goes the wrong way because we pop from the back of the
  // vector, not the front, and that's where we want the lowest-cost elements to
  // end up.
  static auto comp = [](const CostedVertex &lhs,
                        const CostedVertex &rhs) {
    return lhs.cost > rhs.cost;
  };
  std::sort(costed_vertices.begin(), costed_vertices.end(), comp);

  // To ensure we can go the "last mile", we check if the required paths, as
  // projected on the tracks on which the nearest vertex lies, are legal.
  // Consider 4 vertices X on the RoutingGrid surrounding the port O.
  //
  //     (A)
  //    X---+       X           X
  // (B)|   |(B')
  //    +---O
  //     (A')
  //
  //
  //    X           X           X
  //
  // To access O we must go off-grid and beat a path on the layer closest to
  // it. We should not need to hop between horizontal/vertical track layers
  // unless a direct path on a single layer is blocked.
  //
  // In the diagram, (A) and (B) are the bridging edges, and (A') and (B') are
  // the off-grid edges.
  //
  // We have to check each possible path {(A), (B')} and {(B), (A')} for each
  // of the vertices. If we can't establish the path on the cloest layer alone
  // we might have to hop between them - in practice however this is unlikely
  // to work since grid spacing won't allow for two vias so close.
  //
  // We generate a new RoutingVertex for the landing spot on each track and
  // provide that to the grid-router to use in finding a shortest path.
  //
  // TODO(aryap): A better idea is to create up to, say, 4 candidate bridging
  // vertices on legal tracks around the point so that they can be included in
  // the global shortest-path search. This would avoid having to turn corners
  // and go backwards, for example.
  while (!costed_vertices.empty()) {
    RoutingVertex *candidate = costed_vertices.back().vertex;
    const geometry::Layer vertex_layer = costed_vertices.back().layer;
    costed_vertices.pop_back();

    VLOG(90) << "Searching "  << costed_vertices.size() << " vertex "
             << candidate << " centre " << candidate->centre()
             << " layer " << vertex_layer
             << " cost " << costed_vertices.back().cost;

    if (candidate->vertical_track() == nullptr) {
      // FIXME(aryap): Is this a problem?
      VLOG(90) << "Cannot use vertex " << candidate
               << " as candidate because vertical track is nullptr";
      continue;
    } else if (candidate->horizontal_track() == nullptr) {
      VLOG(90) << "Cannot use vertex " << candidate
               << " as candidate because horizontal track is nullptr";
      continue;
    }

    std::unique_ptr<RoutingVertex> &off_grid =
        off_grid_candidate_by_layer[vertex_layer];

    // Try putting it on the vertical track and then horizontal track.
    std::vector<RoutingTrack*> tracks = candidate->Tracks();
    RoutingVertex *bridging_vertex = nullptr;
    bool bridging_vertex_is_new = false;

    bool success = false;
    for (size_t i = 0; i < tracks.size(); ++i) {
      bridging_vertex = nullptr;
      bool off_grid_already_exists = false;
      success = tracks[i]->CreateNearestVertexAndConnect(
          *this,
          off_grid.get(),
          vertex_layer,
          for_nets,
          &bridging_vertex,
          &bridging_vertex_is_new,
          &off_grid_already_exists);

      if (!success) {
        continue;
      } else if (off_grid_already_exists) {
        if (bridging_vertex_is_new) {
          std::stringstream ss;
          ss << "Doesn't make sense for off_grid_already_exists and "
             << "bridging_vertex_is_new to both be true";
          return absl::InternalError(ss.str());
        }
        // We're done! We can just use an existing vertex since 'off_grid'
        // happens to already exist. 'off_grid' should NOT be added to the
        // routing grid, it should be discarded.
        return bridging_vertex;
      }
      break;
    }

    if (!success) {
      continue;
    }

    // Add off_grid now that we have a viable bridging_vertex.
    RoutingVertex *off_grid_copy = off_grid.get();
    AddVertex(off_grid.release());

    if (bridging_vertex == off_grid_copy) {
      // off_grid landed on the track and was subsumed and connected, we have
      // nothing left to do.
      return off_grid_copy;
    }

    // TODO(aryap): Need a way to roll back these temporary objects in case the
    // caller's entire process fails - i.e. a vertex can be created for the
    // starting point but not for the ending point.

    if (bridging_vertex_is_new) {
      // If the bridging_vertex was an existing vertex on the track, we don't
      // need to add it.
      AddVertex(bridging_vertex);
    }

    RoutingEdge *edge = new RoutingEdge(bridging_vertex, off_grid_copy);
    edge->set_layer(vertex_layer);
    if (!ValidAgainstKnownBlockages(*edge, for_nets).ok() ||
        !ValidAgainstInstalledPaths(*edge, for_nets).ok()) {
      VLOG(15) << "Invalid off grid edge between " << bridging_vertex->centre()
               << " and " << off_grid_copy->centre();
      // Rollback extra hard!
      if (bridging_vertex_is_new) {
        RemoveVertex(bridging_vertex, true);  // and delete!
      }
      RemoveVertex(off_grid_copy, true);  // and delete!
      delete edge;    

      // Have to recreate an off-grid candidate vertex for the next guy:
      std::unique_ptr<RoutingVertex> off_grid_replacement(
          new RoutingVertex(target_point));
      off_grid_replacement->AddConnectedLayer(vertex_layer);
      off_grid_replacement->AddConnectedLayer(target_layer);
      off_grid = std::move(off_grid_replacement);

      continue;
    }
    LOG(INFO) << "Connected new vertex " << bridging_vertex->centre()
              << " on layer " << edge->EffectiveLayer();
    bridging_vertex->AddEdge(edge);
    off_grid_copy->AddEdge(edge);
    
    // TODO(aryap): It's unclear what layer this edge is on. The opposite of
    // what the bridging edge is on, I guess.
    // TODO(aryap): It's not clear if the off-grid edge will be legal. We have
    // to check with the whole grid.

    off_grid_edges_.insert(edge);
    return off_grid_copy;
  }
  return absl::NotFoundError("");
}

std::set<RoutingTrackDirection> RoutingGrid::ValidAccessDirectionsForVertex(
    const RoutingVertex &vertex,
    const EquivalentNets &for_nets) {
  std::set<RoutingTrackDirection> access_directions = {
      RoutingTrackDirection::kTrackHorizontal,
      RoutingTrackDirection::kTrackVertical};
  for (auto it = access_directions.begin(); it != access_directions.end();) {
    const RoutingTrackDirection &direction = *it;
    absl::Status blocked =
        ValidAgainstKnownBlockages(vertex, for_nets, direction);
    blocked.Update(
        ValidAgainstInstalledPaths(vertex, for_nets, direction));
    if (!blocked.ok()) {
      VLOG(15) << "Cannot connect to " << vertex << " in direction "
               << direction << ": " << blocked.message();
      it = access_directions.erase(it);
    } else {
      ++it;
    }
  }
  return access_directions;
}

std::optional<geometry::Rectangle> RoutingGrid::ViaFootprint(
    const geometry::Point &centre,
    const geometry::Layer &other_layer,
    const geometry::Layer &footprint_layer,
    int64_t padding,
    const std::optional<RoutingTrackDirection> &direction) const {
  if (footprint_layer == other_layer) {
    // Empty footprint.
    return std::nullopt;
  }
  // Get the applicable via info for via sizing and encapsulation values:
  const RoutingViaInfo &routing_via_info = GetRoutingViaInfoOrDie(
      footprint_layer, other_layer);
  int64_t via_width =
      routing_via_info.EncapWidth(footprint_layer) + 2 * padding;
  int64_t via_length =
      routing_via_info.EncapLength(footprint_layer) + 2 * padding;

  geometry::Point lower_left;
  geometry::Rectangle footprint;

  if (!direction) {
    int64_t square_width = std::max(via_width, via_length);
    lower_left = centre - geometry::Point(square_width / 2, square_width / 2);
    footprint = geometry::Rectangle(lower_left, square_width, square_width);
    footprint.set_layer(footprint_layer);
  } else {
    switch (*direction) {
      case RoutingTrackDirection::kTrackVertical:
        lower_left = centre - geometry::Point(via_width / 2, via_length / 2);
        footprint = geometry::Rectangle(lower_left, via_width, via_length);
        break;
      case RoutingTrackDirection::kTrackHorizontal:
        lower_left = centre - geometry::Point(via_length / 2, via_width / 2);
        footprint = geometry::Rectangle(lower_left, via_length, via_width);
        break;
      default:
        LOG(FATAL) << "Unknown RoutingTrackDirection: " << *direction;
    }
  }
  footprint.set_layer(footprint_layer);
  return footprint;
}

std::optional<geometry::Rectangle> RoutingGrid::ViaFootprint(
    const RoutingVertex &vertex,
    const geometry::Layer &footprint_layer,
    int64_t padding,
    const std::optional<RoutingTrackDirection> &direction) const {
  std::set<geometry::Layer> vertex_layers = vertex.connected_layers();

  // We expect footprint_layer to appear in the vertex's list of connected
  // layers.
  size_t erased = vertex_layers.erase(footprint_layer);
  if (erased == 0) {
    // This vertex doesn't even connect footprint_layer. It has to have an empty
    // footprint.
    return std::nullopt;
  }

  // If there is more than 1 layer left in the connected layer list, we have a
  // problem because we assume that the vertex connects to at most 2 layers.
  // That shouldn't happen. There should be 0 or 1.
  if (vertex_layers.empty()) {
    return std::nullopt;
  }
  if (vertex_layers.size() > 1) {
    // This is an error.
    LOG(FATAL) << "Vertex connects more than 2 layers!";
  }

  const geometry::Layer &other_layer = *vertex_layers.begin();

  return ViaFootprint(
      vertex.centre(), other_layer, footprint_layer, padding, direction);

  //const std::vector<geometry::Layer> &layers = vertex.connected_layers();
  //if (layers.size() != 2) {
  //  return std::nullopt;
  //}
  //const geometry::Layer &first_layer = layers.front();
  //const geometry::Layer &second_layer = layers.back();

  //return ViaFootprint(
  //    vertex.centre(), first_layer, second_layer, padding, direction);
}

std::optional<geometry::Rectangle> RoutingGrid::TrackFootprint(
    const RoutingEdge &edge, int64_t padding) const {
  const geometry::Layer &layer = edge.EffectiveLayer();
  const RoutingLayerInfo &layer_info = GetRoutingLayerInfoOrDie(layer);
  auto edge_as_rectangle = edge.AsRectangle(layer_info.wire_width());
  if (!edge_as_rectangle)
    return std::nullopt;
  if (padding == 0)
    return edge_as_rectangle;
  geometry::Rectangle &rectangle = edge_as_rectangle.value();
  return rectangle.WithPadding(padding);
}

std::optional<geometry::Rectangle> RoutingGrid::EdgeWireFootprint(
    const RoutingEdge &edge, int64_t padding) const {
  if (!edge.layer()) {
    LOG(WARNING) << "Edge cannot be turned into rectangle without layer_ set";
    return std::nullopt;
  }

  const geometry::Layer &layer = *edge.layer();
  int64_t width = GetRoutingLayerInfoOrDie(layer).wire_width() + padding;

  // A rectangle of wire-width without via encaps at either end represents the
  // middle section of the edge.
  auto wire_only_bounds = edge.AsRectangle(width);
  if (!wire_only_bounds) {
    LOG(FATAL) << "Edge does not have simple rectangle form!";
    return std::nullopt;
  }
  return *wire_only_bounds;
}

// TODO(aryap): It's convenient to have an edge generate the footprint it would
// imply as a wire, complete with vias at either end for the vertices (with
// appropriate metal encap sizes given the layers the vertices connect). That
// would require RoutingEdge to have knowledge of physical constraints or at
// least the RoutingGrid, since we hide behind that abstraction in Routing
// stuff. And that would require RoutingGrid to know a lot about RoutingEdge and
// also the opposite, which is annoying and not very clean as an abstraction.
//
// TODO(aryap): Memoise this: we only need to generate the footprint once for
// many checks.
//
// This returns the rectangle which covers both the end via encaps and the wire
// for the edge, and is therefore a worst-case scenario.
std::optional<geometry::Rectangle> RoutingGrid::EdgeFootprint(
    const RoutingEdge &edge, int64_t padding) const {
  if (!edge.layer()) {
    LOG(WARNING) << "Edge cannot be turned into rectangle without layer_ set";
    return std::nullopt;
  }

  const geometry::Layer &layer = *edge.layer();

  // A rectangle of wire-width without via encaps at either end represents the
  // middle section of the edge.
  auto wire_only_bounds = EdgeWireFootprint(edge, padding);
  if (!wire_only_bounds) {
    LOG(FATAL) << "Edge does not have simple rectangle form!";
    return std::nullopt;
  }

  // Sort [first, second] vertices into bottom-left-most to upper-right-most:
  std::vector<RoutingVertex*> vertices = {edge.first(), edge.second()};
  std::sort(
      vertices.begin(),
      vertices.end(),
      [](RoutingVertex *lhs, RoutingVertex *rhs) {
        const geometry::Point &left_point = lhs->centre();
        const geometry::Point &right_point = rhs->centre();
        if (left_point.x() == right_point.x()) {
          return left_point.y() < right_point.y();
        }
        return left_point.x() < right_point.x();
      }
  );

  const RoutingVertex *lower_left = vertices.front();
  const RoutingVertex *upper_right = vertices.back();

  auto lower_left_footprint = ViaFootprint(
      *lower_left, layer, padding, edge.Direction());
  auto upper_right_footprint = ViaFootprint(
      *upper_right, layer, padding, edge.Direction());

  std::vector<geometry::Point> lower_left_options = {
    wire_only_bounds->lower_left()
  };
  std::vector<geometry::Point> upper_right_options = {
    wire_only_bounds->upper_right()
  };
  if (lower_left_footprint) {
    lower_left_options.push_back(lower_left_footprint->lower_left());
    upper_right_options.push_back(lower_left_footprint->upper_right());
  }
  if (upper_right_footprint) {
    lower_left_options.push_back(upper_right_footprint->lower_left());
    upper_right_options.push_back(upper_right_footprint->upper_right());
  }

  // TODO(aryap): A good exercise! Define a function on geometry::Point that
  // will accept either any iterator of Points or any container of Points to
  // automatically return the (min, max) x (x, y) over a range.
  geometry::Point lower_left_point = {
      std::min_element(lower_left_options.begin(),
                       lower_left_options.end(),
                       geometry::Point::CompareX)->x(),
      std::min_element(lower_left_options.begin(),
                       lower_left_options.end(),
                       geometry::Point::CompareY)->y()
  };
  geometry::Point upper_right_point = {
      std::max_element(upper_right_options.begin(),
                       upper_right_options.end(),
                       geometry::Point::CompareX)->x(),
      std::max_element(upper_right_options.begin(),
                       upper_right_options.end(),
                       geometry::Point::CompareY)->y()
  };

  geometry::Rectangle footprint = {{lower_left_point, upper_right_point}};
  if (edge.PermanentNet()) {
    footprint.set_net(*edge.PermanentNet());
  }
  footprint.set_layer(layer);
  return footprint;
}

std::vector<RoutingVertex*> &RoutingGrid::GetAvailableVertices(
    const geometry::Layer &layer) {
  auto it = available_vertices_by_layer_.find(layer);
  if (it == available_vertices_by_layer_.end()) {
    auto insert_it = available_vertices_by_layer_.insert({layer, {}});
    LOG_IF(FATAL, !insert_it.second)
        << "Couldn't create entry for layer " << layer
        << " in available vertices map.";
    it = insert_it.first;
  }
  // Gotta dereference the iterator to get to the goods!
  return it->second;
}

std::optional<std::pair<geometry::Layer, double>> RoutingGrid::ViaLayerAndCost(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  if (lhs == rhs)
    return std::nullopt;

  std::optional<std::reference_wrapper<const RoutingViaInfo>> needs_via =
      GetRoutingViaInfo(lhs, rhs);
  if (!needs_via) {
    return std::nullopt;
  }
  return std::make_pair(needs_via->get().layer(), needs_via->get().cost());
}

absl::Status RoutingGrid::ConnectLayers(
    const geometry::Layer &first, const geometry::Layer &second) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  const RoutingLayerInfo &horizontal_info = split_directions.first;
  const RoutingLayerInfo &vertical_info = split_directions.second;

  auto maybe_routing_via_info = GetRoutingViaInfo(first, second);
  if (!maybe_routing_via_info) {
    std::stringstream ss;
    ss << "Could not get RoutingViaInfo for " << first
       << ", " << second;
    LOG(ERROR) << ss.str();
    return absl::NotFoundError(ss.str());
  }
  const RoutingViaInfo &routing_via_info = maybe_routing_via_info->get();

  LOG(INFO) << "Drawing grid between layers " << horizontal_info.layer()
            << ", " << vertical_info.layer();

  RoutingGridGeometry grid_geometry;
  grid_geometry.ComputeForLayers(horizontal_info, vertical_info);

  size_t num_vertices = 0;
  size_t num_x = 0;
  size_t num_y = 0;

  std::map<int64_t, RoutingTrack*> vertical_tracks;
  std::map<int64_t, RoutingTrack*> horizontal_tracks;

  // Generate tracks to hold edges and vertices in each direction.
  for (int64_t x = grid_geometry.x_start();
       x <= grid_geometry.x_max();
       x += grid_geometry.x_pitch()) {
    RoutingTrack *track = new RoutingTrack(
        vertical_info.layer(),
        RoutingTrackDirection::kTrackVertical,
        grid_geometry.x_pitch(),
        vertical_info.wire_width(),
        routing_via_info.EncapWidth(vertical_info.layer()),
        routing_via_info.EncapLength(vertical_info.layer()),
        vertical_info.min_separation(),
        x);
    vertical_tracks.insert({x, track});
    grid_geometry.vertical_tracks_by_index().push_back(track);
    AddTrackToLayer(track, vertical_info.layer());
    num_x++;
  }

  for (int64_t y = grid_geometry.y_start();
       y <= grid_geometry.y_max();
       y += grid_geometry.y_pitch()) {
    RoutingTrack *track = new RoutingTrack(
        horizontal_info.layer(),
        RoutingTrackDirection::kTrackHorizontal,
        grid_geometry.y_pitch(),
        horizontal_info.wire_width(),
        routing_via_info.EncapWidth(horizontal_info.layer()),
        routing_via_info.EncapLength(horizontal_info.layer()),
        horizontal_info.min_separation(),
        y);
    horizontal_tracks.insert({y, track});
    grid_geometry.horizontal_tracks_by_index().push_back(track);
    AddTrackToLayer(track, horizontal_info.layer());
    num_y++;
  }

  std::vector<std::vector<RoutingVertex*>> &vertices =
      grid_geometry.vertices_by_grid_position();

  // Generate a vertex at the intersection of every horizontal and vertical
  // track.
  size_t i = 0;
  for (int64_t x = grid_geometry.x_start();
       x <= grid_geometry.x_max();
       x += grid_geometry.x_pitch()) {
    // This (and the horizontal one) must exist by now, so we can make this
    // fatal.
    RoutingTrack *vertical_track = vertical_tracks.find(x)->second;
    LOG_IF(FATAL, !vertical_track) << "Vertical routing track is nullptr";

    size_t j = 0;
    for (int64_t y = grid_geometry.y_start();
         y <= grid_geometry.y_max();
         y += grid_geometry.y_pitch()) {
      RoutingTrack *horizontal_track = horizontal_tracks.find(y)->second;
      LOG_IF(FATAL, !horizontal_track) << "Horizontal routing track is nullptr";

      RoutingVertex *vertex = new RoutingVertex(geometry::Point(x, y));
      // These methods will assign the respective horizontal_track and
      // vertical_tracks of the vertex to the tracks themselves.
      horizontal_track->AddVertex(vertex);
      vertical_track->AddVertex(vertex);

      ++num_vertices;
      vertex->AddConnectedLayer(first);
      vertex->AddConnectedLayer(second);

      AddVertex(vertex);

      VLOG(20) << "Vertex created: " << vertex->centre() << " on layers: "
               << absl::StrJoin(vertex->connected_layers(), ", ");

      vertex->set_grid_position_x(i);
      vertex->set_grid_position_y(j);

      vertices[i][j] = vertex;

      // Assign neighbours. Since we do the reciprocal relationship too, we
      // assign up to all 8 neighbours per iteration.
      if (i > 0) {
        // Left neighbour.
        RoutingVertex *neighbour = vertices[i - 1][j];
        vertex->AddNeighbour(Compass::LEFT, neighbour);
        neighbour->AddNeighbour(Compass::RIGHT, vertex);

        if (j > 0) {
          // Lower left neighbour.
          RoutingVertex *neighbour = vertices[i - 1][j - 1];
          vertex->AddNeighbour(Compass::LOWER_LEFT, neighbour);
          neighbour->AddNeighbour(Compass::UPPER_RIGHT, vertex);
        }

        if (j < vertices[i].size() - 1) {
          // Upper left neighbour.
          RoutingVertex *neighbour = vertices[i - 1][j + 1];
          vertex->AddNeighbour(Compass::UPPER_LEFT, neighbour);
          neighbour->AddNeighbour(Compass::LOWER_RIGHT, vertex);
        }
      }
      if (j > 0) {
        // Lower neighbour.
        RoutingVertex *neighbour = vertices[i][j - 1];
        vertex->AddNeighbour(Compass::LOWER, neighbour);
        neighbour->AddNeighbour(Compass::UPPER, vertex);
      }
      j++;
    }
    i++;
  }

  // This adds a copy of the object to our class's bookkeeping. It's kinda
  // annoying. I'd rather create it on the fly to avoid the copy.
  // TODO(aryap): Avoid this copy.
  absl::Status add_grid = AddRoutingGridGeometry(first, second, grid_geometry);
  if (!add_grid.ok()) {
    return add_grid;
  }

  size_t num_edges = 0;
  for (auto entry : tracks_by_layer_)
    for (RoutingTrack *track : entry.second)
      num_edges += track->edges().size();

  LOG(INFO) << "Connected layer " << first << " and " << second << "; "
            << "generated " << horizontal_tracks.size() << " horizontal and "
            << vertical_tracks.size() << " vertical tracks, "
            << num_vertices << " vertices and "
            << num_edges << " edges.";

  if (VLOG_IS_ON(80)) {
    for (auto entry : tracks_by_layer_) {
      const geometry::Layer &layer = entry.first;
      for (RoutingTrack *track : entry.second) {
        LOG(INFO) << layer << " track: " << *track;
      }
    }
  }

  return absl::OkStatus();
}

bool RoutingGrid::ContainsVertex(RoutingVertex *vertex) const {
  // Did you know? std::begin(...) and std::end(...) were introduced in C++11
  // and have more general compatibility, such as with old C-style arrays.
  return std::find(
      vertices_.begin(), vertices_.end(), vertex) != vertices_.end();
}

void RoutingGrid::AddVertex(RoutingVertex *vertex) {
  for (const geometry::Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingVertex*> &available = GetAvailableVertices(layer);
    available.push_back(vertex);
  }
  DCHECK(!ContainsVertex(vertex));
  vertices_.push_back(vertex);  // The class owns all of these.
}

void RoutingGrid::AddOffGridEdge(RoutingEdge *edge) {
  off_grid_edges_.insert(edge);
}

absl::Status RoutingGrid::AddMultiPointRoute(
    const Layout &layout,
    const std::vector<std::vector<geometry::Port*>> ports,
    const std::optional<std::string> &primary_net_name) {
  EquivalentNets net_aliases;
  for (const auto &port_set : ports) {
    for (const geometry::Port *port : port_set) {
      net_aliases.Add(port->net());
    }
  }
  if (primary_net_name) {
    net_aliases.set_primary(*primary_net_name);
  }

  geometry::ShapeCollection connectables;
  layout.CopyConnectableShapesNotOnNets(net_aliases, &connectables);

  return AddMultiPointRoute(ports,
                            connectables,
                            net_aliases);
}

absl::Status RoutingGrid::AddMultiPointRoute(
    const std::vector<std::vector<geometry::Port*>> ports,
    const geometry::ShapeCollection &avoid,
    const EquivalentNets &nets) {
  bool all_ok = true;
  // The net_name is set once the first route is laid between some pair of
  // ports. Subsequent routes are to the net, not any particular point.
  std::optional<std::string> net_name = std::nullopt;
  for (auto it = ports.begin(); it != ports.end(); ++it) {
    const std::vector<geometry::Port*> &port_group = *it;
    if (!net_name) {
      if ((it + 1) == ports.end()) {
        break;
      }
      const std::vector<geometry::Port*> &next_port_group = *(it + 1);
      geometry::PortSet begin_ports = geometry::Port::MakePortSet();
      begin_ports.insert(port_group.begin(), port_group.end());
      geometry::PortSet end_ports = geometry::Port::MakePortSet();
      end_ports.insert(next_port_group.begin(), next_port_group.end());
      bool path_found = AddBestRouteBetween(
          begin_ports,
          end_ports,
          avoid,
          nets).ok();
      if (path_found) {
        net_name = nets.primary();
        ++it;
      } else {
        all_ok = false;
      }
      continue;
    }

    bool path_found = false;
    for (geometry::Port *port : port_group) {
      auto route_status = AddRouteToNet(*port, *net_name, nets, avoid);
      if (route_status.ok()) {
        path_found = true;
        break;
      }
    }
    all_ok = path_found && all_ok;
  }
  return all_ok ?
      absl::OkStatus() : absl::NotFoundError("Not all ports could be routed");
}

absl::Status RoutingGrid::AddBestRouteBetween(
    const geometry::PortSet &begin_ports,
    const geometry::PortSet &end_ports,
    const geometry::ShapeCollection &avoid,
    const EquivalentNets &nets) {
  std::vector<RoutingPath*> options;
  for (const geometry::Port *begin : begin_ports) {
    for (const geometry::Port *end : end_ports) {
      auto maybe_path = FindRouteBetween(*begin, *end, avoid, nets);
      if (!maybe_path.ok()) {
        continue;
      }
      options.push_back(*maybe_path);
    }
  }
  if (options.empty()) {
    LOG(ERROR) << "None of the begin/end combinations yielded a workable path.";
    return absl::NotFoundError(
        "None of the begin/end combinations yielded a workable path.");
  }
  auto sort_fn = [&](RoutingPath *lhs, RoutingPath *rhs) {
    return lhs->Cost() < rhs->Cost();
  };
  std::sort(options.begin(), options.end(), sort_fn);

  for (RoutingPath *path : options) {
    LOG(INFO) << "cost: " << path->Cost() << " option: " << path->Describe();
  }

  // Install lowest-cost path. The RoutingGrid takes ownership of this one. The
  // rest must be deleted.
  absl::Status install_status = InstallPath(options.front());

  for (auto it = options.begin() + 1; it != options.end(); ++it) {
    delete *it;
  }
  return install_status;
}

absl::Status RoutingGrid::AddRouteBetween(
    const geometry::Port &begin,
    const geometry::Port &end,
    const geometry::ShapeCollection &avoid,
    const EquivalentNets &nets) {
  absl::StatusOr<RoutingPath*> find_path =
      FindRouteBetween(begin, end, avoid, nets);

  if (!find_path.ok()) {
    return find_path.status();
  }
  std::unique_ptr<RoutingPath> shortest_path =
      std::unique_ptr<RoutingPath>(*find_path);
  absl::Status install = InstallPath(shortest_path.release());
  return install;
}

absl::StatusOr<RoutingPath*> RoutingGrid::FindRouteBetween(
    const geometry::Port &begin,
    const geometry::Port &end,
    const geometry::ShapeCollection &avoid,
    const EquivalentNets &nets) {
  // Override the vertex availability check for this search to avoid
  // obstructions in the given avoid set. Useful since it doesn't mutate the
  // global starting state for the purpose of the search, but we have to
  // serialise searches to ensure determinism anyway, making it kind of
  // a pointless feature. It's also noticeably slower, since now a boolean check
  // per graph entity is replaced by a container lookup.
  //
  // Instead, we could temporarily mark the effected vertices unavailable and
  // reset them after.
  // TODO(aryap): That doesn't work today because RoutingTrack has a very
  // complicated way of managing, sorting and merging blockages, and temporary
  // blockages would need to be freshly supported. Probably not worth the
  // complexity just to avoid temporarily blockages like pins.
  //auto usable_vertex = [&](RoutingVertex *vertex) {
  //  if (!vertex->available()) return false;

  //  std::optional<geometry::Rectangle> via_footprint = ViaFootprint(
  //      *vertex,
  //      100);   // FIXME(aryap): This padding value is the max of the
  //              // min_separations on the top and bottom layers and should just
  //              // be automatically found.
  //  if (!via_footprint)
  //    return true;

  //  for (geometry::Port *port : avoid) {
  //    if (port->Overlaps(via_footprint.value()))
  //      return false;
  //  }
  //  return true;
  //};
  TemporaryBlockageInfo temporary_blockages;
  SetUpTemporaryBlockages(avoid, &temporary_blockages);

  absl::Cleanup tear_down_temporary_blockages = [&]() {
    TearDownTemporaryBlockages(temporary_blockages);
  };

  auto begin_connection = ConnectToGrid(begin, nets);
  if (!begin_connection.ok()) {
    std::stringstream ss;
    ss << "Could not find available vertex for begin port: "
       << begin_connection.status().message();
    LOG(ERROR) << ss.str();
    return absl::NotFoundError(ss.str());
  }
  RoutingVertex *begin_vertex = begin_connection->vertex;
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  auto end_connection = ConnectToGrid(end, nets);
  if (!end_connection.ok()) {
    std::stringstream ss;
    ss << "Could not find available vertex for end port: "
       << end_connection.status().message();
    LOG(ERROR) << ss.str();
    return absl::NotFoundError(ss.str());
  }
  RoutingVertex *end_vertex = end_connection->vertex;
  LOG(INFO) << "Nearest vertex to end (" << end << ") is "
            << end_vertex->centre();

  auto shortest_path_result = ShortestPath(begin_vertex, end_vertex);
  if (!shortest_path_result.ok()) {
    std::string message = absl::StrCat(
        "No path found: ", shortest_path_result.status().message());
    LOG(WARNING) << message;
    return absl::NotFoundError(message);
  }
  std::unique_ptr<RoutingPath> shortest_path(*shortest_path_result);

  // Remember the ports to which the path should connect.
  //
  // Ports are typically on port layers (i.e. PINs), but for convenience we also
  // record the layer we need to use to access said ports:
  shortest_path->set_start_port(&begin);
  shortest_path->start_access_layers().insert(begin_connection->layer);
  shortest_path->set_end_port(&end);
  shortest_path->end_access_layers().insert(end_connection->layer);

  LOG(INFO) << "Found path: " << *shortest_path;

  // Assign net:
  if (!nets.Empty())
    shortest_path->set_nets(nets);

  // It is important that temporary blockages be torn down before the path is
  // installed, but since that is managed by the caller and since teardown will
  // happen when the absl::Cleanup goes out of scope, we no longer have to do
  // it manually.
  // std::move(tear_down_temporary_blockages).Invoke();
 
  return shortest_path.release();
}

namespace {

std::set<geometry::Layer> EffectiveLayersForInstalledVertex(
    RoutingVertex *vertex) {
  std::set<geometry::Layer> layers;
  for (auto &entry : vertex->installed_in_paths()) {
    for (RoutingEdge *edge : entry.second) {
      layers.insert(edge->EffectiveLayer());
    }
  }
  return layers;
}

}   // namespace

absl::Status RoutingGrid::AddRouteToNet(
    const geometry::Port &begin,
    const EquivalentNets &target_nets,
    const EquivalentNets &usable_nets,
    const geometry::ShapeCollection &avoid) {
  absl::StatusOr<RoutingPath*> find_path =
      FindRouteToNet(begin, target_nets, usable_nets, avoid);
  if (!find_path.ok()) {
    return find_path.status();
  }
  std::unique_ptr<RoutingPath> shortest_path =
      std::unique_ptr<RoutingPath>(*find_path);
  absl::Status install = InstallPath(shortest_path.release());
  return install;
}

absl::StatusOr<RoutingPath*> RoutingGrid::FindRouteToNet(
    const geometry::Port &begin,
    const EquivalentNets &target_nets,
    const EquivalentNets &usable_nets,
    const geometry::ShapeCollection &avoid) {
  TemporaryBlockageInfo temporary_blockages;
  SetUpTemporaryBlockages(avoid, &temporary_blockages);

  auto begin_connection = ConnectToGrid(begin, usable_nets);
  if (!begin_connection.ok()) {
    std::stringstream ss;
    ss << "Could not find available vertex for begin port: "
       << begin_connection.status().message();
    LOG(ERROR) << ss.str();
    TearDownTemporaryBlockages(temporary_blockages);
    return absl::NotFoundError(ss.str());
  }
  RoutingVertex *begin_vertex = begin_connection->vertex;
  LOG(INFO) << "Nearest vertex to begin (" << begin << ") is "
            << begin_vertex->centre();

  RoutingVertex *end_vertex;

  auto shortest_path_result = ShortestPath(
      begin_vertex, target_nets, &end_vertex);
  if (!shortest_path_result.ok()) {
    std::string message = absl::StrCat(
        "No path found to net ", target_nets.primary(), ".");
    LOG(WARNING) << message;
    TearDownTemporaryBlockages(temporary_blockages);
    return absl::NotFoundError(message);
  }

  // Claim the pointer.
  std::unique_ptr<RoutingPath> shortest_path(*shortest_path_result);

  // Remember the ports to which the path should connect.
  shortest_path->set_start_port(&begin);
  shortest_path->start_access_layers().insert(begin_connection->layer);

  // We expect that we now have a path terminating in a vertex that is attached
  // to the given net.
  //
  // We can assume that the vertex attaches to the net on one of its
  // connectable_layers_, but which one exactly is the best to use depends on
  // which path the vertex is installed in.
  //
  // Because we haven't called InstallPath yet, vertices have not been assigned
  // permanent in/out_edge() values.
  std::set<geometry::Layer> end_layers = EffectiveLayersForInstalledVertex(
      end_vertex);
  // TODO(aryap): Is this ever not-empty? InstallPath also populates
  // installed_in_paths on the RoutingVertex...
  shortest_path->end_access_layers().insert(
      end_layers.begin(), end_layers.end());

  if (shortest_path->End()->net() != "" &&
      shortest_path->End()->explicit_net_layer()) {
    shortest_path->end_access_layers().insert(
        *shortest_path->End()->explicit_net_layer());
  }

  LOG(INFO) << "Found path: " << *shortest_path;

  // Assign net and install:
  EquivalentNets all_nets(target_nets);
  all_nets.Add(usable_nets);
  shortest_path->set_nets(all_nets);

  TearDownTemporaryBlockages(temporary_blockages);

  return shortest_path.release();
}

bool RoutingGrid::RemoveVertex(RoutingVertex *vertex, bool and_delete) {
  if (vertex->horizontal_track())
    vertex->horizontal_track()->RemoveVertex(vertex);
  if (vertex->vertical_track())
    vertex->vertical_track()->RemoveVertex(vertex);

  // Check for instances of this vertex in off-grid edges:
  for (auto it = off_grid_edges_.begin(); it != off_grid_edges_.end();) {
    RoutingEdge *edge = *it;
    if (edge->first() == vertex || edge->second() == vertex) {
      VLOG(10) << "Removing off-grid edge " << edge
               << " because it includes vertex " << vertex;
      edge->PrepareForRemoval();
      delete edge;
      it = off_grid_edges_.erase(it);
    } else {
      ++it;
    }
  }

  for (const geometry::Layer &layer : vertex->connected_layers()) {
    auto it = available_vertices_by_layer_.find(layer);
    if (it == available_vertices_by_layer_.end())
      continue;
    auto &available_vertices = it->second;
    auto pos = std::find(
        available_vertices.begin(), available_vertices.end(), vertex);
    if (pos == available_vertices.end()) {
      // Already removed from availability list.
      continue;
    }
    available_vertices.erase(pos);
  }

  auto pos = std::find(vertices_.begin(), vertices_.end(), vertex);
  LOG_IF(WARNING, pos == vertices_.end())
      << "Did not find vertex we're removing in RoutingGrid list of "
      << "vertices: " << vertex;
  vertices_.erase(pos);
  if (and_delete)
    delete vertex;
  return true; // TODO(aryap): Always returning true, huh...
}

// Disable neighbouring vertices now that this one is in use. The implication of
// a vertex here is that a via will be placed at vertex->centre(), which means
// that none of the surrounding vertices can be used for vias.
//
// We do this even though the vias might get "optimised out".
void RoutingGrid::InstallVertexInPath(
    RoutingVertex *vertex, const std::string &net) {
  if (vertex->horizontal_track() && vertex->vertical_track()) {
    // If the vertex is on the grid, we only disable the recorded neighbours.
    // We maybe could get await without adding blockages to their tracks as well
    // because these neighbours are, by virtue of being on the grid, spaced
    // appropriately to accommodate a via and a wire track next to each other -
    // however, we rely on these blockages to determine appropriate connection
    // points for new off-grid vertices, so we should add the blockages anyway.
    //
    // NOTE(aryap): Nearby bridging vertices will not be disabled, be will be
    // unusable anyway (they are created to access a single off-grid point)
    // so this optimisation is ok.
    //
    // TODO(aryap): Determine if the junction vertices should have blockages
    // added where they are on-grid in InstallVertexInPath.
    static const std::vector<Compass> kDisabledNeighbours = {
      Compass::UPPER_LEFT,
      Compass::UPPER,
      Compass::UPPER_RIGHT,
      Compass::LEFT,
      Compass::RIGHT,
      Compass::LOWER_LEFT,
      Compass::LOWER,
      Compass::LOWER_RIGHT,
    };
    for (const auto &position : kDisabledNeighbours) {
      std::set<RoutingVertex*> neighbours = vertex->GetNeighbours(position);
      for (RoutingVertex *neighbour : neighbours) {
        if (neighbour->available() ) {
          neighbour->set_available(false);
          neighbour->set_connectable_net(vertex->net());
        } else if (neighbour->connectable_net() &&
                   *neighbour->connectable_net() != vertex->net()) {
          // If the neighbour is flagged as usable for a different net, disable
          // that.
          neighbour->set_connectable_net(std::nullopt);
        }
      }
    };
    return;
  }
  // If the vertex is off-grid, we have to search for affected neighbours
  // more painstakingly.
  //
  // There are two different sets of surrounding vertices on the grid:
  //
  //      |E     |I     |K     |M
  //  ----x------x------x------x-----
  //      |      |      |      |
  //      |F     |A     |C     |N
  //  ----x------x------x------x-----
  //      |      |  x <------------------ Z, off-grid point
  //      |G     |B     |D     |O
  //  ----x------x------x------x-----
  //      |      |      |      |
  //      |H     |J     |L     |P
  //  ----x------x------x------x-----
  //      |      |      |      |
  //
  // The inner vertices surrounding Z (A, B, C, D) will definitely conflict, so
  // we don't bother to check the distance between a via at O and a via at any
  // of their positions. The outer vertices (E - P) will only conflict if the
  // via at Z is positioned in a certain way within the ABDC rectangle, so we
  // have to check those explicitly.
  std::set<RoutingVertex*> vertices;
  std::set<RoutingVertex*> inner_vertices;
  for (const geometry::Layer &layer : vertex->connected_layers()) {
    std::vector<RoutingGridGeometry*> grid_geometries =
        FindRoutingGridGeometriesUsingLayer(layer);
    for (RoutingGridGeometry *grid_geometry : grid_geometries) {
      grid_geometry->EnvelopingVertices(
          vertex->centre(),
          &vertices,
          0,
          2);   // num_concentric_layers = 2 yields vertices A - P in the above
                // example.
      grid_geometry->EnvelopingVertices(
          vertex->centre(),
          &inner_vertices,
          0,
          1);   // num_concentric_layers = 1 yields vertices A - D.
    }
  }
  std::vector<RoutingVertex*> outer_vertices;
  std::set_difference(vertices.begin(), vertices.end(),
                      inner_vertices.begin(), inner_vertices.end(),
                      std::back_inserter(outer_vertices));

  std::set<RoutingTrack*> blocked_tracks;
  for (RoutingVertex *enveloping_vertex : inner_vertices) {
    enveloping_vertex->set_available(false);
    enveloping_vertex->set_connectable_net(vertex->net());
    // We also have to add blockages to the tracks on which these vertices
    // appear, since by being off-grid we're _presumably_ too close to
    // accomodate both a via and an edge next to each other.
    //
    // (We can explicitly check this if it is ever necessary.)
    //
    //      |A           |C
    //  ----x-----(1)----x-----
    //   (4)|            |
    //            O      |
    //      x-----x     (2)
    //      |B           |
    //      |            |
    //  --- x ----(3)----x-----
    //      |E           |D
    //
    //  B is too close to A. We project the via footprint at B onto tracks (1)
    //  and (3). Since B is on track (4), the blockage to track (4) will be
    //  taken care of elsewhere.
    //
    //  O is too close to A, C, D, B, E.  We project the via footprint onto
    //  tracks (1), (2), (3), (4), since O participates in none of those tracks.
    blocked_tracks.insert(enveloping_vertex->horizontal_track());
    blocked_tracks.insert(enveloping_vertex->vertical_track());
  }
  // This would happen anyway (because off-grid vertices have at least one
  // nullptr track), but we make it explicit:
  blocked_tracks.erase(nullptr);
  blocked_tracks.erase(vertex->horizontal_track());
  blocked_tracks.erase(vertex->vertical_track());

  for (const geometry::Layer &layer : vertex->connected_layers()) {
    // If there is an edge on this layer, we use its direction. Otherwise we use
    // the routing grid default direction for the layer.
    RoutingEdge *edge = vertex->GetEdgeOnLayer(layer);
    RoutingTrackDirection direction;
    if (edge) {
      direction = edge->Direction();
    } else {
      auto routing_layer_info = GetRoutingLayerInfo(layer);
      if (!routing_layer_info) {
        // No routing on this layer and no known direction, ignore.
        continue;
      }
      direction = routing_layer_info->get().direction();
    }

    std::optional<geometry::Rectangle> via_encap = ViaFootprint(
       *vertex, layer, 0, direction);
    if (!via_encap)
      continue;
    //LOG(INFO) << "via encap: " << *via_encap << " about " << vertex->centre()
    //          << " layer " << layer << " for edge " << *edge;
    for (RoutingTrack *track : blocked_tracks) {
      if (track->layer() != layer)
        continue;
      track->AddBlockage(*via_encap, 0, net, nullptr, nullptr);
    }

    int64_t min_separation = physical_db_.Rules(layer).min_separation;
    for (RoutingVertex *enveloping_vertex : outer_vertices) {
      std::optional<geometry::Rectangle> outer_via_encap = ViaFootprint(
          *enveloping_vertex, layer, 0);
      int64_t min_distance = static_cast<int64_t>(std::ceil(
          via_encap->ClosestDistanceTo(*outer_via_encap)));
      if (min_distance < min_separation) {
        //LOG(INFO) << "vertex " << enveloping_vertex->centre()
        //          << " is too close (" << min_distance << " < "
        //          << min_separation << ") to "
        //          << *via_encap << " at " << vertex->centre();
        enveloping_vertex->set_available(false);
        enveloping_vertex->set_connectable_net(vertex->net());
      }
    }
  }
}

absl::Status RoutingGrid::InstallPath(RoutingPath *path) {
  if (path->Empty()) {
    return absl::InvalidArgumentError("Cannot install an empty path.");
  }

  LOG(INFO) << "Installing path " << *path << " with net "
            << path->nets().primary();

  // Legalise the path. TODO(aryap): This might modify the edges the path
  // contains, which smells funny.
  path->Legalise();

  // Mark edges as unavailable with track which owns them.
  for (RoutingEdge *edge : path->edges()) {
    if (edge->track() != nullptr) {
      edge->track()->MarkEdgeAsUsed(edge, path->nets().primary());
    } else {
      edge->SetPermanentNet(path->nets().primary());
      // Edges which aren't on a track (off grid edges) could be blockages to
      // other tracks!
      // TODO(aryap): We use the wire footprint because the full edge footprint
      // is unnecessarily high penalty: it's as wide as the widest via encaps on
      // either end. Until we can correctly represent the whole footprint with
      // just a polygon, this will do.
      auto footprint = EdgeWireFootprint(*edge);
      if (footprint) {
        AddBlockage(*footprint);
      }
    }

    std::vector<RoutingVertex*> spanned_vertices = edge->SpannedVertices();
    for (RoutingVertex *vertex : spanned_vertices) {
      vertex->installed_in_paths()[path].insert(edge);
    }
  }

  if (path->vertices().size() != path->edges().size() + 1) {
    std::stringstream ss;
    ss << "Path vertices and edges mismatched. There are "
       << path->edges().size() << " edges and "
       << path->vertices().size() << " vertices";
    return absl::InvalidArgumentError(ss.str());
  }

  size_t i = 0;
  RoutingEdge *edge = nullptr;
  path->vertices()[0]->set_available(false);
  while (i < path->edges().size()) {
    RoutingVertex *last_vertex = path->vertices()[i];
    RoutingVertex *next_vertex = path->vertices()[i + 1];
    RoutingEdge *edge = path->edges()[i];
    last_vertex->set_out_edge(edge);
    next_vertex->set_in_edge(edge);
    next_vertex->set_available(false);
    next_vertex->set_net(path->nets().primary());
    ++i;
  }

  for (RoutingVertex *vertex : path->vertices()) {
    InstallVertexInPath(vertex, path->nets().primary());
  }
  
  paths_.push_back(path);
  return absl::OkStatus();
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin, RoutingVertex *end) {
  return ShortestPath(
      begin,
      [=](RoutingVertex *v) { return v == end; },   // The target.
      nullptr,
      [](RoutingVertex *v) { return v->available(); },
      [](RoutingEdge *e) { return e->Available(); },
      true);
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin,
    const EquivalentNets &to_nets,
    RoutingVertex **discovered_target) {
  auto path = ShortestPath(
      begin,
      [&](RoutingVertex *v) {
        // Check that putting a via at this position doesn't conflict with vias
        // for other nets (since the encapsulating metal layers would
        // conflict):
        std::set<RoutingVertex*> neighbours = v->GetNeighbours();
        // ChangesEdge is a proxy for a vertex that might become a via.
        // NOTE: It's not the *same* as a vertex that will become a via, but
        // that isn't decided til RoutingPath has to export geometry :/
        for (RoutingVertex *neighbour : neighbours) {
          if (!neighbour->available() &&
              neighbour->ChangesEdge() &&
              !to_nets.Contains(neighbour->net())) {
            VLOG(16) << "(ShortestPath) Vertex " << v->centre()
                     << " not viable because a via wouldn't fit here";
            return false;
          }
        }
        return to_nets.Contains(v->net());
      },
      discovered_target,
      // Usable vertices are:
      [&](RoutingVertex *v) {
        return v->available() || (
            v->connectable_net() && to_nets.Contains(*v->connectable_net()));
      },
      // Usable edges are:
      [&](RoutingEdge *e) {
        if (e->Available()) return true;
        if (e->Blocked()) {
          VLOG(16) << "edge " << *e << " is blocked";
          return false;
        }
        if (e->EffectiveNet() && to_nets.Contains(*e->EffectiveNet())) {
          return true;
        } else {
          VLOG(16) << "cannot use edge " << *e << " for net "
                   << to_nets.primary();
        }
        return false;
      },
      false);   // Targets don't have to be 'usable', since we expect them to
                // already be used by the target net.
  // TODO(aryap): InstallPath obviates this.
  //if (path.ok()) {
  //  (*path)->set_encap_end_port(true);
  //}
  return path;
}

absl::StatusOr<RoutingPath*> RoutingGrid::ShortestPath(
    RoutingVertex *begin, 
    std::function<bool(RoutingVertex*)> is_target,
    RoutingVertex **discovered_target,
    std::function<bool(RoutingVertex*)> usable_vertex,
    std::function<bool(RoutingEdge*)> usable_edge,
    bool target_must_be_usable) {
  // FIXME(aryap): This is very bad.
  if (!usable_vertex(begin)) {
    return absl::NotFoundError("Start vertex for path is not available");
  }

  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  // Prefer consistent C++/STLisms over e.g. bool seen[vertices_.size()];
  std::vector<double> cost(vertices_.size());
  std::vector<bool> seen(vertices_.size());

  // Records the edges to follow backward to the start, forming the shortest
  // path. If RoutingEdge* is nullptr then this is invalid. The index into this
  // array is the index of the sink node, the entry gives the path back to the
  // source.
  // TODO(aryap): Apparently using pairs everywhere is bad style and I should
  // use structs:
  // https://google.github.io/styleguide/cppguide.html#Structs_vs._Classes
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  // We want the lowest value at the back of the array. But in a priority_queue,
  // we want the highest value at the start of the collection so that the
  // 'least' element is popped first (because that's how priority_queue works).
  auto vertex_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    return cost[a->contextual_index()] > cost[b->contextual_index()];
  };
  // All vertices sorted according to their cost.
  std::priority_queue<RoutingVertex*,
                      std::vector<RoutingVertex*>,
                      decltype(vertex_sort_fn)> queue(vertex_sort_fn);
  std::set<RoutingVertex*,
           bool (*)(RoutingVertex*, RoutingVertex*)> found_targets(
      RoutingVertex::Compare);

  size_t begin_index = begin->contextual_index();

  cost[begin_index] = 0;

  for (size_t i = 0; i < vertices_.size(); ++i) {
    RoutingVertex *vertex = vertices_[i];
    if (i != vertex->contextual_index()) {
      std::stringstream ss;
      ss << "Vertex " << i << " no longer matches its index "
         << vertex->contextual_index();
      return absl::InternalError(ss.str());
    }
    prev[i].second = nullptr;
    if (i == begin_index)
      continue;
    cost[i] = std::numeric_limits<double>::max();
    seen[i] = false;
  }

  queue.push(begin);
  seen[begin_index] = true;

  while (!queue.empty()) {
    // NOTE(aryap): CPU profiler says this is slow:
    //  CPUPROFILE=bfg.prof LD_PRELOAD="/usr/local/lib/libprofiler.so" ./bfg
    //  pprof --gv bfg bfg.prof
    // 
    // Have to re-sort the queue so that new cost changes take effect. (The
    // queue is already mostly sorted so an insertion would be fast.)
    // std::sort(queue.begin(), queue.end(), vertex_sort_fn);

    struct DebugVertexStatus {
      bool is_target;
      bool is_unusable_vertex;
    };

    RoutingVertex *current = queue.top();
    queue.pop();

    DebugVertexStatus status = {
      .is_target = false,
      .is_unusable_vertex = false
    };
#ifndef NDEBUG
    // NDEBUG is defined to disable C assertions and is set by cmake for
    // optimised builds by defaults. If we excuse that's forces us into a
    // double-negative, this makes a convenient debug-only post-iteration
    // report:
    absl::Cleanup report = [&]() {
      std::stringstream ss;
      ss << current->centre();
      if (status.is_target) {
        ss << " target";
      }
      if (status.is_unusable_vertex) {
        ss << " unusable_vertex";
      }
      ss << (current->available() ? " available" : " not_available");
      if (!current->net().empty()) {
        ss << " net:" << current->net();
      }
      if (current->connectable_net()) {
        ss << " connectable_net:" << *current->connectable_net();
      }
      VLOG(15) << ss.str();
    };
#endif  // NDEBUG

    if (target_must_be_usable) {
      // If the target must be usable for a valid route (e.g. point to point
      // routing from vertex to vertex), we ignore unusable nodes as possible
      // targets.
      if (!usable_vertex(current)) {
        status.is_unusable_vertex = true;
        continue;
      }
      if (is_target(current)) {
        status.is_target = true;
        found_targets.insert(current);
        continue;
      }
    } else {
      // If the target doesn't necessarily have to be usable, we check for a
      // valid target _before_ culling unusable nodes.
      if (is_target(current)) {
        status.is_target = true;
        found_targets.insert(current);
        continue;
      }
      if (!usable_vertex(current)) {
        status.is_unusable_vertex = true;
        continue;
      }
    }

    size_t current_index = current->contextual_index();

    for (RoutingEdge *edge : current->edges()) {
      if (!usable_edge(edge)) {
#ifndef NDEBUG
        VLOG(15) << *edge << " unusable_edge";
#endif  // NDEBUG
        continue;
      }

      // We don't know what direction we're using the edge in, and edges are
      // not directional per se, so pick the side that isn't the one we came in
      // on:
      // TODO(aryap): Maybe bake this into the RoutingEdge.
      RoutingVertex *next =
          edge->first() == current ? edge->second() : edge->first();

      size_t next_index = next->contextual_index();

      double next_cost = cost[current_index] + edge->cost() + next->cost();

      LOG_IF(FATAL, !std::isfinite(next_cost)) << "!";

      if (next_cost < cost[next_index]) {
        cost[next_index] = next_cost;
        prev[next_index] = std::make_pair(current_index, edge);

        // If we haven't seen this node before we should definitely visit it.
        if (!seen[next_index]) {
          queue.push(next);
          seen[next_index] = true;
        }
      }
    }
  }

  if (found_targets.empty()) {
    LOG(INFO) << "No usable targets found.";
    return absl::NotFoundError("No usable targets found.");
  }

  // Sort all of the found targets according to their final cost:
  std::vector<RoutingVertex*> sorted_targets(
      found_targets.begin(), found_targets.end());
  auto target_sort_fn = [&](RoutingVertex *a, RoutingVertex *b) {
    //double cost_to_complete = 
    return cost[a->contextual_index()] < cost[b->contextual_index()];
  };
  std::sort(sorted_targets.begin(), sorted_targets.end(), target_sort_fn);
  RoutingVertex *end_target = sorted_targets.front();
  size_t end_index = end_target->contextual_index();

  if (discovered_target)
    *discovered_target = end_target;

  std::deque<RoutingEdge*> shortest_edges;

  size_t last_index = prev[end_index].first;
  RoutingEdge *last_edge = prev[end_index].second;

  while (last_edge != nullptr) {
    LOG_IF(FATAL, (last_edge->first() != vertices_[last_index] &&
                   last_edge->second() != vertices_[last_index]))
        << "last_edge does not land back at source vertex";

    shortest_edges.push_front(last_edge);

    if (last_index == begin_index) {
      // We found our way back.
      break;
    }

    auto &last_entry = prev[last_index];
    last_index = last_entry.first;

    last_edge = last_entry.second;
  }

  if (shortest_edges.empty()) {
    return absl::InternalError("shortest_edges was empty?");
  } else if (shortest_edges.front()->first() != begin &&
             shortest_edges.front()->second() != begin) {
    LOG(ERROR) << "Did not find beginning vertex.";
    return absl::InternalError("Could not back-track to beginning vertex.");
  }

  RoutingPath *path = new RoutingPath(begin, shortest_edges, this);
  return path;
}

void RoutingGrid::ClearAllBlockages() {
  // Since these are vectors of unique_ptr, we just have to clear them to
  // invoke their destructors.
  rectangle_blockages_.clear();
  polygon_blockages_.clear();
}

RoutingGridBlockage<geometry::Rectangle> *RoutingGrid::AddBlockage(
    const geometry::Rectangle &rectangle,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  const geometry::Layer &layer = rectangle.layer();
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end())
    return nullptr;

  // TODO(aryap): RoutingTracks are equipped with min_separation, but
  // RoutingGridBlockages are not. padding is sometimes treated as a temporary
  // additional value and sometimes as the min_separation value.
  // RoutingGridBlockage has two explicit checks:
  //    BlocksWithoutPadding, meaning that the shapes overlap, and
  //    Blocks, meaning that the shapes come within the min_separation.
  // In RoutingGridBlockage "padding" is min_separation. Does it need to have
  // both?
  //
  // Or should RoutingGridBlockage be able to look up the min_separation on its
  // own? RoutingGridBlockage could easily do this, but then it would need a
  // handle to physical information:
  int64_t min_separation = physical_db_.Rules(layer).min_separation;

  // Create and save the blockage:
  RoutingGridBlockage<geometry::Rectangle> *blockage =
      new RoutingGridBlockage<geometry::Rectangle>(
          *this, rectangle, padding + min_separation);
  rectangle_blockages_.emplace_back(blockage);

  for (RoutingTrack *track : it->second) {
    if (is_temporary) {
      // TODO(aryap): Move the addition of the track temporary blockage into
      // the blockage 'AddChildTrackBlockage' function. That way managing
      // temporary track blockages as a result of RoutingGridBlockages is kept
      // together in one place.
      RoutingTrackBlockage *vertex_blockage = nullptr;
      RoutingTrackBlockage *edge_blockage = nullptr;
      track->AddTemporaryBlockage(rectangle,
                                  padding,
                                  rectangle.net(), 
                                  &vertex_blockage,
                                  &edge_blockage,
                                  blocked_vertices,
                                  blocked_edges);
      if (vertex_blockage) {
        blockage->AddChildTrackBlockage(track, vertex_blockage);
      }
      if (edge_blockage) {
        blockage->AddChildTrackBlockage(track, edge_blockage);
      }
    } else {
      // Add permanent blockage.
      track->AddBlockage(rectangle, padding, rectangle.net(), nullptr, nullptr);
    }
  }

  ApplyBlockage(*blockage, is_temporary, blocked_vertices);
  return blockage;
}

RoutingGridBlockage<geometry::Polygon> *RoutingGrid::AddBlockage(
    const geometry::Polygon &polygon,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices) {
  const geometry::Layer &layer = polygon.layer();

  int64_t min_separation = physical_db_.Rules(layer).min_separation;

  // Create and save the blockage:
  RoutingGridBlockage<geometry::Polygon> *blockage =
      new RoutingGridBlockage<geometry::Polygon>(
          *this, polygon, padding + min_separation);
  polygon_blockages_.emplace_back(blockage);

  // Find tracks on the blockage layer, if any.
  auto it = tracks_by_layer_.find(layer);
  if (it != tracks_by_layer_.end()) {
    if (is_temporary) {
      // TODO(aryap): Support polygons on tracks because otherwise this is gonna
      // get painful:
      VLOG(12) << "Temporary blockage is a Polygon which tracks don't "
               << "support: " << polygon << ")";
    } else {
      for (RoutingTrack *track : it->second) {
        track->AddBlockage(polygon, padding, polygon.net());
      }
    }
  }

  ApplyBlockage(*blockage, is_temporary, blocked_vertices);
  return blockage;
}

void RoutingGrid::RemoveUnavailableVertices() {
  for (auto &entry : available_vertices_by_layer_) {
    std::vector<RoutingVertex*> &available = entry.second;
    for (auto it = available.begin(); it != available.end();) {
      RoutingVertex *vertex = *it;
      if (!vertex->available()) {
        it = available.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void RoutingGrid::ExportEdgesAsRectangles(
    const std::string &layer, bool available_only, Layout *layout) const {
  layout->SetActiveLayerByName(layer);

  const int64_t kPadding = 2;

  for (const auto &entry : tracks_by_layer_) {
    const geometry::Layer &track_layer = entry.first;
    for (const RoutingTrack *track : entry.second) {
      track->ExportEdgesAsRectangles(layer, available_only, layout);
    }
  }

  for (RoutingEdge *edge : off_grid_edges_) {
    if (available_only && edge->Blocked())
      continue;
    auto rectangle = edge->AsRectangle(kPadding);
    if (!rectangle)
      continue;
    layout->AddRectangle(*rectangle);
  }
}

void RoutingGrid::ExportVerticesAsSquares(
    const std::string &layer, bool available_only, Layout *layout) const {
  layout->SetActiveLayerByName(layer);
  for (RoutingVertex *vertex : vertices_) {
    if (!available_only || vertex->available()) {
      layout->AddSquare(vertex->centre(), 10);
    }
  }
}

absl::Status RoutingGrid::AddRoutingViaInfo(
    const Layer &lhs,
    const Layer &rhs,
    const RoutingViaInfo &info) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  // Order first and second.
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  if (via_infos_.find(first) != via_infos_.end() &&
      via_infos_[first].find(second) != via_infos_[first].end()) {
    std::stringstream ss;
    ss << "Attempt to specify RoutingViaInfo for layers " << first << " and "
       << second << " again.";
    return absl::InvalidArgumentError(ss.str());
  }
  via_infos_[first][second] = info;
  return absl::OkStatus();
}

const RoutingViaInfo &RoutingGrid::GetRoutingViaInfoOrDie(
    const Layer &lhs, const Layer &rhs) const {
  auto via_info = GetRoutingViaInfo(lhs, rhs);
  LOG_IF(FATAL, !via_info)
      << "No known connection between layer " << lhs << " and layer " << rhs;
  return *via_info;
}

std::optional<std::reference_wrapper<const RoutingViaInfo>>
RoutingGrid::GetRoutingViaInfo(const Layer &lhs, const Layer &rhs) const {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;

  const auto first_it = via_infos_.find(first);
  if (first_it == via_infos_.end())
    return std::nullopt;
  const std::map<Layer, RoutingViaInfo> &inner_map = first_it->second;
  const auto second_it = inner_map.find(second);
  if (second_it == inner_map.end())
    return std::nullopt;
  return second_it->second;
}

absl::Status RoutingGrid::AddRoutingLayerInfo(const RoutingLayerInfo &info) {
  const Layer &layer = info.layer();
  auto layer_info_it = routing_layer_info_.find(layer);
  if (layer_info_it != routing_layer_info_.end()) {
    std::stringstream ss;
    ss << "Duplicate routing layer info: " << layer;
    return absl::InvalidArgumentError(ss.str());
  }
  routing_layer_info_.insert({layer, info});
  return absl::OkStatus();
}

std::optional<std::reference_wrapper<const RoutingLayerInfo>>
    RoutingGrid::GetRoutingLayerInfo(const geometry::Layer &layer) const {
  auto it = routing_layer_info_.find(layer);
  if (it == routing_layer_info_.end())
    return std::nullopt;
  return it->second;
}

const RoutingLayerInfo &RoutingGrid::GetRoutingLayerInfoOrDie(
    const geometry::Layer &layer) const {
  auto info = GetRoutingLayerInfo(layer);
  LOG_IF(FATAL, !info)
      << "Could not find routing information for layer " << layer;
  return *info;
}

void RoutingGrid::AddTrackToLayer(
    RoutingTrack *track, const geometry::Layer &layer) {
  // Create the first vector of tracks.
  auto it = tracks_by_layer_.find(layer);
  if (it == tracks_by_layer_.end()) {
    tracks_by_layer_.insert({layer, {track}});
    return;
  }
  it->second.push_back(track);
}

bool RoutingGrid::PointsAreTooCloseForVias(
    const geometry::Layer &shared_layer,
    const geometry::Point &lhs,
    const geometry::Layer &lhs_connectee,
    const geometry::Point &rhs,
    const geometry::Layer &rhs_connectee) const {
  int64_t separation = static_cast<int64_t>(lhs.L2DistanceTo(rhs));

  auto maybe_lhs_via = GetRoutingViaInfo(shared_layer, lhs_connectee);
  if (!maybe_lhs_via)
    return false;

  auto maybe_rhs_via = GetRoutingViaInfo(shared_layer, rhs_connectee);
  if (!maybe_rhs_via)
    return false;

  auto maybe_shared_layer_info = GetRoutingLayerInfo(shared_layer);
  if (!maybe_shared_layer_info)
    return false;

  const RoutingViaInfo &lhs_via = *maybe_lhs_via;
  const RoutingViaInfo &rhs_via = *maybe_rhs_via;
  const RoutingLayerInfo &shared_layer_info = *maybe_shared_layer_info;
  
  int64_t lhs_max_via_half_width = lhs_via.MaxViaSide() / 2;
  int64_t lhs_max_via_overhang = lhs_via.MaxOverhang();

  int64_t rhs_max_via_half_width = rhs_via.MaxViaSide() / 2;
  int64_t rhs_max_via_overhang = rhs_via.MaxOverhang();

  int64_t min_separation = shared_layer_info.min_separation();

  int64_t required =
      lhs_max_via_half_width + lhs_max_via_overhang +
      min_separation +
      rhs_max_via_half_width + rhs_max_via_overhang;

  if (separation < required) {
    LOG(INFO)
        << "Via between " << shared_layer << " and " << lhs_connectee
        << " requires at least " << required << " units to via between "
        << shared_layer << " and " << rhs_connectee
        << ", but there are only " << separation << " units; therefore "
        << lhs << " and " << rhs << " are too close together.";
    return true;
  }
  return false;
}

bool RoutingGrid::VerticesAreTooCloseForVias(
    const RoutingVertex &lhs, const RoutingVertex &rhs) const {
  const std::set<geometry::Layer> &lhs_layers = lhs.connected_layers();
  const std::set<geometry::Layer> &rhs_layers = rhs.connected_layers();
  std::set<geometry::Layer> shared_layers =
      RoutingVertex::CommonLayers(lhs, rhs);
  if (shared_layers.empty())
    return false;

  // Shortcuts for on-grid vertices:
  if (lhs.horizontal_track() == rhs.horizontal_track() &&
      lhs.grid_position_x() &&
      rhs.grid_position_x()) {
    // They might be horizontal neighbours:
    size_t diff = std::max(*lhs.grid_position_x(), *rhs.grid_position_x()) -
        std::min(*lhs.grid_position_x(), *rhs.grid_position_x());
    return diff == 1U;
    // (This is the same as:
    //  int64_t diff = std::abs(static_cast<int64_t>(*lhs.grid_position_x()) -
    //      static_cast<int64_t>(*rhs.grid_position_x()));
    //  return diff == 1;
    // )
  } else if (lhs.vertical_track() == rhs.vertical_track() &&
             lhs.grid_position_y() &&
             rhs.grid_position_y()) {
    // They might be vertical neighbours:
    size_t diff = std::max(*lhs.grid_position_y(), *rhs.grid_position_y()) -
        std::min(*lhs.grid_position_y(), *rhs.grid_position_y());
    return diff == 1U;
  }

  // Check that the distance between the two vertices can accommodate vias on
  // each of the shared layers. To do this we have to get the rules for the vias
  // placeable on each `shared_layer`, which means getting the rules for via
  // encapsulation and such. That means means we need to get a handle to all
  // vias between `shared_layer` and every connectable layer, then figure out
  // if the rules for that via would cause a violation.
  for (const geometry::Layer &source_layer : shared_layers) {
    for (const geometry::Layer &lhs_connectee : lhs.connected_layers()) {
      if (lhs_connectee == source_layer)
        continue;
      for (const geometry::Layer &rhs_connectee : rhs.connected_layers()) {
        if (rhs_connectee == source_layer)
          continue;
        if (PointsAreTooCloseForVias(source_layer,
                                     lhs.centre(),
                                     lhs_connectee,
                                     rhs.centre(),
                                     rhs_connectee)) {
          return true;
        }
      }
    }
  }

  return false;
}

std::vector<RoutingGridBlockage<geometry::Rectangle>*> RoutingGrid::AddBlockage(
    const geometry::Port &port,
    int64_t padding,
    bool is_temporary,
    std::set<RoutingVertex*> *blocked_vertices,
    std::set<RoutingEdge*> *blocked_edges) {
  std::vector<RoutingGridBlockage<geometry::Rectangle>*> blockages;
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      layer_access = physical_db_.FindReachableLayersByPinLayer(
          port.layer());
  for (const auto &entry : layer_access) {
    const geometry::Layer &access_layer = entry.first;
    const std::set<geometry::Layer> &reachable_by_one_via =
        entry.second;
    for (const auto &footprint_layer : reachable_by_one_via) {
      // Not all reachable layers are actually usable by the routing grid.
      // Instead of making ViaFootprint handle this, we just check:
      if (!GetRoutingViaInfo(access_layer, footprint_layer)) {
        continue;
      }

      // If the footprint layer has an associated routing track direction, we
      // use that to determine the footprint.
      std::optional<RoutingTrackDirection> access_direction;
      auto routing_info = GetRoutingLayerInfo(footprint_layer);
      if (routing_info) {
        access_direction = routing_info->get().direction();
      }

      auto pin_projection = ViaFootprint(port.centre(),
                                         access_layer,
                                         footprint_layer,
                                         0,
                                         access_direction);
      if (!pin_projection) {
        continue;
      }
      int64_t min_separation =
          physical_db_.Rules(footprint_layer).min_separation;

      pin_projection->set_layer(footprint_layer);
      pin_projection->set_net(port.net());

      RoutingGridBlockage<geometry::Rectangle> *pin_blockage =
          AddBlockage(*pin_projection,
                      padding,
                      true,     // Temporary blockage.
                      blocked_vertices,
                      blocked_edges);
      if (pin_blockage) {
        blockages.push_back(pin_blockage);
      }
    }
  }
  return blockages;
}

void RoutingGrid::SetUpTemporaryBlockages(
    const geometry::ShapeCollection &avoid,
    TemporaryBlockageInfo *blockage_info) {
  for (const auto &rectangle : avoid.rectangles()) {
    RoutingGridBlockage<geometry::Rectangle> *blockage = AddBlockage(
        *rectangle,
        0,      // No extra padding on shapes.
        true,   // Temporary.
        &blockage_info->blocked_vertices,
        &blockage_info->blocked_edges);
  }
  for (const auto &polygon : avoid.polygons()) {
    //RoutingGridBlockage<geometry::Polygon> *blockage = AddBlockage(
    //    *polygon,
    //    0,      // No extra padding on shapes.
    //    true,   // Temporary.
    //    &blockage_info->blocked_vertices);
    geometry::Rectangle bounding_box = polygon->GetBoundingBox();
    VLOG(12) << "Temporary blockage is a Polygon which tracks don't "
             << "support, using the bounding box: " << bounding_box
             << " (for: " << *polygon << ")";
    bounding_box.set_net(polygon->net());
    RoutingGridBlockage<geometry::Rectangle> *blockage = AddBlockage(
        bounding_box,
        0,      // No extra padding on shapes.
        true,   // Temporary.
        &blockage_info->blocked_vertices,
        &blockage_info->blocked_edges);
  }
  for (const auto &poly_line : avoid.poly_lines()) {
    LOG(ERROR) << "Unimplemented: not sure how to add PolyLines as blockages "
               << "to routing grid: " << *poly_line;
  }
  for (const auto &port : avoid.ports()) {
    std::vector<RoutingGridBlockage<geometry::Rectangle>*> blockages =
        AddBlockage(*port,
                    0,
                    true,
                    &blockage_info->blocked_vertices,
                    &blockage_info->blocked_edges);
    blockage_info->pin_blockages.insert(
        blockage_info->pin_blockages.end(),
        blockages.begin(),
        blockages.end());
  }
  VLOG(13) << "avoiding "
           << blockage_info->blocked_vertices.size() << " vertices and "
           << blockage_info->blocked_edges.size() << " edges";
}


void RoutingGrid::TearDownTemporaryBlockages(
    const TemporaryBlockageInfo &blockage_info) {
  for (RoutingVertex *const vertex : blockage_info.blocked_vertices) {
    vertex->set_available(true);
  }
  for (RoutingEdge *const edge : blockage_info.blocked_edges) {
    // This should clear any used nets and unblock the edge.
    edge->ResetTemporaryStatus();
  }
  for (RoutingGridBlockage<geometry::Rectangle> *const blockage :
          blockage_info.pin_blockages) {
    ForgetBlockage(blockage);
  }
}

std::vector<RoutingGrid::CostedLayer> RoutingGrid::LayersReachableByVia(
    const geometry::Layer &from_layer) const {
  std::vector<RoutingGrid::CostedLayer> reachable;

  // Greater (in the std::less sense) layers are found directly:
  auto it = via_infos_.find(from_layer);
  if (it != via_infos_.end()) {
    for (const auto &inner : it->second) {
      const geometry::Layer &to = inner.first;
      double cost = inner.second.cost();
      reachable.push_back({to, cost});
    }
  }

  // Lesser layers are found indirectly:
  for (const auto &outer : via_infos_) {
    const geometry::Layer &maybe_reachable = outer.first;
    if (maybe_reachable == from_layer)
      continue;
    for (const auto &inner : outer.second) {
      if (inner.first == from_layer) {
        double cost = inner.second.cost();
        reachable.push_back({maybe_reachable, cost});
      }
    }
  }
  return reachable;
}

std::optional<double> RoutingGrid::FindViaStackCost(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  auto maybe_stack = FindViaStack(lhs, rhs);
  if (!maybe_stack)
    return std::nullopt;
  const std::vector<RoutingViaInfo> &via_stack = *maybe_stack;
  double total_cost = 0.0;
  for (const RoutingViaInfo &info : via_stack) {
    total_cost += info.cost();
  }
  return total_cost;
}

std::optional<std::vector<RoutingViaInfo>> RoutingGrid::FindViaStack(
    const geometry::Layer &lhs, const geometry::Layer &rhs) const {
  std::vector<RoutingViaInfo> via_stack;
  if (lhs == rhs) {
    return via_stack;
  }

  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const geometry::Layer &from = ordered_layers.first;
  const geometry::Layer &to = ordered_layers.second;

  // Dijkstra's shortest path but over the graph of via connectivity.

  // Best-known cost so far to get to the given layer from `from`.
  std::map<geometry::Layer, double> cost;
  std::map<geometry::Layer, geometry::Layer> previous;
  std::set<geometry::Layer> seen;

  // We can't easily enumerate all known layers from our given structures, so we
  // make the various bookkeeping sparse:
  auto get_cost = [&](const geometry::Layer &layer) {
    auto it = cost.find(layer);
    return it == cost.end() ? std::numeric_limits<double>::max() : it->second;
  };
  auto layer_sort_fn = [&](const geometry::Layer &from,
                           const geometry::Layer &to) {
    return get_cost(from) > get_cost(to);
  };
  std::priority_queue<geometry::Layer,
                      std::vector<geometry::Layer>,
                      decltype(layer_sort_fn)> queue(layer_sort_fn);

  cost[from] = 0.0;
  queue.push(from);

  while (!queue.empty()) {
    const geometry::Layer &current = queue.top();
    queue.pop();

    if (current == to) {
      break;
    }

    std::vector<CostedLayer> reachable = LayersReachableByVia(current);

    for (const auto &next : reachable) {
      const geometry::Layer &next_layer = next.layer;
      double next_cost = get_cost(current) + next.cost;
      if (next_cost < get_cost(next_layer)) {
        cost[next_layer] = next_cost;
        previous[next_layer] = current;

        if (seen.find(next_layer) == seen.end()) {
          queue.push(next_layer);
          seen.insert(next_layer);
        }
      }
    }
  }

  // Walk backwards to find the 'shortest path'.
  if (previous.find(to) == previous.end()) {
    // No path.
    return std::nullopt;
  }

  // [to, intermediary, other_intermediary, from]
  std::vector<geometry::Layer> layer_stack;
  auto it = previous.find(to);
  layer_stack.push_back(to);
  while (it != previous.end()) {
    geometry::Layer next_previous = it->second;
    layer_stack.push_back(next_previous);
    if (next_previous == from) {
      break;
    }
    it = previous.find(next_previous);
  }
  if (layer_stack.back() != from) {
    // No path found.
    return std::nullopt;
  }
  
  for (size_t i = layer_stack.size() - 1; i > 0; --i) {
    const geometry::Layer &rhs = layer_stack.at(i);
    const geometry::Layer &lhs = layer_stack.at(i - 1);
    const RoutingViaInfo &via_info = GetRoutingViaInfoOrDie(lhs, rhs);
    via_stack.push_back(via_info);
  }
  return via_stack;
}

PolyLineCell *RoutingGrid::CreatePolyLineCell() const {
  std::unique_ptr<PolyLineCell> cell(new PolyLineCell());
  for (RoutingPath *path : paths_) {
    path->ToPolyLinesAndVias(&cell->poly_lines(), &cell->vias());
  }
  return cell.release();
}

Layout *RoutingGrid::GenerateLayout() const {
  PolyLineInflator inflator(physical_db_);
  std::unique_ptr<PolyLineCell> grid_lines(CreatePolyLineCell());
  std::unique_ptr<bfg::Layout> grid_layout(
      inflator.Inflate(*this, *grid_lines));
  return grid_layout.release();
}

absl::Status RoutingGrid::AddRoutingGridGeometry(
    const geometry::Layer &lhs, const geometry::Layer &rhs,
    const RoutingGridGeometry &grid_geometry) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  if (grid_geometry_by_layers_.find(first) !=
          grid_geometry_by_layers_.end() &&
      grid_geometry_by_layers_[first].find(second) !=
          grid_geometry_by_layers_[first].end()) {
    std::stringstream ss;
    ss << "Attempt to add RoutingGridGeometry for layers " << first << " and "
       << second << " again.";
    return absl::InvalidArgumentError(ss.str());
  }
  grid_geometry_by_layers_[first][second] = grid_geometry;
  return absl::OkStatus();
}

std::optional<std::reference_wrapper<RoutingGridGeometry>>
    RoutingGrid::GetRoutingGridGeometry(
        const geometry::Layer &lhs, const geometry::Layer &rhs) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  auto first_it = grid_geometry_by_layers_.find(first);
  if (first_it == grid_geometry_by_layers_.end())
    return std::nullopt;
  std::map<Layer, RoutingGridGeometry> &inner_map = first_it->second;
  auto second_it = inner_map.find(second);
  if (second_it == inner_map.end())
    return std::nullopt;
  return second_it->second;
}

std::vector<RoutingGridGeometry*>
RoutingGrid::FindRoutingGridGeometriesUsingLayer(
    const geometry::Layer &layer) {
  std::vector<RoutingGridGeometry*> grid_geometries;
  for (auto &entry : grid_geometry_by_layers_) {
    const Layer &first = entry.first;
    for (auto &inner : entry.second) {
      const Layer &second = inner.first;
      if (first != layer && second != layer)
        continue;
      grid_geometries.push_back(&inner.second);
    }
  }
  return grid_geometries;
}

} // namespace bfg
