#include "utility.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace bfg {
namespace {

TEST(Utility, UpdateMax) {
  std::optional<int> value;
  ASSERT_EQ(std::nullopt, value);
  Utility::UpdateMax(5, &value);
  EXPECT_TRUE(value.has_value());
  EXPECT_EQ(5, *value);
  Utility::UpdateMax(3, &value);
  EXPECT_EQ(5, *value);
  Utility::UpdateMax(8, &value);
  EXPECT_EQ(8, *value);
}

TEST(Utility, UpdateMin) {
  std::optional<int> value;
  ASSERT_EQ(std::nullopt, value);
  Utility::UpdateMin(5, &value);
  EXPECT_TRUE(value.has_value());
  EXPECT_EQ(5, *value);
  Utility::UpdateMin(3, &value);
  EXPECT_EQ(3, *value);
  Utility::UpdateMin(8, &value);
  EXPECT_EQ(3, *value);
}

TEST(Utility, NextMultiple) {
  int multiple = 5;
  int min = 4;
  EXPECT_EQ(5, Utility::NextMultiple(min, multiple));

  min = 5;
  EXPECT_EQ(5, Utility::NextMultiple(min, multiple));

  min = 6;
  EXPECT_EQ(10, Utility::NextMultiple(min, multiple));

  multiple = 0;
  EXPECT_EQ(6, Utility::NextMultiple(min, multiple));
}

TEST(Utility, NextGreaterMultiple) {
  int multiple = 5;
  int min = 4;
  EXPECT_EQ(5, Utility::NextGreaterMultiple(min, multiple));

  min = 5;
  EXPECT_EQ(10, Utility::NextGreaterMultiple(min, multiple));

  min = 6;
  EXPECT_EQ(10, Utility::NextGreaterMultiple(min, multiple));

  multiple = 0;
  EXPECT_EQ(6, Utility::NextGreaterMultiple(min, multiple));
}

TEST(Utility, StripInUnits) {
  std::vector<int64_t> expected = {9, 9, 9, 3};
  EXPECT_THAT(
      Utility::StripInUnits(32, 9, 3), testing::ContainerEq(expected));

  expected = {9, 9, 9, 6};
  EXPECT_THAT(
      Utility::StripInUnits(33, 9, 3), testing::ContainerEq(expected));

  expected = {6, 6, 6, 6, 6};
  EXPECT_THAT(
      Utility::StripInUnits(32, 8, 3), testing::ContainerEq(expected));
}

}   // namespace
}   // namespace bfg
