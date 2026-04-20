#ifndef ATOMS_SKY130_DOUBLE_BUF_H_
#define ATOMS_SKY130_DOUBLE_BUF_H_

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
// TODO(aryap): Explicitly uses two parallel output FETs in each stage. This is
// not the most elegant way to scale a buffer, but it does capture a distinct
// layout method.  A meta-buffer should be used to hide the exact layout
// strategy from a user.
class Sky130DoubleBuf: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm = 1380;
    uint64_t height_nm = 2720;

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

  Sky130DoubleBuf(const Parameters &parameters, DesignDatabase *design_db)
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

#endif  // ATOMS_SKY130_DOUBLE_BUF_H_
