#include <iomanip>
#include <memory>
#include <optional>
#include <string>
#include <sstream>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/status/status.h>

#include "geometry/layer.h"
#include "geometry/point.h"
#include "geometry/port.h"
#include "router_session.h"
#include "routing_path.h"

#include "services/router_service.grpc.pb.h"

namespace bfg {

absl::StatusOr<geometry::Port> RouterSession::PointAndLayerToPort(
    const std::string &net,
    const router_service::PointOnLayer &point_on_layer) const {
  auto layer =
      routing_grid_->physical_db().FindLayer(point_on_layer.layer_name());
  if (!layer) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Could not convert point in requested route to port: (",
        point_on_layer.point().x(), ", ",
        point_on_layer.point().y(), "), layer: ",
        point_on_layer.layer_name(), ". Does the layer exist?"));
  }
  geometry::Point centre = {
      point_on_layer.point().x(), point_on_layer.point().y()};
  return geometry::Port(centre, 100U, 100U, *layer, net);
}

absl::Status RouterSession::AddRoutes(
    const router_service::AddRoutesRequest &request) {
  // We will have a list of nets to route with 2+ points:
  //  - Connect first two points with shortest path AddRouteBetween(...),
  //  give them the net label.
  //  - Connect successive points to the existing net.
  //  - Pray.
  std::vector<absl::Status> results;

  bool conjunction = true;
  for (const router_service::NetRouteOrder &net_route_order :
       request.net_route_orders()) {
    absl::Status routed = PerformNetRouteOrder(net_route_order);
    conjunction = routed.ok() && conjunction;
    results.push_back(routed);
  }

  if (!conjunction) {
    std::stringstream overall_error;
    for (size_t i = 0; i < results.size(); ++i) {
      const auto &result = results.at(i);
      if (!result.ok()) {
        overall_error << "For net \"" << request.net_route_orders(i).net()
                      << "\": " << result.message() << "; ";
      }
    }
    return absl::InternalError(overall_error.str());
  }

  return absl::OkStatus();
}

void RouterSession::ExportRoutes(router_service::AddRoutesReply *reply) const {
  for (RoutingPath *path : routing_grid_->paths()) {
    router_service::Route *route = reply->add_routes();
    route->set_net(path->net());

    std::vector<geometry::Point> points;
    std::vector<geometry::Layer> layers;
    path->ToPointsAndLayers(&points, &layers);

    for (const geometry::Point &point : points) {
      router_service::Point *point_pb = route->add_points();
      point_pb->set_x(point.x());
      point_pb->set_y(point.y());
    }

    for (const geometry::Layer &layer : layers) {
      auto maybe_name = physical_db_.GetLayerName(layer);
      if (maybe_name) {
        route->add_layers(*maybe_name);
      } else {
        route->add_layers(absl::StrFormat("unknown_%d", layer));
      }
    }
  }
}

absl::Status RouterSession::PerformNetRouteOrder(
    const router_service::NetRouteOrder &request) {
  LOG(INFO) << "Routing net " << std::quoted(request.net());

  if (request.points_size() < 2) {
    // Nothing to do.
    return absl::OkStatus();
  }

  auto start = PointAndLayerToPort(request.net(), *request.points().begin());
  if (!start.ok()) {
    return start.status();
  }

  auto next = PointAndLayerToPort(
      request.net(), *(request.points().begin() + 1));
  if (!next.ok()) {
    return next.status();
  }

  LOG(INFO) << "Routing " << *start << " to " << *next;
  absl::Status initial = routing_grid_->AddRouteBetween(
      *start, *next, {}, request.net());
  if (!initial.ok()) {
    return initial;
  }

  for (size_t i = 2; i < request.points_size(); ++i) {
    auto next = PointAndLayerToPort(
        request.net(), *(request.points().begin() + i));
    if (!next.ok()) {
      return next.status();
    }
    LOG(INFO) << "Routing " << *next << " to net "
              << std::quoted(request.net());
    absl::Status subsequent = routing_grid_->AddRouteToNet(
        *next, request.net(), {});
    if (!subsequent.ok()) {
      // TODO(aryap): Should probably assemble these into a single status like
      // we do above.
    }
  }
  return absl::OkStatus();
}

absl::Status RouterSession::SetUpRoutingGrid(
    const router_service::RoutingGridDefinition &grid_definition) {
  router_service::Status result;
  if (grid_definition.layers_size() < 2) {
    return absl::InvalidArgumentError("Too few grid definitions");
  }
  if (grid_definition.layers_size() > 2) {
    return absl::InvalidArgumentError("Too many routing layer definitions");
  }

  const PhysicalPropertiesDatabase &db = routing_grid_->physical_db();

  std::vector<RoutingLayerInfo> layer_infos;
  for (const router_service::RoutingLayerDefinition &layer_pb :
       grid_definition.layers()) {
    auto maybe_layer_info = db.GetRoutingLayerInfo(layer_pb.name());
    if (!maybe_layer_info) {
      return absl::InvalidArgumentError(
          absl::StrFormat("Missing info for layer: \"%s\"", layer_pb.name()));
    }
    RoutingLayerInfo layer_info = *maybe_layer_info;

    switch (layer_pb.direction()) {
      case router_service::RoutingLayerDirection::TRACK_DIRECTION_VERTICAL:
        layer_info.direction = RoutingTrackDirection::kTrackVertical;
        break;
      case router_service::RoutingLayerDirection::TRACK_DIRECTION_HORIZONTAL:
        layer_info.direction = RoutingTrackDirection::kTrackHorizontal;
        break;
      case router_service::RoutingLayerDirection::TRACK_DIRECTION_NONE:
        // Fallthrough intended.
      default:
        break;
    }

    layer_info.area = geometry::Rectangle(
        geometry::Point(
            layer_pb.area().lower_left().x(),
            layer_pb.area().lower_left().y()
        ),
        geometry::Point(
            layer_pb.area().upper_right().x(),
            layer_pb.area().upper_right().y()
        ));

    layer_info.offset = layer_pb.offset();
    layer_infos.push_back(layer_info);

    absl::Status maybe_add = routing_grid_->AddRoutingLayerInfo(layer_info);
    if (!maybe_add.ok()) {
      return maybe_add;
    }
  }

  // AddRoutingViaInfos
  // AddRoutingLayerInfos
  for (const router_service::RoutingViaDefinition &via_pb :
       grid_definition.vias()) {
    auto first_layer = db.FindLayer(via_pb.between_layer());
    if (!first_layer) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "Missing info for layer: \"%s\"", via_pb.between_layer()));
    }
    auto second_layer = db.FindLayer(via_pb.and_layer());
    if (!second_layer) {
      return absl::InvalidArgumentError(absl::StrFormat(
            "Missing info for layer: \"%s\"", via_pb.and_layer()));
    }

    auto maybe_routing_via_info = db.GetRoutingViaInfo(
        via_pb.between_layer(), via_pb.and_layer());
    if (!maybe_routing_via_info) {
      return absl::InvalidArgumentError(
          "Routing via info unavailable for given layers");
    }
    RoutingViaInfo routing_via_info = *maybe_routing_via_info;

    routing_via_info.set_cost(via_pb.cost());

    absl::Status maybe_add = routing_grid_->AddRoutingViaInfo(
        *first_layer, *second_layer, routing_via_info);
    if (!maybe_add.ok()) {
      return maybe_add;
    }
  }
  
  absl::Status try_connect = routing_grid_->ConnectLayers(
          layer_infos[0].layer, layer_infos[1].layer);
  if (!try_connect.ok()) {
    return try_connect;
  }

  return absl::OkStatus();
}


}  // namespace bfg
