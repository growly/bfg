#ifndef ATOMS_SKY130_TAP_H_
#define ATOMS_SKY130_TAP_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates a P- and N- tap structure that looks like the tap standard cells.
// The top structure connects VDD from metal1 to an N- substrate (i.e. where a
// P-diffusion would go), and the bottom structure connects VSS from metal1 to
// a P+ substrate (i.e. where an N-diffusion would go).
class Sky130Tap: public Atom {
 public:
  struct Parameters {
    uint64_t height_nm = 2720;
    uint64_t width_nm = 460;

    uint64_t li_width_nm = 170;   // This is just the sky130 min width?
    uint64_t metal_1_width_nm = 480;
  };

  Sky130Tap(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db), parameters_(parameters) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_TAP_H_
