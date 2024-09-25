#ifndef PHYSICAL_PROPERTIES_DATABASE_H_
#define PHYSICAL_PROPERTIES_DATABASE_H_

#include <map>
#include <unordered_map>
#include <ostream>
#include <functional>
#include <optional>

#include "routing_track_direction.h"
#include "geometry/layer.h"
#include "routing_via_info.h"
#include "vlsir/tech.pb.h"

namespace bfg {

class RoutingLayerInfo;

struct LayerInfo {
  geometry::Layer internal_layer;
  // Shorthand name, e.g. "met1".
  std::string name;
  // Shorthand purpose, e.g. "drawing".
  std::string purpose;

  uint16_t gds_layer;
  uint16_t gds_datatype;

  // For PIN layers in particular, we need to record which layer(s) they're
  // providing acccess to:
  std::optional<std::set<geometry::Layer>> accesses;
};

struct IntraLayerConstraints {
  int64_t min_separation;
  int64_t min_width;
  int64_t min_pitch;
  int64_t min_area;

  int64_t via_width;
  int64_t via_height;
};

struct InterLayerConstraints {
  int64_t min_separation;
  int64_t max_separation;

  // TODO(aryap): An 'enclosure' rule seems more descriptive than via overhang
  // but does not apply equally on all sides usually.
  int64_t min_enclosure;

  // Another way to express this is as min_enclosure on all sides and an
  // 'overhang' in one axis, obviating the need for one of these.
  int64_t via_overhang;
  int64_t via_overhang_wide;

  // If the two layers described can be connected by a via, give the layer used
  // to create that via:
  // TODO(aryap): This doesn't seem that practical, since usually constraints
  // are specified between routing layers and via layers, not routing layers and
  // other routing layers.
  std::optional<geometry::Layer> connecting_via_layer;
};

// TODO(aryap): We have primitive rules, like those above, and we have
// synthesised rules, that come from some combination of those primitive rules.
// We need a SynthesisedConstraints class or equivalent to organise and capture
// these. This is currently managed by creating the entries in the structs and
// manually computing them when the PDK is loaded, but could usefully be a
// user-defined callback or something.

// Manages information about physical layout constraints.
//
// TODO(aryap): It seems that there will be so many parameters that designers
// will want to just use them by name, as would be the case with a simple
// python dict. We can emulate that of course, but it means that we no longer
// define some properties as explicit fields, like pitch, offset, wire_width,
// etc. A challenge with process-portability is mapping the different
// parameters in different processes doing the same thing to the same name, and
// so perhaps some of those should be made explicit and mapped. Unsure.
//
// TODO(aryap): Handling virtual layers:
// Sometimes the same GDS layer behaves differently depending on its usage. We
// can model this with separate internal layers whose rules can be
// distinguished in the natural way. All that is needed is for the two layers
// to map to the same GDS layer at the end. Their treatment internally can be
// virtual. This can also be captured by the relationship properties of
// InterLayerConstraints.
class PhysicalPropertiesDatabase {
 public:
  PhysicalPropertiesDatabase()
      : internal_units_per_external_(0.001),
        next_internal_layer_(0) {}

  void LoadTechnology(const vlsir::tech::Technology &pdk);

  void LoadTechnologyFromFile(const std::string &path);

  // Internally, all positions and lengths are computed in integer units,
  // meaning that truncation and rounding have to be considered when it is
  // prudent to do so.
  int64_t ToInternalUnits(const int64_t external_value) const {
    return external_value * internal_units_per_external_;
  }
  int64_t ToSquareInternalUnits(const int64_t external_square_value) const {
    return external_square_value *
        internal_units_per_external_ * internal_units_per_external_;
  }
  int64_t ToExternalUnits(const int64_t internal_value) const {
    return internal_value / internal_units_per_external_;
  }

  void AddLayerAlias(
      const std::string &alias,
      const std::string &name);

  const geometry::Layer GetLayer(const std::string &name) const;
  std::optional<const geometry::Layer> FindLayer(
      const std::string &name) const;
  std::optional<std::string> GetLayerName(const geometry::Layer &layer) const;

  void AddLayerInfo(const LayerInfo &info);
  const LayerInfo &GetLayerInfo(const geometry::Layer &layer) const;
  const LayerInfo &GetLayerInfo(const std::string &layer_name) const;

  std::optional<const geometry::Layer> GetViaLayer(
      const std::string &left, const std::string &right) const;
  std::optional<const geometry::Layer> GetViaLayer(
      const geometry::Layer &left, const geometry::Layer &right) const;
  void AddViaLayer(const std::string &one_layer,
                   const std::string &another_layer,
                   const std::string &via_layer);
  void AddViaLayer(const geometry::Layer &one_layer,
                   const geometry::Layer &another_layer,
                   const geometry::Layer &via_layer);

