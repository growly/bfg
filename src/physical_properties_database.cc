#include "physical_properties_database.h"

#include <map>
#include <unordered_map>
#include <ostream>
#include <utility>
#include <glog/logging.h>
#include <absl/strings/str_split.h>

#include "geometry/layer.h"
#include "vlsir/tech.pb.h"

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

void PhysicalPropertiesDatabase::LoadTechnology(
    const vlsir::tech::Technology &pdk) {
  geometry::Layer internal_layer = 0;
  for (const auto &layer_info : pdk.layers()) {
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
        .purpose = layer_info.purpose().description(),
        .gds_layer = static_cast<uint16_t>(layer_info.index()),
        .gds_datatype = static_cast<uint16_t>(layer_info.sub_index())
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
  if (name_iterator == layer_infos_by_name_.end()) {
    auto insertion_result = layer_infos_by_name_.insert({info.name,
        std::unordered_map<std::string, geometry::Layer>()});
    LOG_IF(FATAL, !insertion_result.second)
        << "Could not create sub-map for layer: (" << info.name
        << ", " << info.purpose << ")";
    name_iterator = insertion_result.first;
  }

  std::unordered_map<
      std::string, geometry::Layer> &sub_map = name_iterator->second;

  auto sub_it = sub_map.find(info.purpose);
  LOG_IF(FATAL, sub_it != sub_map.end())
      << "Duplicate layer name when adding layer info: (" << info.name
      << ", " << info.purpose << ")";
  sub_map.insert({info.purpose, layer});
}

const LayerInfo &PhysicalPropertiesDatabase::GetLayerInfo(
    const Layer &layer) const {
  auto iterator = layer_infos_.find(layer);
  LOG_IF(FATAL, iterator == layer_infos_.end())
      << "Layer info not found: " << layer;
  return iterator->second;
}

const LayerInfo &PhysicalPropertiesDatabase::GetLayerInfo(
    const std::string &layer_name_and_purpose) const {
  std::vector<std::string> name_parts =
      absl::StrSplit(layer_name_and_purpose, ".");

  LOG_IF(FATAL, name_parts.size() != 2)
      << "GetLayerInfo by name requires a layer in the form name.purpose.";

  std::string &name = name_parts.front();
  std::string &purpose = name_parts.back();

  auto name_it = layer_infos_by_name_.find(name);
  LOG_IF(FATAL, name_it == layer_infos_by_name_.end())
      << "No match for named layer: " << name;

  const std::unordered_map<std::string, geometry::Layer> &sub_map =
      name_it->second;
  auto sub_it = sub_map.find(purpose);
  LOG_IF(FATAL, sub_it == sub_map.end())
      << "No match for named layer (\"" <<  name << "\") purpose: " << purpose;

  return GetLayerInfo(sub_it->second);
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
