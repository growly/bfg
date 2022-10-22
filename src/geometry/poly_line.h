#ifndef GEOMETRY_POLY_LINE_H_
#define GEOMETRY_POLY_LINE_H_

#include <cstdint>
#include <memory>

#include "line_segment.h"
#include "point.h"
#include "port.h"
#include "shape.h"
#include "../abstract_via.h"

namespace bfg {

namespace geometry {

class PolyLine : public Shape {
 public:
  PolyLine() = default;

  PolyLine(const std::vector<Point> &points)
      : start_({0, 0}), overhang_start_(0), overhang_end_(0),
        start_via_(nullptr), end_via_(nullptr),
        start_port_(nullptr), end_port_(nullptr) {
    if (points.empty()) return;

    start_ = points.front();
    for (size_t i = 1; i < points.size(); ++i) {
      segments_.push_back(LineSegment{points[i], 0});
    }
  }

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;

  const Rectangle GetBoundingBox() const override;

  void AddSegment(const Point &point) {
    AddSegment(point, 0);
  }

  void AddSegment(const Point &to, const uint64_t width);

  void SetWidth(const uint64_t width);
  const std::vector<Point> Vertices() const;

  void set_start(const Point &start) { start_ = start; }
  const Point &start() const { return start_; }

  uint64_t overhang_start() const { return overhang_start_; }
  void set_overhang_start(uint64_t overhang) { overhang_start_ = overhang; }

  uint64_t overhang_end() const { return overhang_end_; }
  void set_overhang_end(uint64_t overhang) { overhang_end_ = overhang; }

  AbstractVia *start_via() const { return start_via_; }
  void set_start_via(AbstractVia *via) { start_via_ = via; }
  AbstractVia *end_via() const { return end_via_; }
  void set_end_via(AbstractVia *via) { end_via_ = via; }

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
  AbstractVia *start_via_;
  AbstractVia *end_via_;

  // TODO(aryap): Same with ports. The PolyLine might not connect between vias,
  // but there will be special rules to make sure it connects to a port,
  // whatever that is. It might be, for example, a via, or another rectangle on
  // the same layer which we need to overlap.
  const Port *start_port_;
  const Port *end_port_;

  std::vector<LineSegment> segments_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_POLY_LINE_H_
