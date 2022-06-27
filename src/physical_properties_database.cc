#include "physical_properties_database.h"

#include <utility>
#include <glog/logging.h>

#include "geometry/layer.h"
#include "layer_info.pb.h"

namespace bfg {

using geometry::Layer;

namespace {

std::pair<const Layer&, const Layer&> OrderFirstAndSecondLayers(
    const Layer &lhs, const Layer &rhs) {
  const Layer &first = lhs <= rhs ? lhs : rhs;
  const Layer &second = rhs >= lhs ? rhs : lhs;
  return std::pair<const Layer&, const Layer&>(first, second);
}

}   // namespace

void PhysicalPropertiesDatabase::LoadPDKInfo(const proto::PDKInfo &pdk) {
  geometry::Layer internal_layer = 0;
  for (const auto &layer_info : pdk.layer_infos()) {
    // Find a free internal layer number:
    while (layer_infos_.find(internal_layer) != layer_infos_.end()) {
      internal_layer++;
      LOG_IF(FATAL, internal_layer == 0)
          << "Ran out of internal layer numbers!";
    }

    LOG(INFO) << "Loading layer " << internal_layer << ": \""
              << layer_info.name() << "\"";
    LayerInfo info {
        .internal_layer = internal_layer,
        .name = layer_info.name(),
        .gds_layer = static_cast<uint16_t>(layer_info.gds_layer()),
        .gds_datatype = static_cast<uint16_t>(layer_info.gds_datatype())
    };
    AddLayerInfo(info);
  }
}

void PhysicalPropertiesDatabase::AddRoutingLayerInfo(
    const RoutingLayerInfo &info) {
  const Layer &layer = info.layer;
  auto layer_info_it = routing_layer_infos_.find(layer);
  LOG_IF(FATAL, layer_info_it != routing_layer_infos_.end())
      << "Duplicate routing layer info: " << layer;
  routing_layer_infos_.insert({layer, info});
}

const RoutingLayerInfo &PhysicalPropertiesDatabase::GetRoutingLayerInfo(
    const Layer &layer) const {
  auto lhs_info_it = routing_layer_infos_.find(layer);
  LOG_IF(FATAL, lhs_info_it == routing_layer_infos_.end())
      << "Could not find routing info for layer: " << layer;
  return lhs_info_it->second;
}

void PhysicalPropertiesDatabase::AddLayerInfo(const LayerInfo &info) {
  // Add to mapping by layer number.
  const Layer &layer = info.internal_layer;
  auto number_iterator = layer_infos_.find(layer);
  LOG_IF(FATAL, number_iterator != layer_infos_.end())
      << "Duplicate layer info: " << layer;
  layer_infos_.insert({layer, info});

  auto name_iterator = layer_infos_by_name_.find(info.name);
  LOG_IF(FATAL, name_iterator != layer_infos_by_name_.end())
      << "Duplicate layer name when adding layer info: " << info.name;
  layer_infos_by_name_.insert({info.name, layer});
}

const LayerInfo &PhysicalPropertiesDatabase::GetLayerInfo(
    const Layer &layer) const {
  auto iterator = layer_infos_.find(layer);
  LOG_IF(FATAL, iterator == layer_infos_.end())
      << "Layer info not found: " << layer;
  return iterator->second;
}

const LayerInfo &PhysicalPropertiesDatabase::GetLayerInfo(
    const std::string &layer_name) const {
  auto iterator = layer_infos_by_name_.find(layer_name);
  LOG_IF(FATAL, iterator == layer_infos_by_name_.end())
      << "Layer info not found by name: " << layer_name;
  return GetLayerInfo(iterator->second);
}

void PhysicalPropertiesDatabase::AddViaInfo(
    const Layer &lhs,
    const Layer &rhs,
    const ViaInfo &info) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      OrderFirstAndSecondLayers(lhs, rhs);
  // Order first and second.
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  LOG_IF(FATAL,
      via_infos_.find(first) != via_infos_.end() &&
      via_infos_[first].find(second) != via_infos_[first].end())
      << "Attempt to specify ViaInfo for layers " << first << " and "
      << second << " again.";
  via_infos_[first][second] = info;
}

const ViaInfo &PhysicalPropertiesDatabase::GetViaInfo(
    const Layer &lhs, const Layer &rhs) const {
  std::pair<const Layer&, const Layer&> ordered_layers =
      OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;

  const auto first_it = via_infos_.find(first);
  LOG_IF(FATAL, first_it == via_infos_.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
  const std::map<Layer, ViaInfo> &inner_map = first_it->second;
  const auto second_it = inner_map.find(second);
  LOG_IF(FATAL, second_it == inner_map.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
  return second_it->second;
}

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db) {
  os << "Physical properties database";
  return os;
}

}   // namespace bfg
