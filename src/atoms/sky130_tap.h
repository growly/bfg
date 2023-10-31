#ifndef ATOMS_SKY130_TAP_H_
#define ATOMS_SKY130_TAP_H_

#include <cstdint>

#include "atom.h"
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
class Sky130Tap: public Atom {
 public:
  Sky130Tap(DesignDatabase *design_db)
      : Atom(design_db) {}

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
