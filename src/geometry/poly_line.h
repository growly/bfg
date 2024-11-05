#ifndef GEOMETRY_POLY_LINE_H_
#define GEOMETRY_POLY_LINE_H_

#include <cstdint>
#include <memory>
#include <optional>

#include <gtest/gtest.h>

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
      : enable_narrowing_extensions_(false),
        start_({0, 0}), overhang_start_(0), overhang_end_(0),
        start_via_(nullptr), end_via_(nullptr),
        start_port_(nullptr), end_port_(nullptr) {
    if (points.empty()) return;

    start_ = points.front();
    for (size_t i = 1; i < points.size(); ++i) {
      AddSegment(points[i], 0);
    }
  }

  PolyLine(const PolyLine &other)
      : enable_narrowing_extensions_(false),
        start_(other.start_),
        overhang_start_(other.overhang_start_),
        overhang_end_(other.overhang_end_),
        start_via_(nullptr),  // TODO(aryap): Should these be copied?
        end_via_(nullptr),
        start_port_(nullptr),
        end_port_(nullptr),
        segments_(other.segments_.begin(), other.segments_.end()) {}

  // This constructor skips the segment sanity checks in AddSegment.
  PolyLine(const Point &start,
           const std::vector<LineSegment> &segments)
      : enable_narrowing_extensions_(false),
        start_(start), overhang_start_(0), overhang_end_(0),
        start_via_(nullptr), end_via_(nullptr),
        start_port_(nullptr), end_port_(nullptr),
        segments_(segments.begin(), segments.end()) {}

  std::string Describe() const;

  void MirrorY() override;
  void MirrorX() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Translate(const Point &offset) override;

  const Rectangle GetBoundingBox() const override;

  bool Intersects(const Point &point, size_t *segment_index) const;

  void AddSegment(const Point &point) {
    AddSegment(point, 0);
  }

  void AddSegment(const Point &to, const uint64_t width);

  std::optional<Line> LineAtPoint(const Point &point) const;

  // If the given point lands on an existing line in the PolyLine, do nothing.
  // Otherwise, extend the start or end point (whichever is closest) to include
  // the point, as long as it lands on the existing segment line.
  void ExtendToInclude(const Point &point);

  // Inserts what will become a rectangular bulge into the PolyLine by creating
  // the appropriate segments. The width and length are coaxial: width is
  // distance orthogonal to the direction of the line and length is parallel.
  //
  // We are deliberately taking a copy of the point we're given, because it's
  // possible (i.e. it happened once) that the reference be to some value in
  // this line that we're about to modify. E.g. if you
  // poly_line.InsertBulge(poly_line.End()), you would modify the underlying
  // value half way.
  void InsertBulge(
      const Point point, uint64_t coaxial_width, uint64_t coaxial_length);

  // As above, but will not be applied until ApplyDeferredBulges() call.
  void InsertBulgeLater(
      const Point point, uint64_t coaxial_width, uint64_t coaxial_length);

  void InsertBulge(
      const Point &point,
      uint64_t width,
      uint64_t length,
      std::optional<double> angle_rads);

  void ApplyDeferredBulges();

  void SetWidth(const uint64_t width);
  const std::vector<Point> Vertices() const;

  void set_start(const Point &start) { start_ = start; }
  const Point &start() const { return start_; }

  void set_min_separation(std::optional<int64_t> min_separation) {
      min_separation_ = min_separation;
  }

  const Point &End() const { return segments_.back().end; }

  uint64_t overhang_start() const { return overhang_start_; }
  void set_overhang_start(uint64_t overhang) { overhang_start_ = overhang; }

  uint64_t overhang_end() const { return overhang_end_; }
  void set_overhang_end(uint64_t overhang) { overhang_end_ = overhang; }

  AbstractVia *start_via() const { return start_via_; }
  void set_start_via(AbstractVia *via) { start_via_ = via; }
  AbstractVia *end_via() const { return end_via_; }
  void set_end_via(AbstractVia *via) { end_via_ = via; }

  // TODO(aryap): I don't think these port objects are needed.
  const Port *start_port() const { return start_port_; }
  void set_start_port(const Port *port) { start_port_ = port; }
  const Port *end_port() const { return end_port_; }
  void set_end_port(const Port *port) { end_port_ = port; }

  const std::vector<LineSegment> &segments() const { return segments_; }

 private:
  struct DeferredBulge {
    Point position;
    uint64_t width;
    uint64_t length;
  };

  void ReplaceDuplicateEndPointsWithWidest();
  void RemoveRedundantSegments();
  void RemoveNotchesInAStraightLine();
  void RemoveNotchesAroundCorners();

  void EnforceInvariants();
  void InsertForwardBulgePoint(
      const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
      size_t intersection_index, const Line &intersected_line);
  void InsertBackwardBulgePoint(
      const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
      size_t intersection_index, const Line &intersected_line,
      uint64_t intersected_previous_width);

  double ComputeRequiredLengthForLastSegmentWidth(
      uint64_t previous_segment_original_width,
      const Line &current_line,
      const Line &previous_line,
      double required_length);
  // Returns (required length, new width, sin theta, cos theta).
  std::tuple<double, uint64_t, double, double>
      ComputeRequiredLengthAndWidth(
          const Line &intersected_line,
          const Line &current_line,
          double on_axis_overflow,
          double off_axis_overflow);

  // It often doesn't make sense to create a protrusion from the line which
  // is narrower than the existing line just for a bulge, so we explicitly
  // disable it.
  bool enable_narrowing_extensions_;

  Point start_;

  // How much to extend the line over the start/end segments.
  uint64_t overhang_start_;
  uint64_t overhang_end_;
  std::optional<int64_t> min_separation_;

  // TODO(aryap): I can't figure out where to put this. This isn't exactly a
  // general solution - what about vias that connect part way through a
  // PolyLine? It seems that that problem is far more general and probably can
  // be avoided. At the very least, though, we need some way of making sure
  // that the inflated PolyLine satisfies the layout constraints of connecting
  // to vias at the start or end.
  // I don't think PolyeLine is the rigth place for via information. Some
  // aggregating data structure must do that.
  AbstractVia *start_via_;
  AbstractVia *end_via_;

  // TODO(aryap): Same with ports. The PolyLine might not connect between vias,
  // but there will be special rules to make sure it connects to a port,
  // whatever that is. It might be, for example, a via, or another rectangle on
  // the same layer which we need to overlap.
  const Port *start_port_;
  const Port *end_port_;

  std::vector<LineSegment> segments_;

  std::vector<DeferredBulge> deferred_bulges_;

  FRIEND_TEST(PolyLineTest, NotchAroundCorner);
  FRIEND_TEST(PolyLineTest, NotchAroundCorner2);
  FRIEND_TEST(PolyLineTest, NotchAroundCorner_Backwards);
};

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::PolyLine &poly_line);

}  // namespace bfg

#endif  // GEOMETRY_POLY_LINE_H_
