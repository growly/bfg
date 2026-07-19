#ifndef ATOMS_LAYOUT_DEMO_H_
#define ATOMS_LAYOUT_DEMO_H_

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

#include "proto/parameters/layout_demo.pb.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Minimal layout generator, used as a starting point / example for building
// new atoms.
class LayoutDemo : public Atom {
 public:
  struct Parameters : public Sky130Parameters {
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
