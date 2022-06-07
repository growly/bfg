#ifndef POLY_LINE_INFLATOR_H_
#define POLY_LINE_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "cell.h"
#include "physical_properties_database.h"
#include "poly_line_cell.h"
#include "geometry/line.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"
#include "geometry/polygon.h"
#include "geometry/via.h"

namespace bfg {

class PolyLineInflator {
 public:
  PolyLineInflator(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  // Return a laid-out version of the poly_line diagram.
  Cell Inflate(const PolyLineCell &poly_line_cell);

  void InflateVia(const geometry::Via &via, geometry::Rectangle *rectangle);
  void InflatePolyLine(const geometry::PolyLine &line,
                       geometry::Polygon *polygon);

 private:
  // Shift the given line consistently (relative to its bearing) by half the
  // 'width' amount. Add 'extension_source' to the start and 'extension_source'
  // to end of the line's length.
  geometry::Line *GenerateShiftedLine(
      const geometry::Line &source, double width,
      double extension_source, double extension_end);

  geometry::Line *GenerateShiftedLine(
      const geometry::Line &source, double width) {
    return GenerateShiftedLine(source, width, 0.0, 0.0);
  }

  // Shifts next_source by half of the given width, then add the intersection
  // of the new line with *last_shifted_line to polygon. Returns the newly
  // shifted line. If last_shifted_line is nullptr, the start of next_source is
  // used.
  std::unique_ptr<geometry::Line> ShiftAndAppendIntersection(
    const geometry::Line &next_source, double width,
    geometry::Line *last_shifted_line, geometry::Polygon *polygon);

  // Provides some defaults and rules.
  PhysicalPropertiesDatabase physical_db_;
};

}  // namespace bfg

#endif  // POLY_LINE_INFLATOR_H_