  void AddRules(const std::string &first_layer,
                const std::string &second_layer,
                const InterLayerConstraints &constraints);

  void AddRules(const std::string &layer_name,
                const IntraLayerConstraints &constraints);

  std::optional<std::reference_wrapper<const IntraLayerConstraints>>
      GetRules(const geometry::Layer &layer) const;

  // These look up methods are fatal if the given layer is not found.
  const InterLayerConstraints &Rules(
      const std::string &left, const std::string &right) const;
  const InterLayerConstraints &Rules(
      const geometry::Layer &left, const geometry::Layer &right) const;
  const IntraLayerConstraints &Rules(const std::string &layer_name) const;
  const IntraLayerConstraints &Rules(const geometry::Layer &layer) const;

  RoutingLayerInfo GetRoutingLayerInfoOrDie(
      const std::string &routing_layer_name) const;
  std::optional<RoutingLayerInfo> GetRoutingLayerInfo(
      const std::string &routing_layer_name) const;

  std::optional<RoutingViaInfo> GetRoutingViaInfo(
      const std::string &routing_layer,
      const std::string &other_routing_layer) const;
  RoutingViaInfo GetRoutingViaInfoOrDie(
      const std::string &routing_layer,
      const std::string &other_routing_layer) const;
  RoutingViaInfo GetRoutingViaInfoOrDie(
      const geometry::Layer &first_layer,
      const geometry::Layer &second_layer) const;

  // TODO(aryap): Port these from RoutingGrid to here.
  // std::optional<double> FindViaStackCost(
  //     const geometry::Layer &lhs, const geometry::Layer &rhs) const;
  // std::optional<std::vector<RoutingViaInfo>> FindViaStack(
  //     const geometry::Layer &lhs, const geometry::Layer &rhs) const;
  //
  // This is the same as "FindLayersReachableThroughOneViaFrom", except that it
  // returns a set of layers with costs.
  //  std::vector<CostedLayer> LayersReachableByVia(
  //      const geometry::Layer &from_layer) const;
  // Then put "BuildViaStack" or something in Layout.

  // For a given pin layer, find the layers which can access it. The pin layer
  // represents access to a given layer, which is the first entry. For each of
  // those we have a set of layers which can be accessed through one more via.
  const std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      FindReachableLayersByPinLayer(const geometry::Layer &pin_layer) const;

  const std::set<geometry::Layer>
      FindLayersReachableThroughOneViaFrom(const geometry::Layer &routing_layer)
      const;

  std::string DescribeLayers() const;
  std::string DescribeLayer(const geometry::Layer &layer) const;

  void set_internal_units_per_external(double new_value) {
    internal_units_per_external_ = new_value;
  }
  double internal_units_per_external() const {
    return internal_units_per_external_;
  }

 private:
  geometry::Layer GetNextInternalLayer();

  std::optional<geometry::Layer> FindLayer(
      uint16_t gds_layer, uint16_t gds_datatype);

  double internal_units_per_external_;

  geometry::Layer next_internal_layer_;

  // Store a mapping of internal layer number to layer information.
  std::map<geometry::Layer, LayerInfo> layer_infos_;

  // Store a mapping of layer name to internal layer number.
  std::unordered_map<std::string, geometry::Layer> layers_by_name_;
  std::map<geometry::Layer, std::string> layer_names_;

  std::unordered_map<geometry::Layer,
      std::unordered_map<geometry::Layer, InterLayerConstraints>>
      inter_layer_constraints_;

  // Also store a mapping of GDS layer number/datatype (a LayerKey in the proto)
  // to the internal layer number.
  std::map<uint16_t, std::map<uint16_t, geometry::Layer>> layers_by_layer_key_;

  // Stores the via layer required to get from the first indexed layer to the
  // second indexed layer. If an via layer exists between two layers, we assume
  // that those layers can be connected through that single via layer. If no
  // entry exists, we take that to mean that two layers cannot be directly
  // connected by a via.
  std::map<geometry::Layer,
      std::map<geometry::Layer, geometry::Layer>> via_layers_;

  std::unordered_map<geometry::Layer, IntraLayerConstraints>
      intra_layer_constraints_;

  const std::pair<geometry::Layer, geometry::Layer> OrderLayers(
      const geometry::Layer &one, const geometry::Layer &another) const;
  const std::pair<geometry::Layer, geometry::Layer> GetTwoLayersAndOrder(
      const std::string &left, const std::string &right) const;
};

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db);

}  // namespace bfg

#endif  // PHYSICAL_PROPERTIES_DATABASE_H_
