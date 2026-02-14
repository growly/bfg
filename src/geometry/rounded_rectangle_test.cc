#include <gtest/gtest.h>
#include <glog/logging.h>

#include "rounded_rectangle.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {
namespace {

// A rounded rectangle centred at the origin with known dimensions for testing.
// The bounding box is (-100, -50) to (100, 50), corner radius 10.
RoundedRectangle MakeTestRoundedRect() {
  return RoundedRectangle(Point(-100, -50), Point(100, 50), 10);
}

// --- Overlaps(const Rectangle &) tests ---

TEST(RoundedRectangleTest, OverlapsRectangle_NoOverlap) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Entirely outside, to the right.
  Rectangle far_right(Point(200, 0), Point(300, 50));
  EXPECT_FALSE(rr.Overlaps(far_right));
}

TEST(RoundedRectangleTest, OverlapsRectangle_ClearOverlapInCentre) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Clearly overlapping the centre region.
  Rectangle centre_overlap(Point(-10, -10), Point(10, 10));
  EXPECT_TRUE(rr.Overlaps(centre_overlap));
}

TEST(RoundedRectangleTest, OverlapsRectangle_OverlapInLeftRegion) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Overlaps the left side strip (x < inner_lower_left.x, within y range).
  Rectangle left_overlap(Point(-105, -10), Point(-95, 10));
  EXPECT_TRUE(rr.Overlaps(left_overlap));
}

TEST(RoundedRectangleTest, OverlapsRectangle_OverlapInRightRegion) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Overlaps the right side strip.
  Rectangle right_overlap(Point(95, -10), Point(105, 10));
  EXPECT_TRUE(rr.Overlaps(right_overlap));
}

TEST(RoundedRectangleTest, OverlapsRectangle_OverlapInUpperRegion) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Overlaps the upper strip.
  Rectangle upper_overlap(Point(-10, 42), Point(10, 55));
  EXPECT_TRUE(rr.Overlaps(upper_overlap));
}

TEST(RoundedRectangleTest, OverlapsRectangle_OverlapInLowerRegion) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Overlaps the lower strip.
  Rectangle lower_overlap(Point(-10, -55), Point(10, -42));
  EXPECT_TRUE(rr.Overlaps(lower_overlap));
}

TEST(RoundedRectangleTest, OverlapsRectangle_InCornerRegionButOutsideRadius) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // A small rectangle placed in the corner cutout region. The corner centre
  // is at (-90, -40) with radius 10. This rectangle sits just outside the
  // rounded corner arc but inside the bounding box.
  Rectangle corner_miss(Point(-101, -51), Point(-99, -49));
  EXPECT_FALSE(rr.Overlaps(corner_miss));
}

TEST(RoundedRectangleTest, OverlapsRectangle_InCornerRegionButInsideRadius) {
  RoundedRectangle rr = MakeTestRoundedRect();
  Rectangle corner_hit(Point(-101, -51), Point(-91, -41));
  EXPECT_TRUE(rr.Overlaps(corner_hit));
}

TEST(RoundedRectangleTest, OverlapsRectangle_TouchingEdge) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Rectangle exactly abutting the right edge of the bounding box.
  Rectangle touching(Point(100, -10), Point(110, 10));
  // Abutting (sharing an edge) is overlapping for us..
  EXPECT_TRUE(rr.Overlaps(touching));
}

TEST(RoundedRectangleTest, OverlapsRectangle_FullyContained) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Rectangle fully inside the rounded rectangle.
  Rectangle inside(Point(-50, -20), Point(50, 20));
  EXPECT_TRUE(rr.Overlaps(inside));
}

TEST(RoundedRectangleTest, OverlapsRectangle_FullyContaining) {
  RoundedRectangle rr = MakeTestRoundedRect();
  // Rectangle that fully contains the rounded rectangle.
  Rectangle outside(Point(-200, -200), Point(200, 200));
  EXPECT_TRUE(rr.Overlaps(outside));
}

// --- Overlaps(const RoundedRectangle &) tests ---

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_NoOverlap) {
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(200, 200), Point(300, 300), 10);
  EXPECT_FALSE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_ClearOverlap) {
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(50, 50), Point(150, 150), 10);
  EXPECT_TRUE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_SameRectangle) {
  RoundedRectangle a(Point(0, 0), Point(100, 100), 15);
  EXPECT_TRUE(a.Overlaps(a));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_FullyContained) {
  RoundedRectangle outer(Point(0, 0), Point(200, 200), 20);
  RoundedRectangle inner(Point(50, 50), Point(150, 150), 10);
  EXPECT_TRUE(outer.Overlaps(inner));
  EXPECT_TRUE(inner.Overlaps(outer));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_TouchingEdge) {
  // Abutting along the right/left edge in the non-corner region.
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(100, 0), Point(200, 100), 10);
  EXPECT_TRUE(a.Overlaps(b));
  EXPECT_TRUE(b.Overlaps(a));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_OverlapInCentreStrips) {
  // Overlapping only in the centre strip regions, not in corners.
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(90, 20), Point(200, 80), 10);
  EXPECT_TRUE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_CornersNearMiss) {
  // Two rounded rectangles whose bounding boxes overlap only in the corner
  // cutout regions. The corners are far enough apart that the arcs don't touch.
  // a's upper-right corner centre is at (90, 90), b's lower-left corner centre
  // is at (110, 110), both with radius 10. Distance between centres is
  // sqrt(800) ~= 28.3, which is > 20 (sum of radii).
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(100, 100), Point(200, 200), 10);
  EXPECT_FALSE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_CornersOverlap) {
  // Two rounded rectangles whose corners overlap. a's upper-right corner
  // centre is at (90, 90), b's lower-left corner centre is at (95, 95),
  // both with radius 10. Distance between centres is sqrt(50) ~= 7.07,
  // which is < 20 (sum of radii).
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(85, 85), Point(200, 200), 10);
  EXPECT_TRUE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_DifferentRadii) {
  RoundedRectangle a(Point(0, 0), Point(100, 100), 5);
  RoundedRectangle b(Point(50, 50), Point(150, 150), 30);
  EXPECT_TRUE(a.Overlaps(b));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_Symmetric) {
  // Overlap check should be symmetric.
  RoundedRectangle a(Point(0, 0), Point(100, 100), 10);
  RoundedRectangle b(Point(60, 60), Point(180, 180), 15);
  EXPECT_EQ(a.Overlaps(b), b.Overlaps(a));
}

TEST(RoundedRectangleTest, OverlapsRoundedRectangle_ZeroRadius) {
  // With zero radius, these are just rectangles.
  RoundedRectangle a(Point(0, 0), Point(100, 100), 0);
  RoundedRectangle b(Point(50, 50), Point(150, 150), 0);
  EXPECT_TRUE(a.Overlaps(b));

  RoundedRectangle c(Point(200, 200), Point(300, 300), 0);
  EXPECT_FALSE(a.Overlaps(c));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
