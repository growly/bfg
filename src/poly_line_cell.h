#ifndef POLY_LINE_CELL_H_
#define POLY_LINE_CELL_H_

#include <vector>
#include <memory>

#include "geometry/via.h"
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
  void AddPolyLine(const PolyLine &poly_line);
  PolyLine *AddPolyLine();

  const std::vector<std::unique_ptr<PolyLine>> &poly_lines() const {
    return poly_lines_;
  }
  const std::vector<std::unique_ptr<Via>> &vias() const { return vias_; }

  std::vector<std::unique_ptr<PolyLine>> &poly_lines() { return poly_lines_; }
  std::vector<std::unique_ptr<Via>> &vias() { return vias_; }

  const std::pair<Point, Point> GetBoundingBox() const;

 private:
  std::vector<std::unique_ptr<PolyLine>> poly_lines_;
  std::vector<std::unique_ptr<Via>> vias_;
};

}  // namespace bfg

#endif  // POLY_LINE_CELL_H_
