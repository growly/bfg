#ifndef ATOMS_SKY130_BUF_H_
#define ATOMS_SKY130_BUF_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates a non-inverting buffer (from two inverters) for Sky130.
//
// This is an experimental class. I use it to figure out what kinds of features
// these atoms need.
//
// TODO(growly):
//  - Once transistors exceed a certain width, we have to be able to
//  automatically split them into two parallel transistors of half that width
//  each. When to do this depends on the maximum diffusion height.
//  - Is an "Atom" any more than a "Generator"?
class Sky130Buf: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm = 1380;
    uint64_t height_nm = 2720;

    // TODO(aryap): These are parameters which the layout should implement
    // dynamically, but currently all the shapes are static from the initial
    // import.
    uint64_t nfet_0_width_nm = 520;
    uint64_t nfet_1_width_nm = 520;
    uint64_t pfet_0_width_nm = 790;
    uint64_t pfet_1_width_nm = 790;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;

    bool label_pins = true;

    // Add the mcon vias on the VPWR/VGND met1 rails. This can be helpful if
    // adjacent cells do not conform to being a multiple of a std. cell site
    // width and so the vias do not line up with those set by cells in adjacent
    // rows.
    bool draw_overflowing_vias_and_pins = true;
  };

  Sky130Buf(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_BUF_H_
