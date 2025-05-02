#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

#include "sky130_transmission_gate.h"
#include "../physical_properties_database.h"
#include "../dev_pdk_setup.h"

namespace bfg {
namespace atoms {
namespace {

class Sky130TransmissionGateTest : public testing::Test {
 protected:
  void SetUp() override {
    bfg::PhysicalPropertiesDatabase &physical_db = design_db_.physical_db();
    design_db_.physical_db().LoadTechnologyFromFile(
        "test_data/sky130.technology.pb");
    bfg::SetUpSky130(&physical_db);
  }

  DesignDatabase design_db_;
};

TEST_F(Sky130TransmissionGateTest, ViaLocations) {
  //std::vector<Sky130SimpleTransistor::ViaPosition> positions = {
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER,
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE,
  //  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_UPPER,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_MIDDLE,
  //  //Sky130SimpleTransistor::ViaPosition::POLY_LOWER,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE,
  //  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER
  //};
  //for (size_t i = 0; i < positions.size(); ++i) {
  //  layout->MakeVia(
  //      pfet_generator_->DiffConnectionLayer(),
  //      pfet_generator_->ViaLocation(positions[i]));
  //  layout->MakeVia(
  //      nfet_generator_->DiffConnectionLayer(),
  //      nfet_generator_->ViaLocation(positions[i]));
  //}
}

}  // namespace
}  // namespace atoms
}  // namespace bfg
