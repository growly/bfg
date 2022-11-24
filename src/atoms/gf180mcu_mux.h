#ifndef ATOMS_GF180MCU_MUX_H_
#define ATOMS_GF180MCU_MUX_H_

#include <cstdint>

#include "atom.h"
#include "sky130_mux.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Gf180McuMux: public Sky130Mux {
 public:
  Gf180McuMux(const Sky130Mux::Parameters &parameters,
            DesignDatabase *design_db)
      : Sky130Mux(parameters, design_db) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_MUX_H_
