#include "physical_properties_database.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <optional>
#include <utility>
#include <glog/logging.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include "geometry/layer.h"
#include "vlsir/tech.pb.h"

namespace bfg {

using geometry::Layer;

void PhysicalPropertiesDatabase::LoadTechnology(
    const vlsir::tech::Technology &pdk) {
  for (const auto &layer_info : pdk.layers()) {

    VLOG(3) << "Loading layer from proto: \"" << layer_info.name() << "\"";
    LayerInfo info {
        .name = layer_info.name(),
        .purpose = layer_info.purpose().description(),
        .gds_layer = static_cast<uint16_t>(layer_info.index()),
        .gds_datatype = static_cast<uint16_t>(layer_info.sub_index())
    };
    AddLayerInfo(info);
  }
}

void PhysicalPropertiesDatabase::AddLayerAlias(
    const std::string &alias, const std::string &name) {
  auto alias_check = FindLayer(alias);
  LOG_IF(FATAL, alias_check)
      << "Cannot use " << alias << " as a layer alias, it already exists";
  auto target = FindLayer(name);
  LOG_IF(FATAL, !target)
      << "Cannot create alias, target layer does not exist: "
      << name;

  layers_by_name_.insert({alias, target.value()});
}

std::optional<const geometry::Layer> PhysicalPropertiesDatabase::FindLayer(
    const std::string &name) const {
  auto name_it = layers_by_name_.find(name);
  if (name_it == layers_by_name_.end())
    return std::nullopt;
  return name_it->second;
}

const Layer PhysicalPropertiesDatabase::GetLayer(
    const std::string &name) const {
  std::optional<const geometry::Layer> layer = FindLayer(name);

  LOG_IF(FATAL, !layer) << "Could not find layer: " << name;

  return *layer;
}

std::optional<std::string> PhysicalPropertiesDatabase::GetLayerName(
    const Layer &layer) const {
  auto it = layer_names_.find(layer);
  if (it == layer_names_.end()) {
    return std::nullopt;
  }
  return it->second;
}

void PhysicalPropertiesDatabase::AddLayerInfo(const LayerInfo &info) {
  // Find a free internal layer number:
  Layer layer = GetNextInternalLayer();
  while (layer_infos_.find(layer) != layer_infos_.end()) {
    layer = GetNextInternalLayer();
  }

  LayerInfo copy = info;
  copy.internal_layer = layer;

  // Add to mapping by layer number.
  auto number_iterator = layer_infos_.find(layer);
  LOG_IF(FATAL, number_iterator != layer_infos_.end())
      << "Duplicate layer info: " << layer;
  layer_infos_.insert({layer, copy});

  std::string internal_name = absl::StrCat(copy.name, ".", copy.purpose);

  auto name_iterator = layers_by_name_.find(internal_name);
  LOG_IF(FATAL, name_iterator != layers_by_name_.end())
      << "Duplicate internal name for layer: " << internal_name;

  layer_names_.insert({layer, internal_name});
  layers_by_name_.insert({internal_name, layer});

  VLOG(3) << "Added layer " << layer << ", name: " << info.name
          << ", purpose: " << info.purpose;
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
  const Layer layer = GetLayer(layer_name);
  return GetLayerInfo(layer);
}

std::optional<const Layer> PhysicalPropertiesDatabase::GetViaLayer(
      const std::string &left, const std::string &right) const {
  const auto layers = GetTwoLayersAndSort(left, right);
  auto outer_it = via_layers_.find(layers.first);
  if (outer_it == via_layers_.end())
    return std::nullopt;
  auto inner_it = outer_it->second.find(layers.second);
  if (inner_it == outer_it->second.end())
    return std::nullopt;
  return inner_it->second;
}

std::optional<const Layer> PhysicalPropertiesDatabase::GetViaLayer(
      const geometry::Layer &left, const geometry::Layer &right) const {
  std::pair<const Layer&, const Layer&> ordered_layers =
      geometry::OrderFirstAndSecondLayers(left, right);
  auto outer_it = via_layers_.find(ordered_layers.first);
  if (outer_it == via_layers_.end())
    return std::nullopt;
  auto inner_it = outer_it->second.find(ordered_layers.second);
  if (inner_it == outer_it->second.end())
    return std::nullopt;
  return inner_it->second;
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
  return Rules(layer);
}

const IntraLayerConstraints &PhysicalPropertiesDatabase::Rules(
    const Layer &layer) const {
  auto rules = GetRules(layer);
  if (!rules) {
    std::optional<std::string> result = GetLayerName(layer);
    std::string name = result ? *result : "unknown";
    LOG(FATAL)
        << "No intra-layer constraints for layer " << layer
        << " (" << name << ")";
  }
  return *rules;
}

std::optional<std::reference_wrapper<const IntraLayerConstraints>>
PhysicalPropertiesDatabase::GetRules(
    const Layer &layer) const {
  const auto map_it = intra_layer_constraints_.find(layer);
  if (map_it == intra_layer_constraints_.end()) {
    return std::nullopt;
  }
  return map_it->second;
}

std::string PhysicalPropertiesDatabase::DescribeLayers() const {
  std::stringstream ss;
  ss << "Physical properties database layer information:" << std::endl;
  for (const auto &entry : layer_names_) {
    const geometry::Layer &layer = entry.first;
    const std::string &name = entry.second;
    const uint16_t gds_layer = GetLayerInfo(layer).gds_layer;
    const uint16_t gds_datatype = GetLayerInfo(layer).gds_datatype;
    ss << absl::StrFormat("%10d %-30s %10u %10u\n",
                          layer, name,
                          gds_layer, gds_datatype);
  }

  ss << "\nLayer name to layer mapping: (" << layers_by_name_.size() << ")"
     << std::endl;
  for (const auto &entry : layers_by_name_) {
    const std::string &name = entry.first;
    const geometry::Layer &layer = entry.second;
    std::string canonical_name = GetLayerName(layer).value();
    ss << absl::StrFormat("%-30s: %u (%s)\n", name, layer, canonical_name);
  }
  return ss.str();
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

Layer PhysicalPropertiesDatabase::GetNextInternalLayer() {
  Layer next = next_internal_layer_;
  ++next_internal_layer_;
  LOG_IF(FATAL, next_internal_layer_ == 0)
      << "Ran out of internal layer numbers!";
  return next;
}

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db) {
  os << "Physical properties database";
  return os;
}

}   // namespace bfg
