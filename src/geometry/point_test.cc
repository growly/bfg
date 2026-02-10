#include <gtest/gtest.h>

#include "point.h"
#include "radian.h"

namespace bfg {
namespace geometry {
namespace {

TEST(PointTest, Scale) {
  Point p = {10, 11};

  Point a = Point(p);
  a.Scale(0.5);
  // Note rounding up instead of truncation to integer:
  EXPECT_EQ(Point({5, 6}), a);

  Point b = Point(p);
  b.Scale(2.0);
  EXPECT_EQ(Point({20, 22}), b);

  Point c = Point(p);
  c.Scale(0.3333333);
  EXPECT_EQ(Point({3, 4}), c);
}

TEST(PointTest, AddComponents) {
  Point test;
  ASSERT_EQ(0, test.x());
  ASSERT_EQ(0, test.y());

  test.AddComponents(3.5, 0.0);
  EXPECT_EQ(4, test.x());
  EXPECT_EQ(0, test.y());

  test.AddComponents(1.2, 0.0);
  EXPECT_EQ(5, test.x());
  EXPECT_EQ(0, test.y());

  test.AddComponents(3, Radian::kPi / 2);
  EXPECT_EQ(5, test.x());
  EXPECT_EQ(3, test.y());
}

TEST(PointTest, Component) {
  Point test(3, 4);
  EXPECT_EQ(3, test.Component(0.0));
  EXPECT_EQ(4, test.Component(Radian::kPi / 2));
}

// TODO(aryap): Test Point::SortAscendingX, Point::SortDescendingY.

TEST(PointTest, MirrorY_AtOrigin) {
  Point p(0, 0);
  p.MirrorY();
  EXPECT_EQ(Point(0, 0), p);
}

TEST(PointTest, MirrorY_PositiveX) {
  Point p(10, 5);
  p.MirrorY();
  // MirrorY negates x coordinate
  EXPECT_EQ(Point(-10, 5), p);
}

TEST(PointTest, MirrorY_NegativeX) {
  Point p(-7, 3);
  p.MirrorY();
  EXPECT_EQ(Point(7, 3), p);
}

TEST(PointTest, MirrorY_TwiceReturnsToOriginal) {
  Point original(15, 25);
  Point p = original;
  p.MirrorY();
  p.MirrorY();
  EXPECT_EQ(original, p);
}

TEST(PointTest, MirrorX_AtOrigin) {
  Point p(0, 0);
  p.MirrorX();
  EXPECT_EQ(Point(0, 0), p);
}

TEST(PointTest, MirrorX_PositiveY) {
  Point p(5, 10);
  p.MirrorX();
  // MirrorX negates y coordinate
  EXPECT_EQ(Point(5, -10), p);
}

TEST(PointTest, MirrorX_NegativeY) {
  Point p(3, -7);
  p.MirrorX();
  EXPECT_EQ(Point(3, 7), p);
}

TEST(PointTest, MirrorX_TwiceReturnsToOriginal) {
  Point original(15, 25);
  Point p = original;
  p.MirrorX();
  p.MirrorX();
  EXPECT_EQ(original, p);
}

TEST(PointTest, Translate_ZeroOffset) {
  Point p(5, 10);
  p.Translate(Point(0, 0));
  EXPECT_EQ(Point(5, 10), p);
}

TEST(PointTest, Translate_PositiveOffset) {
  Point p(5, 10);
  p.Translate(Point(100, 200));
  EXPECT_EQ(Point(105, 210), p);
}

TEST(PointTest, Translate_NegativeOffset) {
  Point p(50, 60);
  p.Translate(Point(-30, -40));
  EXPECT_EQ(Point(20, 20), p);
}

TEST(PointTest, Translate_ToNegativeCoordinates) {
  Point p(10, 20);
  p.Translate(Point(-50, -60));
  EXPECT_EQ(Point(-40, -40), p);
}

TEST(PointTest, FlipHorizontal_IsNoOp) {
  Point original(5, 10);
  Point p = original;
  p.FlipHorizontal();
  EXPECT_EQ(original, p);
}

TEST(PointTest, FlipVertical_IsNoOp) {
  Point original(5, 10);
  Point p = original;
  p.FlipVertical();
  EXPECT_EQ(original, p);
}

TEST(PointTest, Rotate_0Degrees) {
  Point p(10, 0);
  p.Rotate(0);
  EXPECT_EQ(Point(10, 0), p);
}

TEST(PointTest, Rotate_90Degrees) {
  // (10, 0) rotated 90 degrees CCW around origin -> (0, 10)
  Point p(10, 0);
  p.Rotate(90);
  EXPECT_EQ(Point(0, 10), p);
}

TEST(PointTest, Rotate_180Degrees) {
  // (10, 5) rotated 180 degrees around origin -> (-10, -5)
  Point p(10, 5);
  p.Rotate(180);
  EXPECT_EQ(Point(-10, -5), p);
}

TEST(PointTest, Rotate_270Degrees) {
  // (10, 0) rotated 270 degrees CCW (= 90 CW) around origin -> (0, -10)
  Point p(10, 0);
  p.Rotate(270);
  EXPECT_EQ(Point(0, -10), p);
}

TEST(PointTest, Rotate_360DegreesReturnsToOriginal) {
  Point original(7, 13);
  Point p = original;
  p.Rotate(360);
  EXPECT_EQ(original, p);
}

TEST(PointTest, Rotate_NegativeDegrees) {
  // -90 degrees should be equivalent to 270 degrees CCW
  Point p1(10, 0);
  Point p2(10, 0);
  p1.Rotate(-90);
  p2.Rotate(270);
  EXPECT_EQ(p1, p2);
}

TEST(PointTest, Rotate_FourRotationsOf90ReturnsToOriginal) {
  Point original(5, 7);
  Point p = original;
  p.Rotate(90);
  p.Rotate(90);
  p.Rotate(90);
  p.Rotate(90);
  EXPECT_EQ(original, p);
}

TEST(PointTest, Rotate_90Degrees_GeneralPoint) {
  // (3, 4) rotated 90 degrees CCW around origin -> (-4, 3)
  Point p(3, 4);
  p.Rotate(90);
  EXPECT_EQ(Point(-4, 3), p);
}

TEST(PointTest, Rotate_AtOrigin) {
  // Origin should stay at origin regardless of rotation
  Point p(0, 0);
  p.Rotate(90);
  EXPECT_EQ(Point(0, 0), p);
  p.Rotate(180);
  EXPECT_EQ(Point(0, 0), p);
}

TEST(PointTest, CombinedOperations_MirrorYThenTranslate) {
  Point p(5, 10);
  p.MirrorY();
  p.Translate(Point(10, 0));
  // After MirrorY: (-5, 10)
  // After Translate: (5, 10)
  EXPECT_EQ(Point(5, 10), p);
}

TEST(PointTest, CombinedOperations_MirrorXThenMirrorY) {
  // MirrorX then MirrorY is equivalent to 180 degree rotation
  Point p1(5, 10);
  Point p2(5, 10);
  p1.MirrorX();
  p1.MirrorY();
  p2.Rotate(180);
  EXPECT_EQ(p1, p2);
}

TEST(PointTest, CombinedOperations_TranslateThenRotate) {
  Point p(10, 0);
  p.Translate(Point(5, 0));  // Now at (15, 0)
  p.Rotate(90);              // Now at (0, 15)
  EXPECT_EQ(Point(0, 15), p);
}

// --- L1DistanceTo ---

TEST(PointTest, L1DistanceTo_SamePoint) {
  EXPECT_EQ(Point(5, 10).L1DistanceTo(Point(5, 10)), 0);
}

TEST(PointTest, L1DistanceTo_HorizontalOnly) {
  EXPECT_EQ(Point(0, 0).L1DistanceTo(Point(7, 0)), 7);
}

TEST(PointTest, L1DistanceTo_VerticalOnly) {
  EXPECT_EQ(Point(0, 0).L1DistanceTo(Point(0, 13)), 13);
}

TEST(PointTest, L1DistanceTo_Diagonal) {
  // Manhattan distance from (0,0) to (3,4) = 3 + 4 = 7.
  EXPECT_EQ(Point(0, 0).L1DistanceTo(Point(3, 4)), 7);
}

TEST(PointTest, L1DistanceTo_IsSymmetric) {
  Point a(10, 20);
  Point b(35, 47);
  EXPECT_EQ(a.L1DistanceTo(b), b.L1DistanceTo(a));
}

TEST(PointTest, L1DistanceTo_NegativeCoordinates) {
  // |(-3) - 4| + |(-5) - 6| = 7 + 11 = 18.
  EXPECT_EQ(Point(-3, -5).L1DistanceTo(Point(4, 6)), 18);
}

TEST(PointTest, L1DistanceTo_Origin) {
  EXPECT_EQ(Point(0, 0).L1DistanceTo(Point(0, 0)), 0);
}

// --- L2SquaredDistanceTo ---

TEST(PointTest, L2SquaredDistanceTo_SamePoint) {
  EXPECT_EQ(Point(5, 10).L2SquaredDistanceTo(Point(5, 10)), 0);
}

TEST(PointTest, L2SquaredDistanceTo_HorizontalOnly) {
  // 7^2 + 0^2 = 49.
  EXPECT_EQ(Point(0, 0).L2SquaredDistanceTo(Point(7, 0)), 49);
}

TEST(PointTest, L2SquaredDistanceTo_VerticalOnly) {
  // 0^2 + 13^2 = 169.
  EXPECT_EQ(Point(0, 0).L2SquaredDistanceTo(Point(0, 13)), 169);
}

TEST(PointTest, L2SquaredDistanceTo_345Triangle) {
  // 3^2 + 4^2 = 9 + 16 = 25.
  EXPECT_EQ(Point(0, 0).L2SquaredDistanceTo(Point(3, 4)), 25);
}

TEST(PointTest, L2SquaredDistanceTo_IsSymmetric) {
  Point a(10, 20);
  Point b(35, 47);
  EXPECT_EQ(a.L2SquaredDistanceTo(b), b.L2SquaredDistanceTo(a));
}

TEST(PointTest, L2SquaredDistanceTo_NegativeCoordinates) {
  // dx=7, dy=11. 49 + 121 = 170.
  EXPECT_EQ(Point(-3, -5).L2SquaredDistanceTo(Point(4, 6)), 170);
}

TEST(PointTest, L2SquaredDistanceTo_LargeValues) {
  // dx=100000, dy=100000. 10^10 + 10^10 = 2*10^10.
  EXPECT_EQ(Point(0, 0).L2SquaredDistanceTo(Point(100000, 100000)),
            20000000000LL);
}

// --- L2DistanceTo ---

TEST(PointTest, L2DistanceTo_SamePoint) {
  EXPECT_DOUBLE_EQ(Point(5, 10).L2DistanceTo(Point(5, 10)), 0.0);
}

TEST(PointTest, L2DistanceTo_HorizontalOnly) {
  EXPECT_DOUBLE_EQ(Point(0, 0).L2DistanceTo(Point(7, 0)), 7.0);
}

TEST(PointTest, L2DistanceTo_VerticalOnly) {
  EXPECT_DOUBLE_EQ(Point(0, 0).L2DistanceTo(Point(0, 13)), 13.0);
}

TEST(PointTest, L2DistanceTo_345Triangle) {
  // sqrt(9 + 16) = 5.0 exactly.
  EXPECT_DOUBLE_EQ(Point(0, 0).L2DistanceTo(Point(3, 4)), 5.0);
}

TEST(PointTest, L2DistanceTo_IsSymmetric) {
  Point a(10, 20);
  Point b(35, 47);
  EXPECT_DOUBLE_EQ(a.L2DistanceTo(b), b.L2DistanceTo(a));
}

TEST(PointTest, L2DistanceTo_NegativeCoordinates) {
  // sqrt(49 + 121) = sqrt(170).
  EXPECT_DOUBLE_EQ(Point(-3, -5).L2DistanceTo(Point(4, 6)),
                   std::sqrt(170.0));
}

TEST(PointTest, L2DistanceTo_ConsistentWithL2Squared) {
  Point a(17, 43);
  Point b(-29, 81);
  double l2 = a.L2DistanceTo(b);
  int64_t l2_sq = a.L2SquaredDistanceTo(b);
  EXPECT_DOUBLE_EQ(l2 * l2, static_cast<double>(l2_sq));
}

}  // namespace
}  // namespace geometry
}  // namespace bfg
