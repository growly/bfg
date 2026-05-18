#ifndef ATOMS_SKY130_SPLIT_BUFFER_H_
#define ATOMS_SKY130_SPLIT_BUFFER_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates an inverter and a buffer in parallel from the output:
//
// A ---+--->o------ P = ~A
//      |
//      +--->o-->o-- X = ~~A
//
// The purpose of this cell is to try and produce a signal and its complement
// roughly in sync. This is useful for things like transmission gates.
//
// The circuit is:
//                                        
//                               /               /                /
//                               |               |                |
//                          g   _| s        g   _| s         g   _| s
//                          +-o|_ pfet_1    +-o|_  pfet_2a   +-o|_  pfet_2b
//                          |    | d        |    | d         |    | d
//                       +--+    +-----Xb---+----|-----------+    |
//                       |  |   _| d        |    +-----------|----+--- X
//                       |  +--|_ nfet_1    |    |           |    |
//                       |  g    | s        |   _| d         |   _| d
//                       |       |          +--|_  nfet_2a   +--|_  nfet_2b
//                       |       V          g    | s         g    | s
//                       |                       |                |
//                   A --+                       V                V
//                       |
//                       |         /                /
//                       |         |                |
//                       |    g   _| s         g   _| s
//                       |    +-o|_  pfet_0a   +-o|_  pfet_0b
//                       |    |    | d         |    | d
//                       +----+----|-----------+    |
//                            |    +-----------|----+--- P
//                            |    |           |    |
//                            |   _| d         |   _| d
//                            +--|_  nfet_0a   +--|_  nfet_0b
//                            g    | s         g    | s
//                                 |                |
//                                 V                V
//
// The layout should be straightforward. On the right hand side of the cell we
// put the top branch, with nfet1/2a and optionally nfet2b:
//
//                | VDD |                           |VDD|
// +--------------|     |--------------+------------|   |+
// |              +-----+              |            +---+|
// | pfet_1          | pfet_2a         | pfet_2b         |
// |      +---+      |      +---+      |      +---+      |
// +------|   |------+------|   |------+------|   |------+
//        |   |             |   +-------------+   |
//        |   |             |                     |
//        |   |             |   +-------------+   |
// +------|   |------+------|   |------+------|   |------+
// |      +---+      |      +---+      |      +---+      |
// | nfet_1          | nfet_2a         | nfet_2b         |
// |              +-----+              |            +---+|
// +--------------|     |--------------+------------|   |+
//                |VGND |                           |VGND
//
// On the left we put the bottom branch, which is optionally double-wide.
//
class Sky130SplitBuffer: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm = 1380;
    uint64_t height_nm = 2720;

    uint64_t nfet_0_width_nm = 520;
    uint64_t nfet_1_width_nm = 520;
    uint64_t nfet_2_width_nm = 520;
    uint64_t pfet_0_width_nm = 790;
    uint64_t pfet_1_width_nm = 790;
    uint64_t pfet_2_width_nm = 790;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;

    bool double_nfet0 = true;
    bool double_nfet2 = true;
  };

  Sky130SplitBuffer(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {

    Sky130SimpleTransistor::Parameters nfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
      .width_nm = parameters_.n_width_nm,
      .length_nm = parameters_.n_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
      .poly_overhang_top_nm =
          (parameters_.n_tab_position &&
              geometry::CompassHasNorth(*parameters_.n_tab_position)) ?
              parameters_.min_n_tab_diff_separation_nm : std::nullopt,
      .poly_overhang_bottom_nm = 
          (parameters_.n_tab_position &&
              geometry::CompassHasSouth(*parameters_.n_tab_position)) ?
              parameters_.min_n_tab_diff_separation_nm : std::nullopt,
      .stacking_pitch_nm = parameters_.poly_pitch_nm
    };

    Sky130SimpleTransistor::Parameters pfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
      .width_nm = parameters_.p_width_nm,
      .length_nm = parameters_.p_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
      .poly_overhang_top_nm =
          (parameters_.p_tab_position &&
              geometry::CompassHasNorth(*parameters_.p_tab_position)) ?
              parameters_.min_p_tab_diff_separation_nm : std::nullopt,
      .poly_overhang_bottom_nm = 
          (parameters_.p_tab_position &&
              geometry::CompassHasSouth(*parameters_.p_tab_position)) ?
              parameters_.min_p_tab_diff_separation_nm : std::nullopt,
      .stacking_pitch_nm = parameters_.poly_pitch_nm
    }; 
  }

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  Sky130SimpleTransistor Make

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SPLIT_BUFFER_H_
