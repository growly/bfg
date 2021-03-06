#ifndef PHYSICAL_PROPERTIES_DATABASE_H_
#define PHYSICAL_PROPERTIES_DATABASE_H_

#include <map>
#include <unordered_map>
#include <ostream>

#include "abstract_via.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"
#include "vlsir/tech.pb.h"

namespace bfg {

enum RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

struct RoutingLayerInfo {
  geometry::Layer layer;
  geometry::Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingTrackDirection direction;
  int64_t pitch;
};

struct ViaInfo {
  // Vias have their own layer.
  geometry::Layer layer;
  // Need some measure of cost for connecting between these two layers. Maybe
  // a function that describes the cost based on something (like length,
  // sheet resistance).
  double cost;
  int64_t width;
  int64_t height;

  // TODO(aryap): I'm not sure how this generalises.
  int64_t overhang;
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

  void AddRoutingLayerInfo(const RoutingLayerInfo &info);
  const RoutingLayerInfo &GetRoutingLayerInfo(
      const geometry::Layer &layer) const;

  void AddLayerInfo(const LayerInfo &info);
  const LayerInfo &GetLayerInfo(const geometry::Layer &layer) const;
  const LayerInfo &GetLayerInfo(
      const std::string &layer_name_and_purpose) const;

  void AddViaInfo(const geometry::Layer &lhs,
                  const geometry::Layer &rhs,
                  const ViaInfo &info);

  const ViaInfo &GetViaInfo(const AbstractVia &via) const {
    return GetViaInfo(via.bottom_layer(), via.top_layer());
  }
  const ViaInfo &GetViaInfo(
      const geometry::Layer &lhs, const geometry::Layer &rhs) const;

 private:
  double internal_units_per_external_;

  // Store routing information per layer, keyed by internal layer number.
  std::map<geometry::Layer, RoutingLayerInfo> routing_layer_infos_;

  // Stores the connection info between the ith (first index) and jth (second
  // index) layers. The "lesser" layer (std::less) should always be used to
  // index first, so that half of the matrix can be avoided.
  std::map<geometry::Layer, std::map<geometry::Layer, ViaInfo>> via_infos_;

  // Store a mapping of internal layer number to layer information.
  std::map<geometry::Layer, LayerInfo> layer_infos_;

  // Store a mapping of layer name to internal layer number.
  std::unordered_map<std::string, std::unordered_map<
      std::string, geometry::Layer>> layer_infos_by_name_;
};

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &physical_db);

}  // namespace bfg

#endif  // PHYSICAL_PROPERTIES_DATABASE_H_
