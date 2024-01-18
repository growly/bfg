#include "routing_path.h"

#include <algorithm>
#include <deque>
#include <vector>
#include <memory>

#include "abstract_via.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"
#include "routing_track.h"

namespace bfg {

RoutingPath::RoutingPath(
    RoutingVertex *start,
    const std::deque<RoutingEdge*> edges)
    : edges_(edges.begin(), edges.end()),
      start_port_(nullptr),
      end_port_(nullptr) {
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
BulgeDimensions GetBulgeWidthAndLengthForViaBetweenLayers(
    const RoutingGrid &routing_grid,
    const geometry::Layer &first_layer,
    const geometry::Layer &second_layer) {
  const RoutingViaInfo &routing_via_info =
      routing_grid.GetRoutingViaInfoOrDie(first_layer, second_layer);
  int64_t via_width = std::max(
      routing_via_info.width, routing_via_info.height);
  return BulgeDimensions {
    .width = via_width + 2 * routing_via_info.overhang_width,
    .length = via_width + 2 * routing_via_info.overhang_length
  };
}

}   // namespace

void RoutingPath::ToPolyLinesAndVias(
    const RoutingGrid &routing_grid,
    std::vector<std::unique_ptr<PolyLine>> *polylines,
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
  for (size_t i = 2; i < vertices_.size() - 2; ++i) {
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

  std::unique_ptr<PolyLine> last;
  bool last_poly_line_was_first = true;
  RoutingEdge *edge = nullptr;
  std::vector<std::unique_ptr<PolyLine>> generated_lines;
  int64_t bulge_length = 0;
  int64_t bulge_width = 0;
  for (size_t i = 0; i < vertices_.size() - 1; ++i) {
    RoutingVertex *current = vertices_.at(i);
    edge = edges_.at(i);
    const geometry::Layer &layer = edge->ExplicitOrTrackLayer();

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
        auto bulge = GetBulgeWidthAndLengthForViaBetweenLayers(
            routing_grid, last->layer(), layer);
        bulge_width = bulge.width;
        bulge_length = bulge.length;
        last->InsertBulge(current->centre(), bulge_width, bulge_length);

        // Insert the starting bulge on the last poly line unless it was the
        // first one:
        if (!last_poly_line_was_first) {
          last->InsertBulge(last->start(), bulge_width, bulge_length);
        } else {
          last_poly_line_was_first = false;
        }
        generated_lines.push_back(std::move(last));
      }
      // Start a new line.
      last.reset(new PolyLine());
      last->set_overhang_start(0);
      last->set_overhang_end(0);
      last->set_layer(layer);
      last->set_start(current->centre());
      last->set_min_separation(info.min_separation);
      last->set_net(net_);
      //last->set_start_via(via);
      continue;
    }
    last->AddSegment(current->centre(), info.wire_width);
  }

  if (generated_lines.empty() && !last)
    return;

  const RoutingLayerInfo &last_info = routing_grid.GetRoutingLayerInfo(
      edge->ExplicitOrTrackLayer());
  last->AddSegment(vertices_.back()->centre(), last_info.wire_width);
  last->InsertBulge(last->start(), bulge_width, bulge_length);
  generated_lines.push_back(std::move(last));

  // Connect the start and end of the PolyLine to the appropriate layer with
  // appropriate encapsulation.
  PolyLine *front = generated_lines.front().get();
  const geometry::Point &start_point = front->start();
  if (start_access_layer_ &&
      front->layer() != *start_access_layer_) {
    AbstractVia *via = new AbstractVia(
        start_point, front->layer(), *start_access_layer_);
    vias->emplace_back(via);

    auto bulge = GetBulgeWidthAndLengthForViaBetweenLayers(
        routing_grid, front->layer(), *start_access_layer_);
    front->InsertBulge(start_point, bulge.width, bulge.length);
  }
  front->set_start_port(start_port_);

  PolyLine *back = generated_lines.back().get();
  const geometry::Point &end_point = back->End();
  if (end_access_layer_ &&
      back->layer() != *end_access_layer_) {
    AbstractVia *via = new AbstractVia(
        end_point, back->layer(), *end_access_layer_);
    vias->emplace_back(via);

    auto bulge = GetBulgeWidthAndLengthForViaBetweenLayers(
        routing_grid, back->layer(), *end_access_layer_);
    back->InsertBulge(end_point, bulge.width, bulge.length);
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
