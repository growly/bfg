#include <gtest/gtest.h>

#include <cmath>

#include "radian.h"

namespace bfg {
namespace geometry {

TEST(RadianTest, kPiIsApproximatelyPi) {
  EXPECT_NEAR(Radian::kPi, M_PI, 1e-15);
}

TEST(RadianTest, IsEffectivelyZero_Zero) {
  EXPECT_TRUE(Radian::IsEffectivelyZero(0.0));
}

TEST(RadianTest, IsEffectivelyZero_NegativeZero) {
  EXPECT_TRUE(Radian::IsEffectivelyZero(-0.0));
}

TEST(RadianTest, IsEffectivelyZero_SmallPositive) {
  EXPECT_FALSE(Radian::IsEffectivelyZero(1e-10));
}

TEST(RadianTest, IsEffectivelyZero_SmallNegative) {
  EXPECT_FALSE(Radian::IsEffectivelyZero(-1e-10));
}

TEST(RadianTest, IsEffectivelyZero_One) {
  EXPECT_FALSE(Radian::IsEffectivelyZero(1.0));
}

TEST(RadianTest, RadiansToDegrees_Zero) {
  EXPECT_NEAR(0.0, Radian::RadiansToDegrees(0.0), 1e-10);
}

TEST(RadianTest, RadiansToDegrees_Pi) {
  EXPECT_NEAR(180.0, Radian::RadiansToDegrees(Radian::kPi), 1e-10);
}

TEST(RadianTest, RadiansToDegrees_TwoPi) {
  EXPECT_NEAR(360.0, Radian::RadiansToDegrees(2.0 * Radian::kPi), 1e-10);
}

TEST(RadianTest, RadiansToDegrees_HalfPi) {
  EXPECT_NEAR(90.0, Radian::RadiansToDegrees(Radian::kPi / 2.0), 1e-10);
}

TEST(RadianTest, RadiansToDegrees_QuarterPi) {
  EXPECT_NEAR(45.0, Radian::RadiansToDegrees(Radian::kPi / 4.0), 1e-10);
}

TEST(RadianTest, RadiansToDegrees_Negative) {
  EXPECT_NEAR(-90.0, Radian::RadiansToDegrees(-Radian::kPi / 2.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_Zero) {
  EXPECT_NEAR(0.0, Radian::DegreesToRadians(0.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_180) {
  EXPECT_NEAR(Radian::kPi, Radian::DegreesToRadians(180.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_360) {
  EXPECT_NEAR(2.0 * Radian::kPi, Radian::DegreesToRadians(360.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_90) {
  EXPECT_NEAR(Radian::kPi / 2.0, Radian::DegreesToRadians(90.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_45) {
  EXPECT_NEAR(Radian::kPi / 4.0, Radian::DegreesToRadians(45.0), 1e-10);
}

TEST(RadianTest, DegreesToRadians_Negative) {
  EXPECT_NEAR(-Radian::kPi / 2.0, Radian::DegreesToRadians(-90.0), 1e-10);
}

TEST(RadianTest, ConversionIsInverse) {
  for (double deg : {0.0, 30.0, 45.0, 60.0, 90.0, 135.0, 180.0, 270.0, 360.0}) {
    EXPECT_NEAR(deg, Radian::RadiansToDegrees(Radian::DegreesToRadians(deg)), 1e-10);
  }
}

}  // namespace geometry
}  // namespace bfg
