#include "routing_path.h"

#include <algorithm>
#include <deque>
#include <vector>
#include <memory>
#include <optional>

#include <absl/strings/str_join.h>
#include <absl/status/status.h>

#include "geometry/radian.h"
#include "geometry/poly_line.h"
#include "geometry/port.h"
#include "geometry/radian.h"
#include "abstract_via.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_vertex_collector.h"
#include "routing_track.h"
#include "physical_properties_database.h"

namespace bfg {

RoutingPath::RoutingPath(
    RoutingVertex *start,
    const std::deque<RoutingEdge*> edges,
    RoutingGrid *routing_grid)
    : edges_(edges.begin(), edges.end()),
      start_port_(nullptr),
      end_port_(nullptr),
      encap_start_port_(false), 
      encap_end_port_(false),
      legalised_(false),
      routing_grid_(routing_grid) {
  vertices_.push_back(start);
  RoutingVertex *last = start;
  for (RoutingEdge *edge : edges) {
    RoutingVertex *next_vertex =
        edge->first() == last ? edge->second() : edge->first();
    vertices_.push_back(next_vertex);
    last = next_vertex;
  }
}

// TODO(aryap): There are different rules for overhanging from the layer above
// and below. RoutingViaInfo now differentiates these, so we should use them.
RoutingPath::BulgeDimensions RoutingPath::GetBulgeDimensions(
    const RoutingViaInfo &routing_via_info) {
  return BulgeDimensions {
    .width = routing_via_info.MaxEncapWidth(),
    .length = routing_via_info.MaxEncapLength()
  };
}

double RoutingPath::Cost() const {
  // TODO(aryap): Should there be costs for start/end vias in here?
  double cost = 0.0;
  for (RoutingVertex *vertex : vertices_) {
    cost += vertex->cost();
  }
  for (RoutingEdge *edge : edges_) {
    cost += edge->cost();
  }
  return cost;
}

geometry::Layer RoutingPath::DetermineLayerForAbbreviation(
    size_t starting_index,
    RoutingVertex *vertex_b,
    RoutingVertex *vertex_d,
    RoutingTrack *track_c) {
  if (!routing_grid_->VerticesAreTooCloseForVias(*vertex_b, *vertex_d)) {
    if (track_c) {
      return track_c->layer();
    }
    // If track_c is nullptr, because the long edge isn't on the grid, find a
    // layer that vertices b and d have in common.
    std::set<geometry::Layer> shared_layers =
        RoutingVertex::CommonLayers(*vertex_b, *vertex_d);
    LOG_IF(FATAL, shared_layers.empty()) << "No layers in common?!";
    return *shared_layers.begin();
  }
  if (starting_index > 0) {
    // Use the in-bound edge's layer.
    return edges_[starting_index - 1]->EffectiveLayer();
  }
  if (starting_index < edges_.size() - 3) {
    // Use the outbound edge's layer.
    return edges_[starting_index + 3]->EffectiveLayer();
  }
  // As a fallback, use the current edge's layer.
  return edges_[starting_index]->EffectiveLayer();
}

std::optional<std::pair<int64_t, int64_t>>
RoutingPath::GetOverlapOrSkipAbbreviation(size_t starting_index) {
  RoutingEdge *current_edge = edges_[starting_index];
  RoutingEdge *intervening_edge = edges_[starting_index + 1];
  RoutingEdge *next_edge = edges_[starting_index + 2];

  RoutingVertex *first = vertices_[starting_index];
  RoutingVertex *fourth = vertices_[starting_index + 3];

  DCHECK(starting_index < vertices_.size() - 3);

  if (!first->IsOffGrid() && !fourth->IsOffGrid()) {
    return std::nullopt;
  }

  // Only apply abbreviations to parllel edges that are 1x pitch apart:
  int64_t pitch = routing_grid_->GetRoutingLayerInfoOrDie(
      current_edge->EffectiveLayer()).pitch();
  int64_t separation = std::llround(intervening_edge->Length());
  // NOTE(aryap): If extending this to allow abbreviations across more than 1
  // pitch, note that the newly created off-grid edge must be added as
  // blockage to all surrounding tracks on its layer.
  if (separation > pitch) {
    LOG(INFO) << "Will not abbreviate overlap with separation "
              << separation;
    return std::nullopt;
  }

  if (next_edge->Direction() != current_edge->Direction()) {
    LOG(WARNING)
        << "This is weird: alternating edges not in the same direction?";
    return std::nullopt;
  }

  // Find overlap:
  std::optional<geometry::Line> current_line = current_edge->AsLine();
  std::optional<geometry::Line> next_line = next_edge->AsLine();
  if (!current_line || !next_line) {
    return std::nullopt;
  }

  RoutingTrackDirection axis_direction = current_edge->Direction();
  double axis_angle = RoutingTrack::DirectionToAngle(axis_direction);
  double off_axis_angle = RoutingTrack::DirectionToAngle(
      RoutingTrack::OrthogonalDirectionTo(axis_direction));

  return geometry::Line::OverlappingProjectionOnAxis(
      *current_line, *next_line, axis_angle);
}

RoutingEdge *RoutingPath::MaybeMakeAbbreviatingEdge(
    RoutingVertex *bridging_vertex,
    RoutingVertex *off_host_track,
    const geometry::Layer &target_layer) {
  // Try to add edge between the off_host_vertex and the track. Check
  // RoutingGrid to see if this is a valid edge.
  RoutingEdge *new_edge = new RoutingEdge(bridging_vertex, off_host_track);
  new_edge->set_layer(target_layer);

  auto valid = routing_grid_->ValidAgainstKnownBlockages(*new_edge, nets_);
  if (!valid.ok()) {
    LOG(INFO) << "Invalid off grid edge between "
              << bridging_vertex->centre()
              << " and " << off_host_track->centre() << ": "
              << valid.message();
    // TODO(aryap): Don't think this gets added to RoutingGrid, only the
    // track.
    routing_grid_->RemoveVertex(bridging_vertex, true);
    delete new_edge;
    return nullptr;
  }

  bridging_vertex->AddEdge(new_edge);
  off_host_track->AddEdge(new_edge);
  routing_grid_->AddOffGridEdge(new_edge);
  return new_edge;
}

void RoutingPath::InstallAbbreviatingJog(
    size_t starting_index,
    bool new_edge_first,
    RoutingVertex *bridging_vertex,
    RoutingEdge *new_edge,
    RoutingEdge *jog) {
  RoutingEdge *current_edge = edges_[starting_index];
  RoutingEdge *intervening_edge = edges_[starting_index + 1];
  RoutingEdge *next_edge = edges_[starting_index + 2];
  RoutingVertex *second = vertices_[starting_index + 1];
  RoutingVertex *third = vertices_[starting_index + 2];

  // Remove second, third vertices; remove their edges; return true to
  // indicate that the operation occurred.
  // C++20 has std::erase!
  //
  // Also, since the positions of these entries are known we could just use
  // those to do the erasure!
  vertices_.erase(std::remove(vertices_.begin(), vertices_.end(), second),
                  vertices_.end());
  vertices_.erase(std::remove(vertices_.begin(), vertices_.end(), third),
                  vertices_.end());
  edges_.erase(std::remove(edges_.begin(), edges_.end(), current_edge),
               edges_.end());
  edges_.erase(std::remove(edges_.begin(), edges_.end(), intervening_edge),
               edges_.end());
  edges_.erase(std::remove(edges_.begin(), edges_.end(), next_edge),
               edges_.end());

  if (new_edge_first) {
    edges_.insert(edges_.begin() + starting_index, new_edge);
    edges_.insert(edges_.begin() + starting_index + 1, jog);
  } else {
    edges_.insert(edges_.begin() + starting_index, jog);
    edges_.insert(edges_.begin() + starting_index + 1, new_edge);
  }
  vertices_.insert(vertices_.begin() + starting_index + 1, bridging_vertex);
}

bool RoutingPath::MaybeAbbreviate(size_t starting_index) {
  DCHECK(starting_index < vertices_.size() - 3);

  RoutingEdge *current_edge = edges_[starting_index];
  RoutingEdge *intervening_edge = edges_[starting_index + 1];
  RoutingEdge *next_edge = edges_[starting_index + 2];

  RoutingVertex *first = vertices_[starting_index];
  RoutingVertex *second = vertices_[starting_index + 1];
  RoutingVertex *third = vertices_[starting_index + 2];
  RoutingVertex *fourth = vertices_[starting_index + 3];

  auto overlap = GetOverlapOrSkipAbbreviation(starting_index);
  if (!overlap) {
    return false;
  }
  LOG(INFO) << "Overlap detected " << overlap->first << ", "
            << overlap->second << ": " << *current_edge << " " << *next_edge;

  RoutingTrackDirection axis_direction = current_edge->Direction();
  int64_t projection_first = RoutingTrack::ProjectOntoAxis(
      first->centre(), axis_direction);
  int64_t projection_second = RoutingTrack::ProjectOntoAxis(
      second->centre(), axis_direction);
  int64_t projection_third = RoutingTrack::ProjectOntoAxis(
      third->centre(), axis_direction);
  int64_t projection_fourth = RoutingTrack::ProjectOntoAxis(
      fourth->centre(), axis_direction);

  // By construction we expect that 'second' and 'third' are connected by an
  // edge perpendicular to those connecting (first, second) and (third,
  // fourth). Any overlap must therefore include the 'second' and 'third'
  // points. We also assume that if an overlap occurs, it will be up to where
  // 'first' or 'fourth' falls. But for sanity will check this condition.
  //
  // For the geometryic meaning of the following points, refer to this
  // diagram:
  //                     +     +     +     +     +
  //                              a  |
  //      host track --> +-----+--x--+d    +     +      v
  //                     |                              | separation
  //  <--axis direction->+-----+--+b +     +     +---   ^
  //                              |              |
  //  alternative host track -->  +        +-----+
  //
  // Or:
  //                     +     +     +     +
  //                              a     +
  //      host track --> +-----+-----x--+d +      v
  //                     |                        | separation
  //  <--axis direction->+-----+-----+b    +---   ^
  //                                 |     |
  //  alternative host track ------> +e
  //
  // This is complicated because we don't know which order we're iterating
  // over the edges in, and we don't know which way the lines extend beyond
  // the overlapping section.
  int64_t on_axis_a = 0;
  if (overlap->first == projection_second &&
      overlap->first == projection_third) {
    LOG_IF(FATAL, overlap->second == projection_second &&
                  overlap->second == projection_third)
        << "Assumed overlap would never be between two equal-length edges";
    on_axis_a = overlap->second;
  } else if (overlap->second == projection_second &&
             overlap->second == projection_third) {
    on_axis_a = overlap->first;
  } else {
    LOG(FATAL) << "I asserted this was never the case!";
  }

  double axis_angle = RoutingTrack::DirectionToAngle(axis_direction);
  RoutingTrackDirection normal_direction =
      RoutingTrack::OrthogonalDirectionTo(axis_direction);
  double normal_angle = RoutingTrack::DirectionToAngle(normal_direction);

  geometry::Point point_a;
  // The host of the jog from a to do or a to b.
  RoutingTrack *host_track = nullptr;
  RoutingVertex *vertex_b = nullptr;
  RoutingVertex *vertex_d = nullptr;
  RoutingVertex *on_host_track = nullptr;   // d or b
  RoutingVertex *off_host_track = nullptr;  // d or b
  bool new_edge_first = false;

  auto assign_labels = [&](RoutingVertex *lhs, RoutingVertex *rhs) {
    vertex_b = lhs;
    vertex_d = rhs;
    point_a.AddComponents(on_axis_a, axis_angle);
    std::vector<RoutingTrack*> axis_tracks =
        rhs->TracksInDirection(axis_direction);
    if (!axis_tracks.empty()) {
      host_track = axis_tracks.front();
      on_host_track = rhs;
      off_host_track = lhs;
    } else {
      std::vector<RoutingTrack*> normal_tracks =
          lhs->TracksInDirection(normal_direction);
      LOG_IF(FATAL, normal_tracks.empty())
          << "Need vertex d to have an axis-oriented track or vertex b to have "
          << "a perpendicular one";
      // The alternate host track.
      host_track = normal_tracks.front();
      on_host_track = lhs;
      off_host_track = rhs;
    }
    int64_t normal_projection =  RoutingTrack::ProjectOntoAxis(
        vertex_d->centre(), normal_direction);
    point_a.AddComponents(normal_projection, normal_angle);
  };


  if (std::abs(projection_first - on_axis_a) <
      std::abs(projection_fourth - on_axis_a)) {
    assign_labels(first, fourth);
    new_edge_first = true;
  } else {
    assign_labels(fourth, first);
  }
  LOG(INFO) << "Trying new edge from " << vertex_b->centre()
            << " to " << point_a;

  // Try to add new routing vertex to host_track; since this is called before
  // InstallPath() finishes, none of the other edges/vertices for this path
  // should have been used and we don't need to do any additional bookkeeping.
  //
  // In some cases it is necessary to revert to the layer of the edges on
  // either side of the overlap. This is mostly the case when vertex_b and
  // vertex_d are too close together for vias.
  geometry::Layer target_layer = DetermineLayerForAbbreviation(
      starting_index, vertex_b, vertex_d, host_track);
  RoutingVertex *bridging_vertex = host_track->CreateNewVertexAndConnect(
      *routing_grid_, point_a, target_layer, nets_);
  if (!bridging_vertex) {
    LOG(INFO) << "Could not abbreviate: " << point_a << " not available on "
              << *host_track;
    return false;
  }

  RoutingEdge *new_edge = MaybeMakeAbbreviatingEdge(
      bridging_vertex, off_host_track, target_layer);
  if (!new_edge) {
    return false;
  }

  // Connect bridging_vertex with vertex b or d, whichever has landed on the
  // host track.
  RoutingEdge *jog = host_track->GetEdgeBetween(bridging_vertex, on_host_track);
  LOG_IF(FATAL, !jog) << "This edge should definitely have been made";

  // FIXME(aryap): We should explicitly fix jog to the same layer as the
  // new_edge (target_layer). We're currently relying on the other peephole
  // optimisations to skip the vias around it.

  InstallAbbreviatingJog(starting_index,
                         new_edge_first,
                         bridging_vertex,
                         new_edge,
                         jog);

  return true;
}

// Remove reundant direction switching.
bool RoutingPath::AbbreviateOnce() {
  if (vertices_.size() <= 3) {
    return false;
  }
  for (size_t i = 0; i < vertices_.size() - 3; ++i) {
    MaybeAbbreviate(i);
  }

  return false;
}

void RoutingPath::MergeConsecutiveEdgesOnSameTrack() {
  if (edges_.empty()) {
    return;
  }
  RoutingEdge *last_edge = edges_[0];

  size_t i = 0;
  for (size_t i = 1; i < edges_.size(); ++i) {
    RoutingEdge *edge = edges_[i];

    if (!edge->track() || edge->track() != last_edge->track()) {
      last_edge = edge;
      continue;
    }

    // last_edge and edge must be replaced with the edge that spans (previous,
    // next);
    RoutingVertex *previous = vertices_[i - 1];
    RoutingVertex *next = vertices_[i + 1];

    RoutingEdge *replacement = edge->track()->GetEdgeBetween(previous, next);
    if (!replacement) {
      LOG(WARNING)
          << "These consecutive edges cannot be replaced by single spanning "
          << "edge since there is no edge between " << previous->centre()
          << " and " << next->centre() << " on " << *edge->track();
      continue;
    }

    LOG(INFO) << "Merging consecutive edges on same track: "
              << last_edge->Describe() << ", "
              << edge->Describe();

    // NOTE(aryap): This is buggy. It seems to work but I'm not sure I
    // understand why, since it doesn't seem to work if I use offsets to the
    // iterators only. I would've though calling .begin() repeatedly would give
    // us fresh valid iterator each time, but apparently not.

    // Remove edge i, i-1 and remove vertex i
    vertices_.erase(vertices_.begin() + i);
    edges_.erase(edges_.begin() + i);
    auto it = edges_.erase(edges_.begin() + i - 1);
    --i;

    edges_.insert(it, replacement);
    last_edge = replacement;
  }
}

void RoutingPath::Flatten() {
  // We look for and try to eliminate wires that are too short to allow another
  // layer N wire over the top:
  //
  //    +-------+
  //    |       +---
  //    |     layer N
  //    |       +---
  //    +-------+
  //      |   |     <- connecting wire on layer (N - 1) or (N + 1) is
  //    +-------+      too short. We should just connect on layer N.
  // ---+       |
  //   layer N  |
  // ---+       |
  //    +-------+
  skipped_vias_.clear();
  for (size_t i = 1; i < vertices_.size(); ++i) {
    // Edge i connects vertex i and (i + 1).
    geometry::Layer last_layer;
    geometry::Layer last_target_layer;
    if (i == 1) {
      if (start_access_layers_.empty())
        continue;
      std::set<geometry::Layer> source_options = {
          edges_.at(0)->EffectiveLayer()
      };
      if (edges_.size() > 1) {
        source_options.insert(edges_.at(1)->EffectiveLayer());
      }
      auto start_access = PickAccessLayerPair(
          source_options, start_access_layers_);
      if (!start_access) {
        continue;
      }
      last_layer = start_access->source;
      last_target_layer = start_access->target;
    } else {
      //    0     1     2     3     4
      //  0    1     2     3     4     5
      int64_t last_edge = 
      last_layer = edges_.at(i - 2)->EffectiveLayer();
      last_target_layer = last_layer;
    }

    geometry::Layer next_layer;
    geometry::Layer next_target_layer;
    if (i == vertices_.size() - 1) {
      if (end_access_layers_.empty())
        continue;
      std::set<geometry::Layer> dest_options = {
          last_layer,
          edges_.back()->EffectiveLayer()
      };
      // Since we're deciding if we should skip the current edge, we pick an
      // appropriate end access layer based on the previous edge layer (since
      // that is the layer we will end up putting the current edge on if we do
      // decide to skip).
      auto end_access = PickAccessLayerPair(dest_options, end_access_layers_);
      if (!end_access) {
        continue;
      }
      next_layer = end_access->source;
      next_target_layer = end_access->target;
    } else {
      next_layer = edges_.at(i)->EffectiveLayer();
      next_target_layer = next_layer;
    }

    RoutingVertex *last_vertex = vertices_.at(i - 1);
    RoutingVertex *current_vertex = vertices_.at(i);

    // If either of the pair of vertices under consideration already appears in
    // the skip list, we must skip this to avoid inadvertently switching the
    // layer of an adjacent edge. (Also, since one of the vias is skipped it's
    // moot that they're too close together.)
    if (skipped_vias_.find(last_vertex) != skipped_vias_.end() ||
        skipped_vias_.find(current_vertex) != skipped_vias_.end()) {
      continue;
    }

    // last_vertex and current_vertex span current_edge.
    if (routing_grid_->VerticesAreTooCloseForVias(
            *last_vertex, *current_vertex) &&
        last_layer == next_layer) {
      skipped_vias_.insert(last_vertex);
      skipped_vias_.insert(current_vertex);
      RoutingEdge *flattened_edge = edges_.at(i - 1);

      geometry::Layer previous_layer_value = flattened_edge->EffectiveLayer();

      // Downgrade the edge layer!
      flattened_edge->set_layer(last_layer);

      // Danger! We might now have now changed the directions of the via encaps
      // at one end of the edge. For example, if previous it was a horizontal
      // encap on metal 1, but now the vertical metal 2 edge above it is being
      // reduced to metal 1, it is going to end up a vertical encap on metal 1.
      // That's bad. So we have to check how bad. Note also! This is only really
      // valid for one of the edges since one or both of them are having a via
      // skipped, as is the whole point of this process. But figuring out which
      // one is too annoying.
      //
      // These are no-ops if _layer == _target_layer.
      //
      // TODO(aryap): If either of these fails, we might have a DRC error. This
      // is really bad, since this condition should have been cleared before the
      // RoutingPath was created, as part of the path search.
      auto check_encaps = CheckAndForceEncapDirections(
          last_layer,
          last_target_layer,
          last_vertex);
      if (check_encaps.ok()) {
        last_vertex->set_connected_layers({last_layer, last_target_layer});
      }

      check_encaps.Update(CheckAndForceEncapDirections(
          next_layer,
          next_target_layer,
          current_vertex));
      if (check_encaps.ok()) {
        current_vertex->set_connected_layers({next_layer, next_target_layer});
      }
    }
  }
}

const std::vector<RoutingVertex*> RoutingPath::SpannedVerticesWithVias() const {
  // Straightforward filter.
  std::vector<RoutingVertex*> vias_;
  for (RoutingVertex *vertex : vertices_) {
    auto skipped = skipped_vias_.find(vertex);
    if (skipped == skipped_vias_.end()) {
      vias_.push_back(vertex);
    }
  }
  return vias_;
}

const std::set<RoutingVertex*> RoutingPath::SpannedVertices() const {
  std::set<RoutingVertex*> all;
  for (RoutingEdge *const edge : edges_) {
    std::vector<RoutingVertex*> spanned = edge->SpannedVertices();
    all.insert(spanned.begin(), spanned.end());
  }
  for (RoutingVertex *const vertex : vertices_) {
    DCHECK(all.find(vertex) != all.end())
        << "Union of spanned vertices in path did not include crucial vertex: "
        << *vertex;
  }
  return all;
}

const std::set<RoutingVertex*> RoutingPath::SpannedVerticesWithoutVias() const {
  std::set<RoutingVertex*> all = SpannedVertices();
  std::vector<RoutingVertex*> with_vias = SpannedVerticesWithVias();
  std::set<RoutingVertex*> all_without_vias;
  std::set_difference(all.begin(), all.end(),
                      with_vias.begin(), with_vias.end(),
                      std::inserter(
                          all_without_vias, all_without_vias.begin()));
  return all_without_vias;
}

void RoutingPath::AddPortMidway(const std::optional<std::string> &port_name) {
  // The initial cut of this algorithm is to just pick a near-midway vertex and
  // put the port on it, since if it straddles two different layers it should
  // be able to accommodate a port easily.
  size_t index = (vertices_.size() - skipped_vias_.size()) / 2;

  auto find_unskipped_via_by_index = [&](size_t index) -> RoutingVertex* {
    size_t i = 0;
    for (RoutingVertex *target : vertices_) {
      if (skipped_vias_.find(target) != skipped_vias_.end()) {
        continue;
      }
      if (i == index) {
        return target;
      }
      ++i;
    }
    return nullptr;
  };

  RoutingVertex *target = find_unskipped_via_by_index(index);

  target->set_hosts_port(
      port_name && *port_name != "" ? *port_name : nets_.primary());
}

void RoutingPath::ResolveTerminatingLayersAtBothEnds() {
  if (edges_.empty()) {
    return;
  }
  ResolveTerminatingLayers(start_access_layers_,
                           *edges_.front(),
                           &picked_start_layers_);
  ResolveTerminatingLayers(end_access_layers_,
                           *edges_.back(),
                           &picked_end_layers_);
}

void RoutingPath::ResolveTerminatingLayers(
    const std::set<geometry::Layer> &access_layers,
    const RoutingEdge &edge,
    std::optional<CostedLayerPair> *picked) {
  LOG_IF(FATAL, !edge.layer())
      << "Edge must have a layer assigned by this point.";
  const geometry::Layer &source_layer = *edge.layer();
  auto costed_access_layer = PickAccessLayerPair(
      {source_layer}, access_layers);
  geometry::Layer access_layer;
  if (!costed_access_layer) {
    LOG(WARNING) << "No reachability to access layers "
                 << absl::StrJoin(access_layers, ", ")
                 << " from layer " << source_layer;
    access_layer =
        access_layers.empty() ?  source_layer : *access_layers.begin();
    *picked = CostedLayerPair {
      .cost = 0.0,
      .source = source_layer,
      .target = access_layer
    };
    return;
  }
  *picked = *costed_access_layer;
}

void RoutingPath::Legalise() {
  if (legalised_)
    return;
  Abbreviate();
  MergeConsecutiveEdgesOnSameTrack();
  Flatten();
  ResolveTerminatingLayersAtBothEnds();
  legalised_ = true;
}

// Check every edge and vertex for use by other nets. This is a paranoid check
// that only makes sense in multithreading environments where the path was
// formed from elements that could have changed stated in the meantime.
//
// This isn't the same as a full legality (DRC) check, and is only intended to
// detect collisions when multiple paths are set to use the same elements:
absl::Status RoutingPath::CheckStillAvailable() const {
  for (RoutingVertex *vertex : vertices_) {
    auto using_net = vertex->InUseBySingleNet();
    if (using_net && !nets_.Contains(using_net->net)) {
      return absl::UnavailableError(absl::StrCat(
            "Vertex ", vertex->centre().Describe(), " already assigned to ",
            using_net->net));
    }
  }
  for (RoutingEdge *edge : edges_) {
    auto using_net = edge->PermanentNet();
    if (using_net && !nets_.Contains(*using_net)) {
      return absl::UnavailableError(absl::StrCat(
            "Edge ", edge->Describe(), " already assigned to ",
            *using_net));
    }
  }
  return absl::OkStatus();
}

// This can happen:
//
//            | L1|
//            |   |
//          +-------+
//   +------+       |
//   + +--+    L0   | Path A
//   +-|--|-+       |
//     |  | +-------+
//     |  |^notch
//     |  +-----     Path B
//     |  L0      <- This path, all on L0 at the end due to some previous
//     +--------     optimisations, terminates on an L0 pour in another path, B.
//                   No via is needed from L0 to L0 so a small notch appears in
//                   the overall L0 pour.
//
//  One fix is to add a bulge to the receiving path to make sure the notch
//  doesn't occur.
void RoutingPath::FixLandingOnShortEdgeInAnotherPath(
    const RoutingVertex &vertex,
    const RoutingEdge &edge) {
  const auto &installed_in_paths = vertex.installed_in_paths();
  for (const auto &entry : installed_in_paths) {
    if (entry.first == this)
      continue;
    
  }
}

void RoutingPath::CheckEdgeInPolyLineForIncidenceOfOtherPaths(
    geometry::PolyLine *poly_line,
    RoutingEdge *edge,
    std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const {
  CheckForViaCrowding(poly_line, edge, poly_lines);
  CheckForNotchesToPerpendicularEdges(poly_line, edge, poly_lines);
}

void RoutingPath::CheckForViaCrowding(
    geometry::PolyLine *poly_line,
    RoutingEdge *edge,
    std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const {
  // Add bulges where vertices are crossed by multiple paths on the same net.
  //
  // NOTE(aryap): We do not differentiate where vertices imply vias, because
  // they're at the end of edges, which require bulges, and where they do not.
  // That is, the following routine will insert bulges anytime vertices in a
  // path are crossed by vertices in another path on the same net. We might not
  // *want* to add via in such those cases, but the bulges for the vias are
  // made anyway.
  //
  // Inserting bulges too close to each other should result in final geometry
  // that avoids notches, or gaps between metal shapes that are larger than the
  // minimum separation allowed. This is taken care of by PolyLine. A separate
  // problem is created on the layers above or below that PolyLine, where the
  // metal typically runs orthogonally. We have to insert shapes to connect the
  // bulges on these layers or cover them with another PolyLine:
  //
  //      | A |
  //      |   |
  //    +-------+
  //    |       +---
  //    |     layer N, path B
  //    |       +---
  //    +-------+
  //      |   |     <- Avoiding the notch on layer N +/- 1 is taken care of by
  //    +-------+      PolyLine, but we have to do something about layer N.
  // ---+       |
  //   layer N, path A
  // ---+       |
  //    +-------+
  //
  // The list of sets of vertices which are too close together. Well, it would
  // be a set, but we need to keep the order of the vertices to save us some
  // computation later.
  auto vertices_too_close_for_vias =
      [this](const geometry::Layer &unused_layer,
             RoutingVertex *lhs,
             RoutingVertex *rhs) {
    return this->routing_grid_->VerticesAreTooCloseForVias(*lhs, *rhs);
  };
  LayeredRoutingVertexCollectors close_vertices =
      LayeredRoutingVertexCollectors(vertices_too_close_for_vias);

  std::map<geometry::Layer, int64_t> max_bulge_length_by_layer;

  std::vector<RoutingVertex*> spanned_vertices = edge->SpannedVertices();
  for (size_t i = 0; i < spanned_vertices.size(); ++i) {
    RoutingVertex *vertex = spanned_vertices.at(i);

    // This vertex might not be able to accomodate a new bulge; this so far
    // only has happened when two edges on the same net cross each other but
    // neither terminates on the other.
    //
    // As a reminder, here being 'available' as a vertex means being able to
    // accomodate a via.
    if (!vertex->AvailableForAll(nets_, std::nullopt)) {
      continue;
    }

    auto &installed_in_paths = vertex->installed_in_paths();
    VLOG(12) << "Vertex " << vertex->centre() << " is installed in "
             << installed_in_paths.size() << " paths";
    // The first and last vertices are explicitly considered as via candidates:
    if (i == 0 || i == spanned_vertices.size() - 1) {
      for (const geometry::Layer &layer : vertex->connected_layers()) {
        close_vertices.Offer(layer, vertex);
      }
    }

    for (auto &entry : installed_in_paths) {
      // This structure tells us the paths that are using the given vertex and
      // through which edge.
      RoutingPath *path = entry.first;
      if (path == this) {
        continue;
      }
      if (!nets_.ContainsAny(path->nets())) {
        // Ignore other paths crossing this vertex that aren't on the same net
        // as us.
        continue;
      }
      std::set<RoutingEdge*> &edges  = entry.second;
      int64_t bulge_width = 0;
      int64_t bulge_length = 0;
      auto encap_direction = routing_grid_->VertexEncapDirection(
          *vertex, poly_line->layer());
      std::optional<double> angle_rads;
      if (encap_direction) {
        angle_rads = RoutingTrack::DirectionToAngle(*encap_direction);
      }
      for (RoutingEdge *other_edge : edges) {
        VLOG(13) << "Path " << path << " via " << *other_edge;
        if (other_edge->EffectiveLayer() == poly_line->layer()) {
          continue;
        }
        auto bulge = GetBulgeDimensions(
            routing_grid_->GetRoutingViaInfoOrDie(
                poly_line->layer(), other_edge->EffectiveLayer()));
        max_bulge_length_by_layer[other_edge->EffectiveLayer()] =
            std::max(
                max_bulge_length_by_layer[other_edge->EffectiveLayer()],
                bulge.length);
        bulge_width = std::max(bulge_width, bulge.width);
        bulge_length = std::max(bulge_length, bulge.length);

        close_vertices.Offer(other_edge->EffectiveLayer(), vertex);
      }
      max_bulge_length_by_layer[poly_line->layer()] = bulge_length;
      if (bulge_width > 0 && bulge_length > 0) {
        LOG(INFO) << "Inserting " << bulge_width << " x " << bulge_length
                  << " bulge at " << vertex->centre()
                  << " on layer " << poly_line->layer();
        poly_line->InsertBulgeLater(
            vertex->centre(), bulge_width, bulge_length, angle_rads);
      }
    }
  }

  for (const auto &entry : close_vertices.collectors_by_layer()) {
    const RoutingVertexCollector &collector = entry.second;
    if (collector.num_offers() <= 2) {
      // We only care about super-close vias on the off-edge layers if more than
      // 2 were considered, since there should always be at least the start and
      // end vertices on the edge (and those are taken care of by the edge
      // PolyLine itself). Each offer to the collector is a vertex we suppose
      // might become a via.
      continue;
    }

    const geometry::Layer &layer = entry.first;
    for (const std::vector<RoutingVertex*> &group : collector.groups()) {
      if (group.empty()) continue;

      // TODO: This works but it hurts.
      geometry::PolyLine *cover = new geometry::PolyLine(
          {group.front()->centre(), group.back()->centre()});

      cover->set_layer(layer);
      cover->SetWidth(max_bulge_length_by_layer[layer]);

      poly_lines->emplace_back(cover);
    }
  }
}

// When a new edge comes in perpedicular to an existing edge, the presence of a
// nearby vias, or even just wide paths, can lead to a notch:
//             +-----------+
// +-----------+           |
// |           A           |
// |  +--X--+           O  |
// |  |     |              |
// +--|-----|--+           |
//    |     |  +-----------+
//    |  B  |^notch
//
// Path B landing on path A on the same layer is nice, but the skinny
// perpendicular final edge on B creates a notch to the larger via encap in A.
//
// There is a general, geometric way to check this, in the manner of some DRC
// touchups. Maybe It's actually a function of PolyLine to figure out notch
// prevention in these cases as it does in general for multiple bulges on
// itself. But here we implement a gross hack instead.
//
// We follow the basic method of CheckForViaCrowding, by checking every vertex
// in the edge to see if they are crossed by other paths on the same layer but
// in the perpedicular direction. The fix is to emit a bulge with the same
// dimensions as at the nearest encap site, and we only need to check vertices
// that are close enough to create a conflict.
//
// The test for this hack is whether vertices X and O are both via locations
// whose encap lengths are less than the minimum allowable separation apart, and
// also that the distance between the side of the perpendicular edge (B) is
// closer than the minimum separation to the end of the bulge on the vertex it's
// landing on.
void RoutingPath::CheckForNotchesToPerpendicularEdges(
    geometry::PolyLine *poly_line,
    RoutingEdge *edge,
    std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const {
  // Find vertices on the edge that will host vias:

  std::vector<RoutingVertex*> spanned_vertices = edge->SpannedVertices();

  std::map<RoutingVertex*, BulgeDimensions> bulges;
  for (RoutingVertex *vertex : spanned_vertices) {
    auto layers = vertex->ChangedEdgeAndLayers(
        std::bind(&RoutingGrid::AreLayersConnectableByVia,
                  routing_grid_,
                  std::placeholders::_1,
                  std::placeholders::_2));
    if (!layers)
      continue;
    BulgeDimensions bulge = GetBulgeDimensions(
        routing_grid_->GetRoutingViaInfoOrDie(layers->first, layers->second));
    bulges.insert({vertex, bulge});
  }

  const RoutingLayerInfo &layer_info = routing_grid_->GetRoutingLayerInfoOrDie(
      edge->EffectiveLayer());

  auto is_too_close_to_via = [&](
      RoutingVertex *vertex, RoutingEdge *other_edge) ->
          std::optional<BulgeDimensions> {
    auto it = bulges.find(vertex);
    if (it == bulges.end()) {
      return std::nullopt;
    }

    const BulgeDimensions &bulge = it->second;
    int64_t bulge_length = bulge.length;
    int64_t edge_width = layer_info.wire_width();

    int64_t edge_to_end_of_bulge = (bulge_length - edge_width) / 2;
    if (edge_to_end_of_bulge > layer_info.min_separation()) {
      return std::nullopt;
    }

    for (const auto &entry : bulges) {
      RoutingVertex *via_vertex = entry.first;
      if (via_vertex == vertex) {
        continue;
      }
      const BulgeDimensions &nearby_bulge = entry.second;
      if (nearby_bulge.width <= bulge.width) {
        continue;
      }
      int64_t distance = vertex->centre().L1DistanceTo(via_vertex->centre());
      // No laughing.
      int64_t their_bulge_length = nearby_bulge.length;

      // We're trying to pre-empt what the PolyLine is going to do when there is
      // a notch:
      int64_t separation =
          distance - (their_bulge_length / 2) - (bulge_length / 2);
      if (separation < layer_info.min_separation()) {
        return nearby_bulge;
      }
    }
    return std::nullopt;
  };

  for (RoutingVertex *vertex : spanned_vertices) {
    auto &installed_in_paths = vertex->installed_in_paths();
    for (auto &entry : installed_in_paths) {
      RoutingPath *path = entry.first;
      if (path == this) {
        continue;
      }
      std::set<RoutingEdge*> &edges  = entry.second;
      for (RoutingEdge *other_edge : edges) {
        // Compare to the condition in CheckForViaCrowding.
        if (other_edge->EffectiveLayer() != poly_line->layer()) {
          continue;
        }

        std::optional<BulgeDimensions> nearby_bulge =
            is_too_close_to_via(vertex, other_edge);
        if (!nearby_bulge) {
          continue;
        }
        poly_line->InsertBulgeLater(
            vertex->centre(), nearby_bulge->width, nearby_bulge->length);
      }
    }
  }
}

// A related problem to that described for 
// CheckEdgeInPolyLineForIncidenceOfOtherPaths is that what happens when
// connecting to a vertex which itself is connected to our net on both of its
// laters (typically a via-hosting vertex on some other path).
//
//      +-------+
//     +--------++
// ====+|       || path B
//     +---------+
//      +-+   +-+
//        |   |   <- path B can connect to path A on the vertical layer,
//      +-------+    but the horizontal layers are now too close together.
//     +---------+
//     ||       |+=======
//     +---------+  path A
//      +-+   +-+
//        |   |
//        |   |
//
// The general solution is to select the layer to connect to when beating the
// path, and/or make an exception to the layer for the final edge for short
// distances (so short that this is a problem).
//
// TODO(aryap): But where that doesn't work, it seems like it might be worth
// doing this:
void RoutingPath::MaybeFixFirstAndLastJog(
    std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const {
  // Unimplemented.
}

absl::Status RoutingPath::CheckAndForceEncapDirections(
    const geometry::Layer &from_layer,
    const geometry::Layer &to_layer,
    RoutingVertex *vertex) const {
  if (from_layer == to_layer) {
    return absl::OkStatus();
  }
  std::optional<std::vector<RoutingViaInfo>> via_layers =
      routing_grid_->FindViaStack(from_layer, to_layer);
  if (!via_layers) {
    // Bad.
    return absl::NotFoundError(
        absl::StrCat("No via stack from ", from_layer, " to ", to_layer));
  }

  std::map<geometry::Layer, std::set<RoutingTrackDirection>>
      access_directions_by_layer;

  auto create_or_intersect_fn = [&](
      const geometry::Layer &layer,
      const std::set<RoutingTrackDirection> access_directions) {
    auto it = access_directions_by_layer.find(layer);
    if (it == access_directions_by_layer.end()) {
      access_directions_by_layer.insert({layer, access_directions});
      return;
    }
    std::set<RoutingTrackDirection> &existing = it->second;
    std::set<RoutingTrackDirection> intersection;
    std::set_intersection(access_directions.begin(), access_directions.end(),
                          existing.begin(), existing.end(),
                          std::inserter(intersection, intersection.begin()));
    access_directions_by_layer.insert({layer, intersection});
  };

  for (const RoutingViaInfo &info : *via_layers) {
    const std::vector<geometry::Layer> connected_layers =
        info.ConnectedLayers();

    std::set<RoutingTrackDirection> access_directions =
        routing_grid_->ValidAccessDirectionsAt(
            vertex->centre(),
            connected_layers[0],
            connected_layers[1],  // The footprint layer.
            nets_);
    create_or_intersect_fn(connected_layers[1], access_directions);

    access_directions =
        routing_grid_->ValidAccessDirectionsAt(
            vertex->centre(),
            connected_layers[1],
            connected_layers[0],  // The footprint layer.
            nets_);
    create_or_intersect_fn(connected_layers[0], access_directions);
  }

  // Now, if any layer has an empty set of access directions, it is no longer
  // valid!
  std::string error_message;
  for (const auto &entry : access_directions_by_layer) {
    std::stringstream ss;
    for (const RoutingTrackDirection &direction : entry.second) {
      ss << direction << "; ";
    }
    LOG(INFO) << entry.first << ": " << ss.str();

    const geometry::Layer &layer = entry.first;
    if (entry.second.empty()) {
      error_message = absl::StrCat(
          "No access directions remain valid for layer ",
          layer, " at ", vertex->centre().Describe());
      return absl::InternalError(error_message);
    }
  }

  for (const auto &entry : access_directions_by_layer) {
    const geometry::Layer &layer = entry.first;
    const std::set<RoutingTrackDirection> &directions = entry.second;
    if (directions.size() == 1) {
      const RoutingTrackDirection &forced = *directions.begin();
      LOG(INFO) << "Forcing access direction on layer "
                << layer << " at " << vertex->centre() << " to " << forced;
      vertex->SetForcedEncapDirection(layer, forced);
    }
  }

  return absl::OkStatus();
}

void RoutingPath::BuildVias(
    const geometry::Point &at_point,
    const geometry::Layer &last_layer,
    const std::function<std::optional<RoutingTrackDirection>(
        const geometry::Layer&)> &get_encap_direction_fn,
    bool encap_last_layer,
    geometry::PolyLine *from_poly_line,
    std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  const geometry::Layer &from_layer = from_poly_line->layer();
  if (from_layer == last_layer) {
    // Nothing to do.
    return;
  }
  // We need to find the stack of vias necessary to get to `last_layer` from
  // `from_layer`.
  std::optional<std::vector<RoutingViaInfo>> via_layers =
      routing_grid_->FindViaStack(from_layer, last_layer);
  if (!via_layers) {
    LOG(ERROR) << "No known via stack from " << from_layer << " to "
               << last_layer;
    return;
  }

  std::map<geometry::Layer, BulgeDimensions> metal_pours;
  LOG(INFO) << "Building via stack from " << from_layer << " to " << last_layer;

  // Collect the dimensions required for metal pours interfacing with vias on
  // each layer in the stack, increasing the maximum-known to cover the most
  // restrictive case.
  for (const RoutingViaInfo &info : *via_layers) {
    const std::vector<geometry::Layer> connected_layers =
        info.ConnectedLayers();
    AbstractVia *via = new AbstractVia(
        at_point, connected_layers[0], connected_layers[1]);
    vias->emplace_back(via);
    LOG(INFO) << "Inserting via between layers ("
              << connected_layers[0] << ", "
              << connected_layers[1] << ") at "
              << at_point;

    const BulgeDimensions bulge = GetBulgeDimensions(info);

    const geometry::Layer &via_layer = info.layer();
    for (const geometry::Layer &layer : connected_layers) {
      // Otherwise, just insert a big-enough metal pour on the connected layer.
      auto it = metal_pours.find(layer);
      if (it == metal_pours.end()) {
        metal_pours[layer] = bulge;
      } else {
        it->second.width = std::max(it->second.width, bulge.width);
        it->second.length = std::max(it->second.length, bulge.length);
      }
    }
  }

  for (const auto &entry : metal_pours) {
    const geometry::Layer &layer = entry.first;
    const BulgeDimensions &bulge = entry.second;
    std::optional<RoutingTrackDirection> maybe_forced_direction =
        get_encap_direction_fn(layer);

    std::optional<double> encap_angle;
    if (maybe_forced_direction) {
      encap_angle = RoutingTrack::DirectionToAngle(*maybe_forced_direction);
    }

    if (layer == from_layer) {
      // Insert a bulge on the from_poly_line. If encap_angle is nullopt,
      // InsertBulge with use the axis of the line where the point lands.
      from_poly_line->InsertBulge(at_point,
                                  bulge.width,
                                  bulge.length,
                                  encap_angle);
      continue;
    } else if (!encap_last_layer && layer == last_layer) {
      // Skip.
      continue;
    }

    // Default ¯\_(ツ)_/¯
    RoutingTrackDirection encap_direction = maybe_forced_direction ?
        *maybe_forced_direction : RoutingTrackDirection::kTrackHorizontal;

    int64_t half_length = bulge.length / 2;
    int64_t half_width = bulge.width / 2;
    geometry::Point start;
    geometry::Point end;
    if (encap_direction == RoutingTrackDirection::kTrackHorizontal) {
      start = at_point - geometry::Point {half_length, 0};
      end = start + geometry::Point {bulge.length, 0};
    } else if (encap_direction == RoutingTrackDirection::kTrackVertical) {
      start = at_point - geometry::Point {0, half_length};
      end = start + geometry::Point {0, bulge.length};
    } else {
      LOG(FATAL) << "Unknown encap_direction: " << encap_direction;
    }

    geometry::PolyLine *metal_pour = new geometry::PolyLine(
        start,
        {geometry::LineSegment {end, static_cast<uint64_t>(bulge.width)}});
    metal_pour->set_layer(layer);
    polylines->emplace_back(metal_pour);

    LOG(INFO) << "Inserting PolyLine for metal pour ("
              << bulge.width << ", " << bulge.length << ") on layer " << layer
              << " at " << at_point;
  }

  return;
}

void RoutingPath::BuildTerminatingVias(
    const geometry::Layer &access_layer,
    bool encap_port,
    RoutingVertex *vertex,
    geometry::PolyLine *active_line,
    std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  auto get_encap_direction_fn = [&](const geometry::Layer &layer) {
    return vertex->GetForcedEncapDirection(layer);
  };

  // This is a no-op if front->layer() == start_access_layer:
  BuildVias(vertex->centre(),
            access_layer,
            get_encap_direction_fn,
            encap_port,
            active_line,
            polylines,
            vias);
}

std::optional<RoutingPath::CostedLayerPair> RoutingPath::PickAccessLayerPair(
    const std::set<geometry::Layer> &source_layers,
    const std::set<geometry::Layer> &target_layers) const {
  std::vector<CostedLayerPair> costed_layers;
  for (const geometry::Layer &source : source_layers) {
    for (const geometry::Layer &target : target_layers) {
      auto cost = routing_grid_->FindViaStackCost(source, target);
      if (!cost)
        continue;
      costed_layers.push_back({
            .cost = *cost,
            .source = source,
            .target = target
          });
    }
  }
  if (costed_layers.empty()) {
    LOG(WARNING)
        << "Could not figure out cheapest connected layer pair, sources: "
        << absl::StrJoin(source_layers, ", ") << " targets: "
        << absl::StrJoin(target_layers, ", ");
    return std::nullopt;
  }
  // TODO(aryap): Hmmm best to define this as CostedLayerPair operator<?
  static auto sort_fn = [](const CostedLayerPair &lhs,
                           const CostedLayerPair &rhs) {
    return lhs.cost < rhs.cost;
  };
  std::sort(costed_layers.begin(), costed_layers.end(), sort_fn);

  return costed_layers.front();
}

void RoutingPath::ToPointsAndLayers(
    std::vector<geometry::Point> *points,
    std::vector<geometry::Layer> *layers) const {
  for (RoutingVertex *const vertex : vertices_) {
    points->push_back(vertex->centre());
  }
  for (RoutingEdge *const edge : edges_) {
    layers->push_back(edge->EffectiveLayer());
  }
}

void RoutingPath::ToPolyLinesAndVias(
    std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  if (Empty())
    return;

  LOG_IF(FATAL, vertices_.size() != edges_.size() + 1)
      << "There should be one more vertex than there are edges.";

  std::unique_ptr<geometry::PolyLine> last;
  bool last_poly_line_was_first = true;
  RoutingEdge *last_edge = nullptr;
  RoutingEdge *next_edge = nullptr;
  std::vector<std::unique_ptr<geometry::PolyLine>> generated_lines;
  int64_t bulge_length = 0;
  int64_t bulge_width = 0;
  for (size_t i = 0; i < vertices_.size() - 1; ++i) {
    RoutingVertex *current = vertices_.at(i);
    last_edge = next_edge;
    next_edge = edges_.at(i);
    const geometry::Layer &layer = next_edge->EffectiveLayer();

    const RoutingLayerInfo &info =
        routing_grid_->GetRoutingLayerInfoOrDie(layer);

    // Insert a new PolyLine at layer crossings (or the start). Layer crossings
    // also require a via, unless the vertex via is skipped.
    if (!last || (last->layer() != layer)) {
      // TODO(aryap): Is this even an 'abstract' via still? We seem to have all
      // the concrete details in here.
      // TODO(aryap): It's more straightforward to assign all the vias and then
      // go through and insert bulges on the layers where they are.
      AbstractVia *via = nullptr;
      if (last) {
        // This is a change in layer, so we finish the last line and store it.
        last->AddSegment(current->centre(), info.wire_width());

        via = new AbstractVia(current->centre(), last->layer(), layer);
        vias->emplace_back(via);
        //last->set_end_via(via);

        if (current->hosts_port()) {
          via->set_port_on_top(*current->hosts_port());
        }

        auto bulge = GetBulgeDimensions(
            routing_grid_->GetRoutingViaInfoOrDie(last->layer(), layer));
        bulge_width = bulge.width;
        bulge_length = bulge.length;

        if (i != vertices_.size() - 1) {
          LOG(INFO) << "Inserting " << bulge_width << " x " << bulge_length
                    << " bulge at " << current->centre() << " i=" << i
                    << " " << vertices_.size()
                    << " on layer " << last->layer();
          last->InsertBulgeLater(current->centre(), bulge_width, bulge_length);
        }

        // Insert the starting bulge on the last poly line unless it was the
        // first one:
        if (!last_poly_line_was_first) {
          LOG(INFO) << "Inserting " << bulge_width << " x " << bulge_length
                    << " bulge at " << last->start() << " i=" << i
                    << " " << vertices_.size()
                    << " on layer " << last->layer();
          last->InsertBulgeLater(last->start(), bulge_width, bulge_length);
        } else {
          last_poly_line_was_first = false;
        }

        // When switching to a new PolyLine, this is the only place where we
        // know the final edge in the last PolyLine (last_edge) and the segment
        // for that corresponding edge has been added:
        if (last_edge) {
          CheckEdgeInPolyLineForIncidenceOfOtherPaths(
              last.get(), last_edge, polylines);
        }
    
        generated_lines.push_back(std::move(last));
      }
      // Start a new line.
      last.reset(new geometry::PolyLine());
      last->set_overhang_start(0);
      last->set_overhang_end(0);
      last->set_layer(layer);
      last->set_start(current->centre());
      last->set_min_separation(info.min_separation());
      last->set_net(nets_.primary());
      continue;
    }
    last->AddSegment(current->centre(), info.wire_width());

    if (last_edge) {
      CheckEdgeInPolyLineForIncidenceOfOtherPaths(
          last.get(), last_edge, polylines);
    }
  }

  if (generated_lines.empty() && !last)
    return;

  const RoutingLayerInfo &last_info = routing_grid_->GetRoutingLayerInfoOrDie(
      next_edge->EffectiveLayer());
  last->AddSegment(vertices_.back()->centre(), last_info.wire_width());
  LOG(INFO) << "Inserting " << bulge_width << " x " << bulge_length
            << " bulge at " << last->start();
  last->InsertBulgeLater(last->start(), bulge_width, bulge_length);

  CheckEdgeInPolyLineForIncidenceOfOtherPaths(
      last.get(), next_edge, polylines);

  generated_lines.push_back(std::move(last));

  // Apply all deferred bulges now that spine of each line should have been
  // created.
  for (auto &line : generated_lines) {
    line->ApplyDeferredBulges();
  }

  // Connect the start and end of the PolyLine to the appropriate layer with
  // appropriate encapsulation.
  geometry::PolyLine *front = generated_lines.front().get();

  // If there is more than 1 access layer, we prefer the lowest-cost.
  if (!start_access_layers_.empty()) {
    LOG_IF(FATAL, !picked_start_layers_)
        << "picked_start_layers_ is std::nullopt; did you call "
        << "ResolveTerminatingLayersAtBothEnds()?";
    BuildTerminatingVias(picked_start_layers_->target,
                         encap_start_port_,
                         vertices_.front(),
                         front,
                         polylines,
                         vias);
  }
  front->set_start_port(start_port_);

  geometry::PolyLine *back = generated_lines.back().get();
  if (!end_access_layers_.empty()) {
    LOG_IF(FATAL, !picked_end_layers_)
        << "picked_end_layers_ is std::nullopt; did you call "
        << "ResolveTerminatingLayersAtBothEnds()?";
    BuildTerminatingVias(picked_end_layers_->target,
                         encap_end_port_,
                         vertices_.back(),
                         back,
                         polylines,
                         vias);
  }
  back->set_end_port(end_port_);

  for (auto &poly_line : generated_lines) {
    polylines->push_back(std::move(poly_line));
  }
}

std::string RoutingPath::Describe() const {
  std::stringstream ss;
  if (Empty()) {
    ss << "empty path";
    return ss.str();
  }
  std::vector<std::string> vertex_centres;
  for (RoutingVertex *vertex : vertices_) {
    vertex_centres.push_back(vertex->centre().Describe());
  }
  ss << absl::StrJoin(vertex_centres, ", ");
  return ss.str();
}

std::ostream &operator<<(std::ostream &os, const RoutingPath &path) {
  os << path.Describe();
  return os;
}

}  // namespace bfg
