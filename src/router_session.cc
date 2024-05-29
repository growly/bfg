#include "services/router_service.grpc.pb.h"

#include "router_session.h"

namespace bfg {

bool RouterSession::AddRoutes(const router_service::AddRoutesRequest &request) {
  // TODO(aryap):
  // We will have a list of nets to route with 2+ points:
  //  - Connect first two points with shortest path AddRouteBetween(...),
  //  give them the net label.
  //  - Connect successive points to the existing net.
  //  - Pray.
  return true;
}

router_service::Status RouterSession::SetUpRoutingGrid(
    const router_service::RoutingGridDefinition &grid_definition) {
  router_service::Status result;
  if (grid_definition.layers_size() < 2) {
    result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
    result.set_message("Too few grid definitions");
    return result;
  }
  if (grid_definition.layers_size() > 2) {
    result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
    result.set_message("Too many routing layer definitions");
    return result;
  }

  const PhysicalPropertiesDatabase &db = routing_grid_->physical_db();

  std::vector<RoutingLayerInfo> layer_infos;
  for (const router_service::RoutingLayerDefinition &layer_pb :
       grid_definition.layers()) {
    auto maybe_layer_info = db.GetRoutingLayerInfo(layer_pb.name());
    if (!maybe_layer_info) {
      result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
      result.set_message(
          absl::StrFormat("Missing info for layer: \"%s\"", layer_pb.name()));
      return result;
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

    routing_grid_->AddRoutingLayerInfo(layer_info);
  }

  // AddRoutingViaInfos
  // AddRoutingLayerInfos
  for (const router_service::RoutingViaDefinition &via_pb :
       grid_definition.vias()) {
    auto first_layer = db.FindLayer(via_pb.between_layer());
    if (!first_layer) {
      result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
      result.set_message(absl::StrFormat("Missing info for layer: \"%s\"",
                                         via_pb.between_layer()));
      return result;
    }
    auto second_layer = db.FindLayer(via_pb.and_layer());
    if (!second_layer) {
      result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
      result.set_message(absl::StrFormat("Missing info for layer: \"%s\"",
                                         via_pb.and_layer()));
      return result;
    }

    auto maybe_routing_via_info = db.GetRoutingViaInfo(
        via_pb.between_layer(), via_pb.and_layer());
    if (!maybe_routing_via_info) {
      result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
      result.set_message("Routing via info unavailable for given layers");
      return result;
    }
    RoutingViaInfo routing_via_info = *maybe_routing_via_info;

    routing_via_info.set_cost(via_pb.cost());

    routing_grid_->AddRoutingViaInfo(
        *first_layer, *second_layer, routing_via_info);
  }
  
  
  if (!routing_grid_->ConnectLayers(
          layer_infos[0].layer, layer_infos[1].layer)) {
    result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
    result.set_message("Could not complete layer connection");
    return result;
  }

  result.set_code(router_service::StatusCode::OK);
  return result;
}


}  // namespace bfg
