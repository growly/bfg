#include "physical_properties_database.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <optional>
#include <utility>
#include <glog/logging.h>
#include <string>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include "geometry/layer.h"
#include "routing_layer_info.h"
#include "routing_via_info.h"
#include "vlsir/tech.pb.h"

namespace bfg {

using geometry::Layer;

void PhysicalPropertiesDatabase::LoadTechnologyFromFile(
    const std::string &path) {
  vlsir::tech::Technology tech_pb;
  std::fstream technology_input(path, std::ios::in | std::ios::binary);
  LOG_IF(FATAL, !technology_input)
      << "Could not open technology protobuf " << std::quoted(path);
  if (!tech_pb.ParseFromIstream(&technology_input)) {
    LOG(FATAL) << "Could not parse technology protobuf, "
               << std::quoted(path);
  }
  LoadTechnology(tech_pb);
}

void PhysicalPropertiesDatabase::LoadTechnology(
    const vlsir::tech::Technology &pdk) {
  for (const auto &info_pb : pdk.layers()) {
    VLOG(3) << "Loading layer from proto: \"" << info_pb.name() << "\"";
    LayerInfo info {
        .name = info_pb.name(),
        .purpose = info_pb.purpose().description(),
        .gds_layer = static_cast<uint16_t>(info_pb.index().major()),
        .gds_datatype = static_cast<uint16_t>(info_pb.index().minor())
    };
    AddLayerInfo(info);
  }

  // Do a second pass, translating any references into their internal layer
  // number.
  for (const auto &info_pb : pdk.layers()) {
    std::optional<Layer> layer = FindLayer(info_pb.index().major(),
                                           info_pb.index().minor());
    LOG_IF(FATAL, !layer)
        << "Layer should have been mapped on first pass through technology: "
        << info_pb.index().major() << "/" << info_pb.index().minor();
    auto it = layer_infos_.find(layer.value());
    LOG_IF(FATAL, it == layer_infos_.end())
        << "Layer info should have been created by 2nd pass: " << layer.value();
    auto &layer_info = it->second;

    for (const auto &ref_key : info_pb.pin_access_for()) {
      std::optional<Layer> access_layer = FindLayer(ref_key.major(),
                                                    ref_key.minor());
      LOG_IF(FATAL, !access_layer)
          << "Reference for pin access not added on first pass through "
          << "technology: "
          << ref_key.major() << "/"
          << ref_key.minor();
      if (!layer_info.accesses) {
        layer_info.accesses = std::set<Layer>{*access_layer};
      } else {
        layer_info.accesses->insert(*access_layer);
      }
      // Add back-reference.
      auto access_it = layer_infos_.find(*access_layer);
      auto &access_layer_info = access_it->second;
      if (!access_layer_info.accessed_by) {
        access_layer_info.accessed_by = std::set<Layer>{*layer};
      } else {
        access_layer_info.accessed_by->insert(*layer);
      }
    }

    for (const auto &ref_key : info_pb.labels()) {
      std::optional<Layer> target = FindLayer(ref_key.major(), ref_key.minor());
  
      LOG_IF(FATAL, !target)
          << "Layer "
          << info_pb.index().major() << "/" << info_pb.index().minor()
          << " labels another layer which was not found: "
          << ref_key.major() << "/" << ref_key.minor();

      if (!layer_info.labels) {
        layer_info.labels = std::set<Layer>{*target};
      } else {
        layer_info.labels->insert(*target);
      }

      // Add back-reference.
      auto target_it = layer_infos_.find(*target);
      auto &target_layer_info = target_it->second;
      if (!target_layer_info.labelled_by) {
        target_layer_info.labelled_by = std::set<Layer>{*layer};
      } else {
        target_layer_info.labelled_by->insert(*layer);
      }
    }
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

std::optional<const Layer> PhysicalPropertiesDatabase::FindLayer(
    const std::string &name) const {
  auto name_it = layers_by_name_.find(name);
  if (name_it == layers_by_name_.end())
    return std::nullopt;
  return name_it->second;
}

const Layer PhysicalPropertiesDatabase::GetLayer(
    const std::string &name) const {
  std::optional<const Layer> layer = FindLayer(name);

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

  layers_by_layer_key_[info.gds_layer][info.gds_datatype] = layer;

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

void PhysicalPropertiesDatabase::AddViaLayer(
    const std::string &one_layer,
    const std::string &another_layer,
    const std::string &via_layer) {
  AddViaLayer(GetLayer(one_layer),
              GetLayer(another_layer),
              GetLayer(via_layer));
}

void PhysicalPropertiesDatabase::AddViaLayer(
    const geometry::Layer &one_layer,
    const geometry::Layer &another_layer,
    const geometry::Layer &via_layer) {
  std::optional<const Layer> existing = GetViaLayer(one_layer, another_layer);
  if (existing) {
    LOG_IF(FATAL, existing.value() != via_layer)
        << "Layer " << DescribeLayer(one_layer) << " and "
        << DescribeLayer(another_layer)
        << " are already connected by via layer " << DescribeLayer(via_layer);
    // If the existing layer matches, do nothing.
    return;
  }

  const auto layers = OrderLayers(one_layer, another_layer);
  via_layers_[layers.first][layers.second] = via_layer;
}

std::optional<const Layer> PhysicalPropertiesDatabase::GetViaLayer(
    const std::string &left, const std::string &right) const {
  const auto layers = GetTwoLayersAndOrder(left, right);
  auto outer_it = via_layers_.find(layers.first);
  if (outer_it == via_layers_.end())
    return std::nullopt;
  auto inner_it = outer_it->second.find(layers.second);
  if (inner_it == outer_it->second.end())
    return std::nullopt;
  return inner_it->second;
}

std::optional<const Layer> PhysicalPropertiesDatabase::GetViaLayer(
      const Layer &left, const Layer &right) const {
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
  const auto layers = GetTwoLayersAndOrder(first_layer, second_layer);
  inter_layer_constraints_[layers.first][layers.second] = constraints;

  if (constraints.connecting_via_layer) {
    AddViaLayer(layers.first,
                layers.second,
                constraints.connecting_via_layer.value());
  }
}

const InterLayerConstraints &PhysicalPropertiesDatabase::Rules(
    const std::string &left, const std::string &right) const {
  return Rules(GetLayer(left), GetLayer(right));
}

const InterLayerConstraints &PhysicalPropertiesDatabase::Rules(
    const geometry::Layer &left, const geometry::Layer &right) const {
  const auto layers = OrderLayers(left, right);
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

std::optional<RoutingLayerInfo> PhysicalPropertiesDatabase::GetRoutingLayerInfo(
    const std::string &routing_layer_name) const {
  auto layer = FindLayer(routing_layer_name);
  if (!layer) {
    return std::nullopt;
  }
  RoutingLayerInfo routing_info;
  routing_info.set_layer(*layer);
  auto maybe_rules = GetRules(*layer);
  if (!maybe_rules) {
    LOG(WARNING) << "No intra-layer constraints for layer " << *layer
                 << " (" << routing_layer_name << ")";
    return std::nullopt;
  }
  const IntraLayerConstraints &layer_rules = maybe_rules->get();
  routing_info.set_wire_width(layer_rules.min_width);
  routing_info.set_pitch(layer_rules.min_pitch);
  routing_info.set_min_separation(layer_rules.min_separation);

  const LayerInfo &layer_info = GetLayerInfo(*layer);
  if (layer_info.accessed_by) {
    for (const geometry::Layer &pin_layer : *layer_info.accessed_by) {
      routing_info.set_pin_layer(pin_layer);
    }
  }

  return routing_info;
}

RoutingLayerInfo PhysicalPropertiesDatabase::GetRoutingLayerInfoOrDie(
    const std::string &routing_layer_name) const {
  auto routing_info = GetRoutingLayerInfo(routing_layer_name);
  return *routing_info;
}

std::optional<RoutingViaInfo> PhysicalPropertiesDatabase::GetRoutingViaInfo(
    const std::string &routing_layer,
    const std::string &other_routing_layer) const {
  auto first_layer = FindLayer(routing_layer);
  auto second_layer = FindLayer(other_routing_layer);
  if (!first_layer || !second_layer) {
    return std::nullopt;
  }
  return GetRoutingViaInfoOrDie(*first_layer, *second_layer);
}

RoutingViaInfo PhysicalPropertiesDatabase::GetRoutingViaInfoOrDie(
    const std::string &routing_layer,
    const std::string &other_routing_layer) const {
  const geometry::Layer first_layer = GetLayer(routing_layer);
  const geometry::Layer second_layer = GetLayer(other_routing_layer);
  return GetRoutingViaInfoOrDie(first_layer, second_layer);
}

RoutingViaInfo PhysicalPropertiesDatabase::GetRoutingViaInfoOrDie(
    const geometry::Layer &first_layer,
    const geometry::Layer &second_layer) const {
  auto via_layer = GetViaLayer(first_layer, second_layer);
  LOG_IF(FATAL, !via_layer)
      << "No via layer found connecting " << first_layer << " and "
      << second_layer;

  const IntraLayerConstraints &via_rules = Rules(*via_layer);

  RoutingViaInfo routing_via_info;
  routing_via_info.set_layer(*via_layer);
  routing_via_info.set_width(via_rules.via_width);
  routing_via_info.set_height(via_rules.via_width);

  const InterLayerConstraints &via_to_first_layer_rules =
      Rules(first_layer, *via_layer);
  RoutingViaEncapInfo first_layer_encap = {0};
  first_layer_encap.overhang_length = via_to_first_layer_rules.via_overhang;
  first_layer_encap.overhang_width = via_to_first_layer_rules.via_overhang_wide;

  const InterLayerConstraints &via_to_second_layer_rules =
      Rules(second_layer, *via_layer);
  RoutingViaEncapInfo second_layer_encap = {0};
  second_layer_encap.overhang_length = via_to_second_layer_rules.via_overhang;
  second_layer_encap.overhang_width =
      via_to_second_layer_rules.via_overhang_wide;

  routing_via_info.AddRoutingViaEncapInfo(first_layer, first_layer_encap);
  routing_via_info.AddRoutingViaEncapInfo(second_layer, second_layer_encap);
  return routing_via_info;
}

//                    7    --------- some routing layer
//          accesses /         ^
//                  /          | connected through some via layer
//   pin layer ----------      |                   L
//                             | ---------- the via layer
//                             |
//                             v
//                         --------- some other routing layer
const std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
PhysicalPropertiesDatabase::FindReachableLayersByPinLayer(
    const geometry::Layer &pin_layer) const {
  std::set<geometry::Layer> accessible;
  const auto &layer_info = GetLayerInfo(pin_layer);
  std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      accessibility_by_access_layer;
  if (!layer_info.accesses) {
    return accessibility_by_access_layer;
  }
  for (const auto &directly_accessible_layer : *layer_info.accesses) {
    std::set<geometry::Layer> accessible_through_at_most_one_via =
        FindLayersReachableThroughOneViaFrom(directly_accessible_layer);
    accessible_through_at_most_one_via.insert(directly_accessible_layer);
    accessibility_by_access_layer.push_back(
        {directly_accessible_layer, accessible_through_at_most_one_via});
  }
  return accessibility_by_access_layer;;
}

const std::set<geometry::Layer>
PhysicalPropertiesDatabase::FindLayersReachableThroughOneViaFrom(
    const geometry::Layer &routing_layer) const {
  // via_layers_ is indexed by two layers. Each entry indicates that the layers
  // in the index pair are connected by a via on the layer contained at that
  // position.
  std::set<geometry::Layer> accessible;
  for (const auto &outer : via_layers_) {
    const geometry::Layer &outer_layer = outer.first;
    for (const auto &inner : outer.second) {
      const geometry::Layer &inner_layer = inner.first;
      if (outer_layer == routing_layer) {
        accessible.insert(inner_layer);
      } else if (inner_layer == routing_layer) {
        accessible.insert(outer_layer);
      }
    }
  }
  return accessible;
}

std::string PhysicalPropertiesDatabase::DescribeLayers() const {
  std::stringstream ss;
  ss << "Physical properties database layer information:" << std::endl;
  for (const auto &entry : layer_names_) {
    const Layer &layer = entry.first;
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
    const Layer &layer = entry.second;
    std::string canonical_name = GetLayerName(layer).value();
    ss << absl::StrFormat("%-30s: %u (%s)\n", name, layer, canonical_name);
  }
  return ss.str();
}


const std::pair<geometry::Layer, geometry::Layer>
PhysicalPropertiesDatabase::OrderLayers(
    const geometry::Layer &one, const geometry::Layer &another) const {
  Layer first = one;
  Layer second = another;
  if (second < first) {
    std::swap(first, second);
  }
  return {first, second};
}

const std::pair<Layer, Layer>
PhysicalPropertiesDatabase::GetTwoLayersAndOrder(
    const std::string &left, const std::string &right) const {
  return OrderLayers(GetLayer(left), GetLayer(right));
}

Layer PhysicalPropertiesDatabase::GetNextInternalLayer() {
  Layer next = next_internal_layer_;
  ++next_internal_layer_;
  LOG_IF(FATAL, next_internal_layer_ == 0)
      << "Ran out of internal layer numbers!";
  return next;
}

std::optional<Layer> PhysicalPropertiesDatabase::FindLayer(
    uint16_t gds_layer, uint16_t gds_datatype) {
  auto outer_it = layers_by_layer_key_.find(gds_layer);
  if (outer_it == layers_by_layer_key_.end()) {
    return std::nullopt;
  }

  std::map<uint16_t, geometry::Layer> &inner = outer_it->second;
  auto inner_it = inner.find(gds_datatype);
  if (inner_it == inner.end()) {
    return std::nullopt;
  }
  return inner_it->second;
}

std::string PhysicalPropertiesDatabase::DescribeLayer(
    const geometry::Layer &layer) const {
  std::stringstream ss;
  auto name = GetLayerName(layer);
  ss << layer;
  if (name) {
    ss << " (" << name.value() << ")";
  }
  return ss.str();
}

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db) {
  os << "Physical properties database";
  return os;
}

}   // namespace bfg
