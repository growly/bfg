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
    RoutingVertex *start,
    const std::deque<RoutingEdge*> edges)
    : edges_(edges.begin(), edges.end()),
      start_port_(nullptr),
      end_port_(nullptr),
      encap_start_port_(false), 
      encap_end_port_(false) {
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
// and below.
BulgeDimensions GetBulgeDimensions(const RoutingViaInfo &routing_via_info) {
  int64_t via_width = std::max(
      routing_via_info.width, routing_via_info.height);
  return BulgeDimensions {
    .width = via_width + 2 * routing_via_info.overhang_width,
    .length = via_width + 2 * routing_via_info.overhang_length
  };
}

}    // namespace

void RoutingPath::CheckEdgeInPolyLineForIncidenceOfOtherPaths(
    const RoutingGrid &routing_grid,
    geometry::PolyLine *poly_line,
    RoutingEdge *edge) const {
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
      [&routing_grid](const geometry::Layer &unused_layer,
                      RoutingVertex *lhs,
                      RoutingVertex *rhs) {
    return routing_grid.VerticesAreTooCloseForVias(*lhs, *rhs);
  };
  LayeredRoutingVertexCollectors close_vertices =
      LayeredRoutingVertexCollectors(vertices_too_close_for_vias);

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
        // Ignore other paths crossing this vertex that aren't on the same net as us.
        continue;
      }
      std::set<RoutingEdge*> &edges  = entry.second;
      int64_t bulge_width = 0;
      int64_t bulge_length = 0;
      for (RoutingEdge *other_edge : edges) {
        LOG(INFO) << "Path " << path << " via " << *other_edge;
        if (other_edge->layer() == poly_line->layer()) {
          continue;
        }
        auto bulge = GetBulgeDimensions(routing_grid.GetRoutingViaInfoOrDie(
            poly_line->layer(), other_edge->layer()));
        bulge_width = std::max(bulge_width, bulge.width);
        bulge_length = std::max(bulge_length, bulge.length);

        close_vertices.Offer(other_edge->layer(), vertex);
      }
      if (bulge_width > 0 && bulge_length > 0) {
        poly_line->InsertBulgeLater(vertex->centre(), bulge_width, bulge_length);
      }
    }
  }

  for (const auto &entry : close_vertices.GroupsByLayer()) {
    const geometry::Layer &layer = entry.first;
    for (const auto &group : entry.second) {
      if (group.empty()) continue;
      LOG(INFO) << "on layer " << layer << " need to deal with ";
      for (RoutingVertex *const vertex : group) {
        LOG(INFO) << vertex->centre().Describe();
      }
    }
  }
}

void RoutingPath::BuildVias(
    const RoutingGrid &routing_grid,
    geometry::PolyLine *from_poly_line,
    const geometry::Point &at_point,
    const geometry::Layer &last_layer,
    bool encap_last_layer,
    RoutingTrackDirection encap_direction,
    std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) {
  const geometry::Layer &from_layer = from_poly_line->layer();
  if (from_layer == last_layer) {
    // Nothing to do.
    return;
  }
  // We need to find the stack of vias necessary to get to `last_layer` from
  // `from_layer`.
  std::vector<RoutingViaInfo> via_layers =
      routing_grid.FindViaStack(from_layer, last_layer);

  std::map<geometry::Layer, BulgeDimensions> metal_pours;
  LOG(INFO) << "Building via stack from " << from_layer << " to " << last_layer;

  // Collect the dimensions required for metal pours interfacing with vias on
  // each layer in the stack, increasing the maximum-known to cover the most
  // restrictive case.
  for (const RoutingViaInfo &info : via_layers) {
    AbstractVia *via = new AbstractVia(
        at_point, info.connected_layers[0], info.connected_layers[1]);
    vias->emplace_back(via);
    LOG(INFO) << "Inserting via between layers ("
              << info.connected_layers[0] << ", "
              << info.connected_layers[1] << ") at "
              << at_point;

    const BulgeDimensions bulge = GetBulgeDimensions(info);

    const geometry::Layer &via_layer = info.layer;
    for (const geometry::Layer &layer : info.connected_layers) {
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

void RoutingPath::ToPolyLinesAndVias(
    const RoutingGrid &routing_grid,
    std::vector<std::unique_ptr<geometry::PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  if (Empty())
    return;

  LOG_IF(FATAL, vertices_.size() != edges_.size() + 1)
      << "There should be one more vertex than there are edges.";

  std::set<RoutingVertex*> skipped_vias;
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
  for (size_t i = 2; i < vertices_.size() - 1; ++i) {
    // Edge i connects vertex i and (i + 1).
    RoutingEdge *last_edge = edges_.at(i - 2);
    RoutingEdge *current_edge = edges_.at(i - 1);
    RoutingEdge *next_edge = edges_.at(i);
    RoutingVertex *last_vertex = vertices_.at(i - 1);
    RoutingVertex *current_vertex = vertices_.at(i);

    // last_vertex and current_vertex span current_edge.
    if (routing_grid.VerticesAreTooCloseForVias(
            *last_vertex, *current_vertex) &&
        last_edge->ExplicitOrTrackLayer() ==
            next_edge->ExplicitOrTrackLayer()) {
      skipped_vias.insert(last_vertex);
      skipped_vias.insert(current_vertex);
    }
  }

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
    const geometry::Layer &layer = next_edge->ExplicitOrTrackLayer();

    const RoutingLayerInfo &info = routing_grid.GetRoutingLayerInfo(layer);

    auto it = skipped_vias.find(current);
    // Insert a new PolyLine at layer crossings (or the start). Layer crossings
    // also require a via, unless the vertex via is skipped.
    if (!last || (last->layer() != layer && it == skipped_vias.end())) {
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
            routing_grid.GetRoutingViaInfoOrDie(last->layer(), layer));
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
              routing_grid, last.get(), last_edge);
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
          routing_grid, last.get(), last_edge);
    }
  }

  if (generated_lines.empty() && !last)
    return;

  const RoutingLayerInfo &last_info = routing_grid.GetRoutingLayerInfo(
      next_edge->ExplicitOrTrackLayer());
  last->AddSegment(vertices_.back()->centre(), last_info.wire_width);
  last->InsertBulgeLater(last->start(), bulge_width, bulge_length);

  CheckEdgeInPolyLineForIncidenceOfOtherPaths(
      routing_grid, last.get(), next_edge);

  generated_lines.push_back(std::move(last));

  // Apply all deferred bulges now that spine of each line should have been created.
  for (auto &line : generated_lines) {
    line->ApplyDeferredBulges();
  }

  // Connect the start and end of the PolyLine to the appropriate layer with
  // appropriate encapsulation.
  geometry::PolyLine *front = generated_lines.front().get();

  if (start_access_layer_ &&
      front->layer() != *start_access_layer_) {
    BuildVias(routing_grid,
              front,
              vertices_.front()->centre(),
              *start_access_layer_,
              encap_start_port_,
              RoutingTrackDirection::kTrackHorizontal,
              polylines,
              vias);
  }
  front->set_start_port(start_port_);

  geometry::PolyLine *back = generated_lines.back().get();
  if (end_access_layer_ &&
      back->layer() != *end_access_layer_) {
    BuildVias(routing_grid,
              back,
              vertices_.back()->centre(),
              *end_access_layer_,
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

std::ostream &operator<<(std::ostream &os, const RoutingPath &path) {
  if (path.Empty()) {
    os << "empty path";
    return os;
  }
  for (RoutingVertex *vertex : path.vertices()) {
    os << vertex->centre() << " ";
  }
  return os;
}

}  // namespace bfg
