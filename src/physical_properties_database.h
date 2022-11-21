#ifndef PHYSICAL_PROPERTIES_DATABASE_H_
#define PHYSICAL_PROPERTIES_DATABASE_H_

#include <map>
#include <unordered_map>
#include <ostream>
#include <functional>
#include <optional>

#include "abstract_via.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"
#include "vlsir/tech.pb.h"

namespace bfg {

enum RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

struct RoutingViaInfo {
  // Vias have their own layer.
  geometry::Layer layer;
  // Need some measure of cost for connecting between these two layers. Maybe
  // a function that describes the cost based on something (like length,
  // sheet resistance).
  double cost;
  int64_t width;
  int64_t height;

  // TODO(aryap): I'm not sure how this generalises.
  int64_t overhang_length;
  int64_t overhang_width;
};

struct LayerInfo {
  geometry::Layer internal_layer;
  // Shorthand name, e.g. "met1".
  std::string name;
  // Shorthand purpose, e.g. "drawing".
  std::string purpose;

  uint16_t gds_layer;
  uint16_t gds_datatype;
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
  // but is not commutative necessarily, whereas until now I've
  // assumed they all are.
  int64_t min_enclosure;

  int64_t via_overhang;
  int64_t via_overhang_wide;
};


// Manages information about physical layout constraints.
//
// TODO(aryap): It seems that there will be so many parameters that designers
// will want to just use them by name, as would be the case with a simple
// python dict. We can emulate that of course, but it means that we no longer
// define some properties as explicit fields, like pitch, offset, wire_width,
// etc. A challenge with process-portability is mapping the different
// parameters in different processes doing the same thing to the same name, and
// so perhaps some of those should be made explicit and mapped. Unsure.
class PhysicalPropertiesDatabase {
 public:
  PhysicalPropertiesDatabase()
      : internal_units_per_external_(0.001) {}

  void LoadTechnology(const vlsir::tech::Technology &pdk);

  // Internally, all positions and lengths are computed in integer units.
  // Externally to this program, the user probably expects real units, like
  // mircons or nanometres (i.e. not yards or inches or anything stupid like
  // that). When setting up a process, we must define the conversion factor
  // between external and internal units.
  int64_t ToInternalUnits(const int64_t external_value) const {
    return external_value * internal_units_per_external_;
  }
  int64_t ToExternalUnits(const int64_t internal_value) const {
    return internal_value / internal_units_per_external_;
  }

  const geometry::Layer GetLayer(const std::string &name_and_purpose) const;
  std::optional<std::string> GetLayerNameAndPurpose(
      const geometry::Layer &layer) const;

  void AddLayerInfo(const LayerInfo &info);
  const LayerInfo &GetLayerInfo(const geometry::Layer &layer) const;
  const LayerInfo &GetLayerInfo(
      const std::string &layer_name_and_purpose) const;

  std::optional<const geometry::Layer> GetViaLayer(
      const std::string &left, const std::string &right) const;
  std::optional<const geometry::Layer> GetViaLayer(
      const geometry::Layer &left, const geometry::Layer &right) const;

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
  const IntraLayerConstraints &Rules(const std::string &layer_name) const;
  const IntraLayerConstraints &Rules(const geometry::Layer &layer) const;

  std::string DescribeLayers() const;

 private:
  double internal_units_per_external_;

  // Store a mapping of internal layer number to layer information.
  std::map<geometry::Layer, LayerInfo> layer_infos_;

  // Store a mapping of layer name to internal layer number.
  std::unordered_map<std::string, std::unordered_map<
      std::string, geometry::Layer>> layers_by_name_;
  std::map<geometry::Layer, std::pair<std::string, std::string>> layer_names_;

  std::unordered_map<geometry::Layer,
      std::unordered_map<geometry::Layer, InterLayerConstraints>>
      inter_layer_constraints_;

  // Stores the via layer required to get from the first indexed layer to the
  // second indexed layer.
  std::unordered_map<geometry::Layer,
      std::unordered_map<geometry::Layer, geometry::Layer>> via_layers_;

  std::unordered_map<geometry::Layer, IntraLayerConstraints>
      intra_layer_constraints_;

  const std::pair<geometry::Layer, geometry::Layer> GetTwoLayersAndSort(
      const std::string &left, const std::string &right) const;
};

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db);

}  // namespace bfg

#endif  // PHYSICAL_PROPERTIES_DATABASE_H_
