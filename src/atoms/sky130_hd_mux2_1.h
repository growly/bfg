#ifndef ATOMS_SKY130_HD_MUX2_1_H_
#define ATOMS_SKY130_HD_MUX2_1_H_

#include <cstdint>

#include "atom.h"
#include "../cell.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates an active 2:1 mux for Skywater 130. Based on the
// sky130_fd_sc_hd__mux2_1 standard cell.
class Sky130HdMux21: public Atom {
 public:
  struct Parameters {
  };

  Sky130HdMux21(const Parameters &parameters, DesignDatabase *design_db)
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

#endif  // ATOMS_SKY130_HD_MUX2_1_H_
