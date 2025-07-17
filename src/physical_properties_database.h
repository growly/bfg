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
  // providing acccess to. For example, "met1.pin" is used to access
  // "met1.drawing".
  std::optional<std::set<geometry::Layer>> accesses;

  // For non-PIN layers, we record which pin layers are used to access them.
  // For example, "met1.drawing" is accessed by "met1.pin".
  std::optional<std::set<geometry::Layer>> accessed_by;

  std::optional<std::set<geometry::Layer>> labels;

  // TODO(aryap): It might also be useful to record which label layers are used
  // for this layer. For example, "met1.label" is used to annotate "met1.pin"
  // and "met1.drawing".
  std::optional<std::set<geometry::Layer>> labelled_by;
};

struct IntraLayerConstraints {
  int64_t min_separation;
  int64_t min_width;
  int64_t min_pitch;
  int64_t min_area;

  int64_t via_width;
  int64_t via_height;
  double via_cost;
};

struct InterLayerConstraints {
  int64_t min_separation;
  int64_t max_separation;

  // There are rules on how the minimum extension of e.g. poly over diff and
  // diff over poly. Unfortunately we do not differentiate between the order of
  // the keys when looking up rules, so this asymmetry is not naturally
  // captured. We have to make up a convention for which relationship to store
  // in "min_enclosure" and which to store in "min_extension".
  int64_t min_enclosure;
  int64_t min_extension;

  // (Another way to express this is as min_enclosure on all sides and an
  // 'overhang' in one axis, obviating the need for one of these.)
  int64_t via_overhang;
  int64_t via_overhang_wide;

  // If the two layers described can be connected by a via, give the layer used
  // to create that via:
  // TODO(aryap): This doesn't seem that practical, since usually constraints
  // are specified between routing layers and via layers, not routing layers and
  // other routing layers.
  std::optional<geometry::Layer> connecting_via_layer;
};

struct ViaEncapInfo {
  // Size of the encap in the direction of the wire.
  int64_t length;
  // Size of the encap in the direction across the wire.
  int64_t width;
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
  // lambdas really are much nicer than this std::bind kerfuffle.
  static std::optional<std::vector<RoutingViaInfo>> FindViaStackImpl(
      const geometry::Layer &lhs,
      const geometry::Layer &rhs,
      // TODO(aryap): If I put the const in these function types to signify a
      // const member function (i.e. at the end of the signature), these don't
      // work. Huh?
      const std::function<
          std::vector<CostedLayer>(const geometry::Layer&)>
              &reachable_layers_fn,
      const std::function<
          RoutingViaInfo(
              const geometry::Layer&, const geometry::Layer&)>
                  &routing_via_info_fn);

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

  // This convenience function returns a rectangle in the shape of a
  // typically- (or minimally-)sized via-encap for the given layer.
  ViaEncapInfo TypicalViaEncap(
      const std::string &encap_layer_name,
      const std::string &via_layer_name) const;

  // TODO(aryap): Port these from RoutingGrid to here.
  // std::optional<double> FindViaStackCost(
  //     const geometry::Layer &lhs, const geometry::Layer &rhs) const;
  // Then put "BuildViaStack" or something in Layout.
  //
  // Since the RoutingGrid keeps its own subset of physical information for
  // routing these functions must live in duplicate; the RoutingGrid should
  // simply have its own instance of, and override properties of if necessary,
  // the PhysicalPropertiesDatabase. Then we can remove the duplicate code.
  //
  // This function is identical to RoutingGrid::FindViaStack, except that it
  // uses FindReachableLayersThroughOneVia instead of
  // RoutingGrid::LayersReachableByVia, because RoutingGrid has its own copy of
  // via information :/
  //
  // TODO(aryap): You should at least make the costed-reachability function a
  // parameter and factor the rest out into a reusable static function.
  std::optional<std::vector<RoutingViaInfo>> FindViaStack(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

  // For a given pin layer, find the layers which can access it. The pin layer
  // represents access to a given layer, which is the first entry. For each of
  // those we have a set of layers which can be accessed through one more via.
  const std::vector<std::pair<geometry::Layer, std::set<geometry::Layer>>>
      FindReachableLayersByPinLayer(const geometry::Layer &pin_layer) const;

  const std::set<geometry::Layer>
      FindLayersReachableThroughOneVia(const geometry::Layer &source_layer)
      const;

  const CostedLayer GetCostedLayer(const geometry::Layer &via_layer) const;

  const std::vector<CostedLayer> FindCostedLayersReachableThroughOneVia(
      const geometry::Layer &source_layer) const;

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
