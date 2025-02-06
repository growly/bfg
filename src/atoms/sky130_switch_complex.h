#ifndef ATOMS_SKY130_SWITCH_COMPLEX_H_
#define ATOMS_SKY130_SWITCH_COMPLEX_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Sky130SwitchComplex: public Atom {
 public:
  struct Parameters {
  };

  Sky130SwitchComplex(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}

  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SWITCH_COMPLEX_H_
