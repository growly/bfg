#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "sky130_simple_transistor.h"
#include "../physical_properties_database.h"
#include "../dev_pdk_setup.h"

namespace bfg {
namespace atoms {
namespace {

class Sky130SimpleTransistorTest : public testing::Test {
 protected:
  void SetUp() override {
    bfg::PhysicalPropertiesDatabase &physical_db = design_db_.physical_db();
    design_db_.physical_db().LoadTechnologyFromFile(
        "test_data/sky130.technology.pb");
    bfg::SetUpSky130(&physical_db);
  }

  DesignDatabase design_db_;
};

TEST_F(Sky130SimpleTransistorTest,
       ViaLocation_NMOS_AlignedPolyTop_LeftDiffMiddle) {
  Sky130SimpleTransistor::Parameters params = {
    .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
    .width_nm = 500,   // Also 500 in internal units.
    .length_nm = 150,  // Also 150 in internal units.
    .stacks_left = false,
    .stacks_right = true,
  };
  Sky130SimpleTransistor xtor(params, &design_db_);

  //      285      150    175
  //  <---------><-----><----->
  //             +-----+          ^
  //             |  D  |          |
  // +-----------|     |------+   |
  // |     A     |     |      G   |
  // |           |     |      |   |
  // |     B     |  E  |      H   | 780
  // |           |     |      |   |
  // |     C     |     |      I   |
  // +-----------|     |------+   |
  //             |  F  |          |
  // L           +--x--+          v
  // (-360, 0)      (0, 0)
  // B should be at (-217.50 (rounded), 380)
  // H should be at (250 (610 - 360), 380)

  xtor.AlignTransistorPartTo(
      Sky130SimpleTransistor::Alignment::POLY_BOTTOM_CENTRE,
      geometry::Point(0, 0));
  EXPECT_EQ(
      geometry::Point(std::llround(-217.50f), 380),
      xtor.ViaLocation(Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE));
  EXPECT_EQ(
      geometry::Point(610 - 360, 380),
      xtor.ViaLocation(Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE));
}

TEST_F(Sky130SimpleTransistorTest, LowerLeft) {
  Sky130SimpleTransistor::Parameters params = {
    .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
    .width_nm = 500,   // Also 500 in internal units.
    .length_nm = 150,  // Also 150 in internal units.
    .stacks_left = false,
    .stacks_right = false,
  };
  Sky130SimpleTransistor xtor(params, &design_db_);

  geometry::Point origin;

  EXPECT_EQ(760, xtor.PolyHeight());

  // The un-stacked diffusion wing (extension beyond poly) should be 285 (since
  // it includes space for a via), so the extension from the central y axis
  // should be 285 + 150/2 = 360.

  EXPECT_EQ(geometry::Point(-360, -380), xtor.LowerLeft());
  xtor.AlignTransistorPartTo(
      Sky130SimpleTransistor::Alignment::POLY_TOP_CENTRE, origin);
  EXPECT_EQ(geometry::Point(-360, -760), xtor.LowerLeft());
  xtor.AlignTransistorPartTo(
      Sky130SimpleTransistor::Alignment::POLY_BOTTOM_CENTRE, origin);
  EXPECT_EQ(geometry::Point(-360, 0), xtor.LowerLeft());

  //EXPECT_FALSE(nets.Contains("a"));
  //EXPECT_FALSE(nets.Contains("b"));
  //EXPECT_FALSE(nets.Contains("c"));
  //EXPECT_EQ("", nets.primary());
}

}  // namespace
}  // namespace atoms
}  // namespace bfg
