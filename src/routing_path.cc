#include "routing_path.h"

#include <algorithm>
#include <deque>
#include <vector>
#include <memory>

#include "abstract_via.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"

namespace bfg {

RoutingPath::RoutingPath(
    RoutingVertex *start, const std::deque<RoutingEdge*> edges)
    : edges_(edges.begin(), edges.end()) {
  vertices_.push_back(start);
  RoutingVertex *last = start;
  for (RoutingEdge *edge : edges) {
    RoutingVertex *next_vertex =
        edge->first() == last ? edge->second() : edge->first();
    vertices_.push_back(next_vertex);
    last = next_vertex;
  }
}

void RoutingPath::ToPolyLinesAndVias(
    const RoutingGrid &routing_grid,
    std::vector<std::unique_ptr<PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  if (Empty())
    return;

  LOG_IF(FATAL, vertices_.size() != edges_.size() + 1)
      << "There should be one more vertex than there are edges.";
  std::unique_ptr<PolyLine> last;
  RoutingEdge *edge = nullptr;
  int64_t bulge_length = 0;
  int64_t bulge_width = 0;
  for (size_t i = 0; i < vertices_.size() - 1; ++i) {
    RoutingVertex *current = vertices_.at(i);
    edge = edges_.at(i);
    const geometry::Layer &layer = edge->ExplicitOrTrackLayer();

    const RoutingLayerInfo &info = routing_grid.GetRoutingLayerInfo(layer);

    if (!last || last->layer() != layer) {
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
        last->set_end_via(via);
        const RoutingViaInfo &routing_via_info =
            routing_grid.GetRoutingViaInfo(last->layer(), layer);
        int64_t via_width = std::max(routing_via_info.width, routing_via_info.height);
        bulge_length = via_width + 2 * routing_via_info.overhang_length;
        bulge_width = via_width + 2 * routing_via_info.overhang_width;
        last->InsertBulge(current->centre(), bulge_width, bulge_length);
        last->InsertBulge(last->start(), bulge_width, bulge_length);
        // TODO(aryap): There are different rules for overhanging from the
        // layer above and below.
        polylines->push_back(std::move(last));
      }
      // Start a new line.
      last.reset(new PolyLine());
      last->set_layer(layer);
      last->set_start(current->centre());
      last->set_start_via(via);
      continue;
    }
    last->AddSegment(current->centre(), info.wire_width);
  }

  const RoutingLayerInfo &last_info = routing_grid.GetRoutingLayerInfo(
      edge->ExplicitOrTrackLayer());
  // TODO(aryap): How do get info for last port?
  last->AddSegment(vertices_.back()->centre(), last_info.wire_width);
  last->InsertBulge(last->start(), bulge_width, bulge_length);
  last->InsertBulge(last->segments().back().end, bulge_width, bulge_length);
  polylines->push_back(std::move(last));

  // Copy pointers to the start and end ports, if any.
  if (!polylines->empty()) {
    polylines->front()->set_start_port(start_port_);
    polylines->front()->set_end_port(end_port_);
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
