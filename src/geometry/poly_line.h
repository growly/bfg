#ifndef POLY_LINE_H_
#define POLY_LINE_H_

#include <cstdint>
#include <memory>

#include "line_segment.h"
#include "point.h"
#include "port.h"
#include "shape.h"
#include "via.h"

namespace bfg {

class PolyLine : public Shape {
 public:
  PolyLine() = default;

  PolyLine(const std::vector<Point> &points) {
    if (points.empty()) return;

    start_ = points.front();
    for (size_t i = 1; i < points.size(); ++i) {
      segments_.push_back(LineSegment{points[i], 0});
    }
  }

  const std::pair<Point, Point> GetBoundingBox() const override;

  void AddSegment(const Point &point) {
    AddSegment(point, 0);
  }

  void AddSegment(const Point &to, const uint64_t width);

  void set_start(const Point &start) { start_ = start; }
  const Point &start() const { return start_; }

  uint64_t overhang_start() const { return overhang_start_; }
  void set_overhang_start(uint64_t overhang) { overhang_start_ = overhang; }

  uint64_t overhang_end() const { return overhang_end_; }
  void set_overhang_end(uint64_t overhang) { overhang_end_ = overhang; }

  Via *start_via() const { return start_via_; }
  void set_start_via(Via *via) { start_via_ = via; }
  Via *end_via() const { return end_via_; }
  void set_end_via(Via *via) { end_via_ = via; }

  const Port *start_port() const { return start_port_; }
  void set_start_port(const Port *port) { start_port_ = port; }
  const Port *end_port() const { return end_port_; }
  void set_end_port(const Port *port) { end_port_ = port; }

  const std::vector<LineSegment> &segments() const { return segments_; }

 private:
  Point start_;

  // How much to extend the line over the start/end segments.
  uint64_t overhang_start_;
  uint64_t overhang_end_;

  // TODO(aryap): I can't figure out where to put this. This isn't exactly a
  // general solution - what about vias that connect part way through a
  // PolyLine? It seems that that problem is far more general and probably can
  // be avoided. At the very least, though, we need some way of making sure
  // that the inflated PolyLine satisfies the layout constraints of connecting
  // to vias at the start or end.
  Via *start_via_;
  Via *end_via_;

  // TODO(aryap): Same with ports. The PolyLine might not connect between vias,
  // but there will be special rules to make sure it connects to a port,
  // whatever that is. It might be, for example, a via, or another rectangle on
  // the same layer which we need to overlap.
  const Port *start_port_;
  const Port *end_port_;

  std::vector<LineSegment> segments_;
};

}  // namespace bfg

#endif  // POLY_LINE_H_
