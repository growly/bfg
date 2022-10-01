#include "physical_properties_database.h"

#include <algorithm>
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

void PhysicalPropertiesDatabase::LoadTechnology(
    const vlsir::tech::Technology &pdk) {
  Layer internal_layer = 0;
  for (const auto &layer_info : pdk.layers()) {
    // Find a free internal layer number:
    while (layer_infos_.find(internal_layer) != layer_infos_.end()) {
      internal_layer++;
      LOG_IF(FATAL, internal_layer == 0)
          << "Ran out of internal layer numbers!";
    }

    VLOG(3) << "Loading layer " << internal_layer << ": \"" << layer_info.name()
            << "\"";
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

const Layer PhysicalPropertiesDatabase::GetLayer(
    const std::string &name_and_purpose) const {
  std::vector<std::string> name_parts = absl::StrSplit(name_and_purpose, ".");

  LOG_IF(FATAL, name_parts.size() != 2)
      << "GetLayerInfo by name requires a layer in the form name.purpose.";

  std::string &name = name_parts.front();
  std::string &purpose = name_parts.back();

  auto name_it = layer_infos_by_name_.find(name);
  LOG_IF(FATAL, name_it == layer_infos_by_name_.end())
      << "No match for named layer: " << name;

  const std::unordered_map<std::string, Layer> &sub_map =
      name_it->second;
  auto sub_it = sub_map.find(purpose);
  LOG_IF(FATAL, sub_it == sub_map.end())
      << "No match for named layer (\"" <<  name << "\") purpose: " << purpose;
  return sub_it->second;
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
        std::unordered_map<std::string, Layer>()});
    LOG_IF(FATAL, !insertion_result.second)
        << "Could not create sub-map for layer: (" << info.name
        << ", " << info.purpose << ")";
    name_iterator = insertion_result.first;
  }

  std::unordered_map<
      std::string, Layer> &sub_map = name_iterator->second;

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
  const Layer layer = GetLayer(layer_name_and_purpose);
  return GetLayerInfo(layer);
}

void PhysicalPropertiesDatabase::AddRules(
    const std::string &first_layer,
    const std::string &second_layer,
    const InterLayerConstraints &constraints) {
  const auto layers = GetTwoLayersAndSort(first_layer, second_layer);
  inter_layer_constraints_[layers.first][layers.second] = constraints;
}

const InterLayerConstraints &PhysicalPropertiesDatabase::Rules(
    const std::string &left, const std::string &right) const {
  const auto layers = GetTwoLayersAndSort(left, right);
  const auto first_it = inter_layer_constraints_.find(layers.first);
  LOG_IF(FATAL, first_it == inter_layer_constraints_.end())
      << "No inter-layer constraints for " << left << "/" << right;
  const auto second_it = first_it->second.find(layers.second);
  LOG_IF(FATAL, second_it == first_it->second.end())
      << "No inter-layer constraints for " << left << "/" << right;
  return second_it->second;
}

void PhysicalPropertiesDatabase::AddRules(
    const std::string &layer_name,
    const IntraLayerConstraints &constraints) {
  const Layer &layer = GetLayer(layer_name);
  intra_layer_constraints_[layer] = constraints;
}

const IntraLayerConstraints &PhysicalPropertiesDatabase::Rules(
    const std::string &layer_name) const {
  const auto layer = GetLayer(layer_name);
  const auto map_it = intra_layer_constraints_.find(layer);
  LOG_IF(FATAL, map_it == intra_layer_constraints_.end())
      << "No intra-layer constraints for " << layer_name;
  return map_it->second;
}

const std::pair<Layer, Layer>
PhysicalPropertiesDatabase::GetTwoLayersAndSort(
    const std::string &left, const std::string &right) const {
  Layer first = GetLayer(left);
  Layer second = GetLayer(right);
  if (second < first) {
    std::swap(first, second);
  }
  return {first, second};
}

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db) {
  os << "Physical properties database";
  return os;
}

}   // namespace bfg
