#include "routing_path.h"

#include <algorithm>
#include <deque>
#include <vector>
#include <memory>

#include <absl/strings/str_join.h>

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

namespace {

struct BulgeDimensions {
  int64_t width;
  int64_t length;
};

// TODO(aryap): There are different rules for overhanging from the layer above
// and below. RoutingViaInfo now differentiates these, so we should use them.
BulgeDimensions GetBulgeDimensions(const RoutingViaInfo &routing_via_info) {
  return BulgeDimensions {
    .width = routing_via_info.MaxEncapWidth(),
    .length = routing_via_info.MaxEncapLength()
  };
}

}    // namespace

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
  //                     +     +     +     +
  //                              a  |
  //      host track --> +-----+--x--+ d   +      v
  //                     |                        | separation
  //  <--axis direction->+-----+--+  + +---+      ^
  //                             b|    |
  //  alternative host track -->  | 
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

  auto assign_labels = [&](
      RoutingVertex *lhs, RoutingVertex *rhs) {
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
    if (i == 1) {
      if (start_access_layers_.empty())
        continue;
      last_layer = PickAccessLayer(
          edges_.at(0)->EffectiveLayer(), start_access_layers_);
    } else {
      last_layer = edges_.at(i - 2)->EffectiveLayer();
    }

    geometry::Layer next_layer;
    if (i == vertices_.size() - 1) {
      if (end_access_layers_.empty())
        continue;
      // Since we're deciding if we should skip the current edge, we pick an
      // appropriate end access layer based on the previous edge layer (since
      // that is the layer we will end up putting the current edge on if we do
      // decide to skip).
      next_layer = PickAccessLayer(last_layer, end_access_layers_);
    } else {
      next_layer = edges_.at(i)->EffectiveLayer();
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
      // Downgrade the edge layer!
      flattened_edge->set_layer(last_layer);
    }
  }
}

void RoutingPath::Legalise() {
  if (legalised_)
    return;
  Abbreviate();
  Flatten();
  legalised_ = true;
}

void RoutingPath::CheckEdgeInPolyLineForIncidenceOfOtherPaths(
    geometry::PolyLine *poly_line,
    RoutingEdge *edge,
    std::vector<std::unique_ptr<geometry::PolyLine>> *poly_lines) const {
  // Add bulges where vertices are crossed by multiple paths on the same net.
  //
  // NOTE(aryap): We do not differentiate where vertices imply vias, because
  // they're at the end of edges, which require bulges, and where they do not.
  // That is, the following routine will insert bulges anytime vertices in a
  // path are crossed by vertices in another path on the same net. We might not
  // *want* to add via in such those cases.
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
    auto &installed_in_paths = vertex->installed_in_paths();
    VLOG(12) << "Vertex " << vertex->centre() << " is installed in "
             << installed_in_paths.size() << " paths";
    // The first and last edges are explicitly considered as via candidates:
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
      for (RoutingEdge *other_edge : edges) {
        LOG(INFO) << "Path " << path << " via " << *other_edge;
        if (other_edge->EffectiveLayer() == poly_line->layer()) {
          continue;
        }
        auto bulge = GetBulgeDimensions(routing_grid_->GetRoutingViaInfoOrDie(
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
        poly_line->InsertBulgeLater(
            vertex->centre(), bulge_width, bulge_length);
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

void RoutingPath::BuildVias(
    geometry::PolyLine *from_poly_line,
    const geometry::Point &at_point,
    const geometry::Layer &last_layer,
    bool encap_last_layer,
    RoutingTrackDirection encap_direction,
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

    if (layer == from_layer) {
      // Insert a bulge on the from_poly_line.
      from_poly_line->InsertBulge(at_point, bulge.width, bulge.length);
      continue;
    } else if (!encap_last_layer && layer == last_layer) {
      // Skip.
      continue;
    }

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

geometry::Layer RoutingPath::PickAccessLayer(
    const geometry::Layer &source_layer,
    const std::set<geometry::Layer> &layers) const {
  if (layers.empty()) {
    return source_layer;
  }
  if (layers.size() == 1) {
    return *layers.begin();
  }
  std::vector<std::pair<geometry::Layer, double>> costed_layers;
  for (const geometry::Layer &layer : layers) {
    auto cost = routing_grid_->FindViaStackCost(source_layer, layer);
    if (!cost)
      continue;
    costed_layers.push_back({layer, *cost});
  }
  auto sort_fn = [](const std::pair<geometry::Layer, double> &lhs,
                    const std::pair<geometry::Layer, double> &rhs) {
    return lhs.second < rhs.second;
  };
  std::sort(costed_layers.begin(), costed_layers.end(), sort_fn);
  return costed_layers.front().first;
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
        auto bulge = GetBulgeDimensions(
            routing_grid_->GetRoutingViaInfoOrDie(last->layer(), layer));
        bulge_width = bulge.width;
        bulge_length = bulge.length;
        last->InsertBulgeLater(current->centre(), bulge_width, bulge_length);

        // Insert the starting bulge on the last poly line unless it was the
        // first one:
        if (!last_poly_line_was_first) {
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
    geometry::Layer start_access_layer = PickAccessLayer(
        front->layer(), start_access_layers_);
    // This is a no-op if front->layer() == start_access_layer:
    BuildVias(front,
              vertices_.front()->centre(),
              start_access_layer,
              encap_start_port_,
              RoutingTrackDirection::kTrackHorizontal,
              polylines,
              vias);
  }
  front->set_start_port(start_port_);

  geometry::PolyLine *back = generated_lines.back().get();
  if (!end_access_layers_.empty()) {
    geometry::Layer end_access_layer = PickAccessLayer(
        back->layer(), end_access_layers_);
    BuildVias(back,
              vertices_.back()->centre(),
              end_access_layer,
              encap_end_port_,
              RoutingTrackDirection::kTrackHorizontal,
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
