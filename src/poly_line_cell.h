#ifndef POLY_LINE_CELL_H_
#define POLY_LINE_CELL_H_

#include <vector>
#include <memory>

#include "abstract_via.h"
#include "geometry/point.h"
#include "geometry/poly_line.h"

namespace bfg {

// An abstract view of the cell. All rectangles are represented by PolyLines
// that contain start/end and width information. Connectivity between layers is
// captured by Vias, which contain a position and source/target layers. With
// physical properties at hand, these can be sensibly inflated into complete
// polygons. They are deliberately not stored as such.
class PolyLineCell {
 public:
  void AddPolyLine(const geometry::PolyLine &poly_line);
  geometry::PolyLine *AddPolyLine();

  const std::vector<std::unique_ptr<geometry::PolyLine>> &poly_lines() const {
    return poly_lines_;
  }
  const std::vector<std::unique_ptr<AbstractVia>> &vias() const {
    return vias_;
  }

  std::vector<std::unique_ptr<geometry::PolyLine>> &poly_lines() {
    return poly_lines_;
  }
  std::vector<std::unique_ptr<AbstractVia>> &vias() { return vias_; }

  const geometry::Rectangle GetBoundingBox() const;

 private:
  std::vector<std::unique_ptr<geometry::PolyLine>> poly_lines_;
  std::vector<std::unique_ptr<AbstractVia>> vias_;
};

}  // namespace bfg

#endif  // POLY_LINE_CELL_H_
