#ifndef ATOMS_LAYOUT_DEMO_H_
#define ATOMS_LAYOUT_DEMO_H_

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

#include "proto/parameters/layout_demo.pb.h"

namespace bfg {

class DesignDatabase;

// NOTE TO READER: This class was generated entirely by Claude. I prompted the
// whole thing. I told it to use examples from the rest of the codebase and it
// seems to have correctly copy-pasted the right stuff. In particular, I told
// it to use Sky130SimpleTransistor, which basically does this with a lot of
// extra fluff. Anyway that means that, too, can use a coding agent to help you
// build VLSI layout!

namespace atoms {

// Minimal layout generator, used as a starting point / example for building
// new atoms.
class LayoutDemo : public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    // Transistor dimensions, in nanometres. Note that in layout the FET "width"
    // runs along the diffusion (drawn vertically here), while the "length" is
    // the width of the poly gate that crosses it.
    uint64_t width_nm = 500;
    uint64_t length_nm = 170;

    void ToProto(proto::parameters::LayoutDemo *pb) const {}
    void FromProto(const proto::parameters::LayoutDemo &pb) {}
  };

  LayoutDemo(const Parameters &parameters, DesignDatabase *design_db)
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

#endif  // ATOMS_LAYOUT_DEMO_H_
