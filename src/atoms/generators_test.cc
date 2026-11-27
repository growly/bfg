// Smoke tests: every atom generates, with default parameters, a cell with a
// non-empty layout and a circuit whose key ports have pins in the layout.

#include <gtest/gtest.h>

#include <set>
#include <string>


#include "generator_test_fixture.h"
#include "layout_demo.h"
#include "sky130_buf.h"
#include "sky130_carry1.h"
#include "sky130_decap.h"
#include "sky130_dfxtp.h"
#include "sky130_hd_mux2_1.h"
#include "sky130_interconnect_mux1.h"
#include "sky130_interconnect_mux2.h"
#include "sky130_mux.h"
#include "sky130_passive_mux3.h"
#include "sky130_split_buffer.h"
#include "sky130_switch_complex.h"
#include "sky130_tap.h"
#include "sky130_transmission_gate.h"
#include "sky130_transmission_gate_stack.h"
#include "sky130_xor2.h"

namespace bfg {
namespace atoms {
namespace {

using AtomTest = GeneratorTest;

TEST_F(AtomTest, Sky130Buf) {
  Cell *cell = Generate<Sky130Buf>({}, "buf");
  ExpectWellFormed(cell, {"A", "X"});
}

TEST_F(AtomTest, Sky130Carry1) {
  ExpectWellFormed(Generate<Sky130Carry1>({}, "carry1"), {});
}

TEST_F(AtomTest, Sky130Decap) {
  ExpectLayout(Generate<Sky130Decap>({}, "decap"));
}

TEST_F(AtomTest, Sky130Dfxtp) {
  ExpectWellFormed(Generate<Sky130Dfxtp>({}, "dfxtp"), {"D", "Q", "CLK"});
}

TEST_F(AtomTest, Sky130HdMux21) {
  ExpectWellFormed(Generate<Sky130HdMux21>({}, "mux2_1"),
                   {"A0", "A1", "S", "X"});
}

TEST_F(AtomTest, Sky130InterconnectMux1) {
  ExpectWellFormed(Generate<Sky130InterconnectMux1>({}, "imux1"), {});
}

// The configurations ReducedTile uses.
TEST_F(AtomTest, Sky130InterconnectMux2) {
  Sky130InterconnectMux1::Parameters parameters;
  parameters.num_inputs = 7;
  parameters.num_outputs = 2;
  ExpectWellFormed(
      Generate<Sky130InterconnectMux2>(parameters, "imux2"), {});
}

TEST_F(AtomTest, Sky130InterconnectMux2_InsideOut) {
  Sky130InterconnectMux1::Parameters parameters;
  parameters.num_inputs = 5;
  parameters.num_outputs = 2;
  parameters.inside_out = true;
  ExpectWellFormed(
      Generate<Sky130InterconnectMux2>(parameters, "imux2_inside_out"), {});
}

TEST_F(AtomTest, Sky130Mux) {
  ExpectWellFormed(Generate<Sky130Mux>({}, "mux"), {});
}

// Circuit generation is not implemented for this generator yet.
TEST_F(AtomTest, Sky130PassiveMux3) {
  ExpectLayout(Generate<Sky130PassiveMux3>({}, "pmux3"));
}

TEST_F(AtomTest, Sky130SplitBuffer) {
  ExpectWellFormed(Generate<Sky130SplitBuffer>({}, "split_buf"), {});
}

// Circuit generation is not implemented for this generator yet.
TEST_F(AtomTest, Sky130SwitchComplex) {
  ExpectLayout(Generate<Sky130SwitchComplex>({}, "switch_complex"));
}

TEST_F(AtomTest, Sky130Tap) {
  ExpectLayout(Generate<Sky130Tap>({}, "tap"));
}

TEST_F(AtomTest, Sky130TransmissionGate) {
  ExpectWellFormed(Generate<Sky130TransmissionGate>({}, "tgate"), {});
}

TEST_F(AtomTest, Sky130TransmissionGateStack) {
  ExpectWellFormed(
      Generate<Sky130TransmissionGateStack>({}, "tgate_stack"), {});
}

TEST_F(AtomTest, Sky130Xor2) {
  ExpectWellFormed(Generate<Sky130Xor2>({}, "xor2"), {});
}

TEST_F(AtomTest, LayoutDemo) {
  ExpectLayout(Generate<LayoutDemo>({}, "layout_demo"));
}

}  // namespace
}  // namespace atoms
}  // namespace bfg
