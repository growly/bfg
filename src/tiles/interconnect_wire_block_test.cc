#include "interconnect_wire_block.h"

#include <gtest/gtest.h>

#include "../atoms/generator_test_fixture.h"
#include "../geometry/rectangle.h"

namespace bfg {
namespace tiles {
namespace {

using InterconnectWireBlockTest = GeneratorTest;

TEST_F(InterconnectWireBlockTest, DefaultChannelsAreEmpty) {
  Cell *cell = Generate<InterconnectWireBlock>({}, "iwb_empty");
  ASSERT_NE(nullptr, cell);
  ASSERT_NE(nullptr, cell->layout());
  EXPECT_EQ(0, cell->layout()->GetBoundingBox().Width());
}

TEST_F(InterconnectWireBlockTest, OneChannel) {
  InterconnectWireBlock::Parameters parameters;
  parameters.channels = {
      InterconnectWireBlock::Parameters::Channel {
          .name = "2X",
          .bundles = {
              InterconnectWireBlock::Parameters::Bundle {
                  .num_wires = 4,
                  .tap = true
              }
          }
      }
  };
  EXPECT_EQ(4, parameters.NumWires());
  EXPECT_EQ(4, parameters.NumTaps());

  Cell *cell = Generate<InterconnectWireBlock>(parameters, "iwb");
  ExpectLayout(cell);
  if (HasFatalFailure()) {
    return;
  }
  // The wires run the configured length.
  geometry::Rectangle bounds = cell->layout()->GetBoundingBox();
  EXPECT_GE(std::max(bounds.Width(), bounds.Height()), parameters.length);
}

}  // namespace
}  // namespace tiles
}  // namespace bfg
