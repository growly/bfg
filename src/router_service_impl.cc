#include "router_service_impl.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>
#include <iomanip>
#include <memory>

#include <google/protobuf/text_format.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "dev_pdk_setup.h"

#include "geometry/point.h"
#include "geometry/rectangle.h"
#include "routing_grid.h"
#include "routing_layer_info.h"
#include "physical_properties_database.h"

#include "vlsir/tech.pb.h"
#include "services/router_service.grpc.pb.h"

namespace bfg {

grpc::Status RouterServiceImpl::CreateRoutingGrid(
    grpc::ServerContext *context,
    const CreateRoutingGridRequest *request,
    CreateRoutingGridReply *reply) {
  PhysicalPropertiesDatabase physical_db;

  LOG(INFO) << "CreateRoutingGrid request";
  std::string tech_proto_source;
  switch (request->predefined_technology()) {
    case router_service::TECHNOLOGY_SKY130:
      LOG(INFO) << "Loading sky130";
      physical_db.LoadTechnologyFromFile("../sky130.technology.pb");
      SetUpSky130(&physical_db);
      break;
    case router_service::TECHNOLOGY_GF180MCU:
      LOG(INFO) << "Loading gf180mcu";
      physical_db.LoadTechnologyFromFile("../gf180mcu.technology.pb");
      SetUpGf180Mcu(&physical_db);
      break;
    case router_service::TECHNOLOGY_OTHER:
      // Fallthrough intended.
    default:
      // Do nothing.
      break;
  }

  RoutingGrid *grid = new RoutingGrid(physical_db);

  // Define grid with router and ConnectLayers().
  router_service::Status set_up =
      SetUpRoutingGrid(request->grid_definition(), grid);

  reply->mutable_status()->CopyFrom(set_up);
  if (set_up.code() != router_service::StatusCode::OK) {
    delete grid;
    return grpc::Status::OK;
  }

  RouterServiceImpl::UUID next_uuid = NextUUID();
  auto insertion_it = sessions_.insert({
      next_uuid,
      // new RouterSession(physical_db)
      std::make_unique<RouterSession>(grid)
  });
  reply->set_grid_id(next_uuid);

  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::AddRoutes(
    grpc::ServerContext *context,
    const AddRoutesRequest *request,
    AddRoutesReply *reply) {
  RouterSession *session = GetSession(request->grid_id());
  if (!session) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }
  if (!session->AddRoutes(*request)) {
    // Some error.
    reply->mutable_status()->set_code(
        router_service::StatusCode::OTHER_ERROR);
    return grpc::Status::OK;
  }

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::QueryRoutingGrid(
    grpc::ServerContext *context,
    const QueryRoutingGridRequest *request,
    QueryRoutingGridReply *reply) {
  RouterSession *session = GetSession(request->grid_id());
  if (!session) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

grpc::Status RouterServiceImpl::DeleteRoutingGrid(
    grpc::ServerContext *context,
    const DeleteRoutingGridRequest *request,
    DeleteRoutingGridReply *reply) {
  size_t num_removed = sessions_.erase(request->grid_id());
  if (num_removed < 1) {
    reply->mutable_status()->set_code(
        router_service::StatusCode::GRID_NOT_FOUND);
    return grpc::Status::OK;
  }

  sessions_.erase(request->grid_id());

  reply->mutable_status()->set_code(router_service::StatusCode::OK);
  return grpc::Status::OK;
}

// Other.
RouterSession *RouterServiceImpl::GetSession(
    const RouterServiceImpl::UUID &uuid) {
  auto it = sessions_.find(uuid);
  if (it == sessions_.end()) {
    return nullptr;
  }
  return it->second.get();
}

RoutingGrid *RouterServiceImpl::GetGrid(
    const RouterServiceImpl::UUID &uuid) {
  RouterSession *session = GetSession(uuid);
  if (!session) {
    return nullptr;
  }
  return session->routing_grid();
}

const RouterServiceImpl::UUID RouterServiceImpl::NextUUID() const { 
  RouterServiceImpl::UUID next = highest_index_ + 1;
  while (sessions_.find(next) != sessions_.end()) {
    next++;
  }
  return next;
}

router_service::Status RouterServiceImpl::SetUpRoutingGrid(
    const router_service::RoutingGridDefinition &grid_definition,
    RoutingGrid *grid) {
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

  const PhysicalPropertiesDatabase &db = grid->physical_db();

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

    grid->AddRoutingLayerInfo(layer_info);
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

    grid->AddRoutingViaInfo(*first_layer, *second_layer, routing_via_info);
  }
  
  
  if (!grid->ConnectLayers(layer_infos[0].layer, layer_infos[1].layer)) {
    result.set_code(router_service::StatusCode::INVALID_ARGUMENT);
    result.set_message("Could not complete layer connection");
    return result;
  }

  result.set_code(router_service::StatusCode::OK);
  return result;
}

}  // namespace bfg
