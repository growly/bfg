#include "possessive_routing_path.h"

#include <deque>
#include <vector>
#include <memory>

#include "abstract_via.h"
#include "routing_edge.h"
#include "routing_grid.h"
#include "routing_vertex.h"

namespace bfg {

PossessiveRoutingPath::~PossessiveRoutingPath {
  for (RoutingVertex *vertex : vertices_) { delete vertex; }
  for (RoutingEdge *edge : edges_) { delete edge; }
}

PossessiveRoutingPath::PossessiveRoutingPath(
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

void PossessiveRoutingPath::ToPolyLinesAndVias(
    const RoutingGrid &routing_grid,
    std::vector<std::unique_ptr<PolyLine>> *polylines,
    std::vector<std::unique_ptr<AbstractVia>> *vias) const {
  if (Empty())
    return;

  LOG_IF(FATAL, vertices_.size() != edges_.size() + 1)
      << "There should be one more vertex than there are edges.";
  std::unique_ptr<PolyLine> last;
  for (size_t i = 0; i < vertices_.size() - 1; ++i) {
    RoutingVertex *current = vertices_.at(i);
    RoutingEdge *edge = edges_.at(i);
    const geometry::Layer &layer = edge->ExplicitOrTrackLayer();

    const RoutingLayerInfo &info = routing_grid.GetRoutingLayerInfo(layer);

    if (!last || last->layer() != layer) {
      AbstractVia *via = nullptr;
      if (last) {
        // This is a change in layer, so we finish the last line and store it.
        last->AddSegment(current->centre(), info.wire_width);
        via = new AbstractVia(current->centre(), last->layer(), layer);
        vias->emplace_back(via);
        last->set_end_via(via);
        polylines->push_back(std::move(last));
      }
      // Start a new line.
      last.reset(new PolyLine());
      last->set_layer(layer);
      last->set_start(current->centre());
      last->set_start_via(via);
      continue;
    }
    last->AddSegment(current->centre());
  }
  last->AddSegment(vertices_.back()->centre());
  polylines->push_back(std::move(last));

  // Copy pointers to the start and end ports, if any.
  if (!polylines->empty()) {
    polylines->front()->set_start_port(start_port_);
    polylines->front()->set_end_port(end_port_);
  }
}

std::ostream &operator<<(std::ostream &os, const PossessiveRoutingPath &path) {
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
