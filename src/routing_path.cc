#include "routing_path.h"

#include <algorithm>
#include <deque>
#include <vector>
#include <memory>

#include <absl/strings/str_join.h>

#include "geometry/poly_line.h"
#include "geometry/port.h"
#include "abstract_via.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_vertex_collector.h"
#include "routing_track.h"
#include "physical_properties_database.h"

namespace bfg {

RoutingPath::RoutingPath(
    const RoutingGrid &routing_grid,
    RoutingVertex *start,
    const std::deque<RoutingEdge*> edges)
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
    if (routing_grid_.VerticesAreTooCloseForVias(
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
    return this->routing_grid_.VerticesAreTooCloseForVias(*lhs, *rhs);
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
    std::optional<std::string> same_net = net_;

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
      if (path->net() != same_net.value()) {
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
        auto bulge = GetBulgeDimensions(routing_grid_.GetRoutingViaInfoOrDie(
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
      routing_grid_.FindViaStack(from_layer, last_layer);
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
    auto cost = routing_grid_.FindViaStackCost(source_layer, layer);
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
        routing_grid_.GetRoutingLayerInfoOrDie(layer);

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
        last->AddSegment(current->centre(), info.wire_width);

        via = new AbstractVia(current->centre(), last->layer(), layer);
        vias->emplace_back(via);
        //last->set_end_via(via);
        auto bulge = GetBulgeDimensions(
            routing_grid_.GetRoutingViaInfoOrDie(last->layer(), layer));
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
      last->set_min_separation(info.min_separation);
      last->set_net(net_);
      continue;
    }
    last->AddSegment(current->centre(), info.wire_width);

    if (last_edge) {
      CheckEdgeInPolyLineForIncidenceOfOtherPaths(
          last.get(), last_edge, polylines);
    }
  }

  if (generated_lines.empty() && !last)
    return;

  const RoutingLayerInfo &last_info = routing_grid_.GetRoutingLayerInfoOrDie(
      next_edge->EffectiveLayer());
  last->AddSegment(vertices_.back()->centre(), last_info.wire_width);
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
