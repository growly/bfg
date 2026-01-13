#ifndef POLY_LINE_INFLATOR_H_
#define POLY_LINE_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

#include "layout.h"
#include "physical_properties_database.h"
#include "poly_line_cell.h"
#include "geometry/line.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "geometry/polygon.h"
#include "abstract_via.h"

namespace bfg {

class RoutingGrid;

class PolyLineInflator {
 public:
  PolyLineInflator(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  // Return a laid-out version of the poly_line diagram.
  Layout *Inflate(const RoutingGrid &routing_grid,
                  const PolyLineCell &poly_line_cell);

  void InflateVia(const RoutingViaInfo &info,
                  const AbstractVia &via,
                  geometry::Rectangle *rectangle);
  void InflateVia(const geometry::Layer layer,
                  int64_t width,
                  int64_t height,
                  const AbstractVia &via,
                  geometry::Rectangle *rectangle);

  std::optional<geometry::Polygon> InflatePolyLine(
      const geometry::PolyLine &line);

  std::optional<geometry::Polygon> InflatePoint(
      const geometry::Point &point, int64_t horizontal, int64_t vertical);

 private:
  static void AppendIntersections(
    const std::vector<geometry::Line> &shifted_lines,
    geometry::Polygon *polygon);

  // Provides some defaults and rules.
  const PhysicalPropertiesDatabase &physical_db_;
};

}  // namespace bfg

#endif  // POLY_LINE_INFLATOR_H_
