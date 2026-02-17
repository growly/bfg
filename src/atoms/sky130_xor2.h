#ifndef ATOMS_SKY130_XOR2_H_
#define ATOMS_SKY130_XOR2_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

#include "proto/parameters/sky130_xor2.pb.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates a 2-input XOR gate for Sky130.
//
// Based on the implementation in the HD library.
//
// The XOR function is: X = A⊕B = A·B + A·B
class Sky130Xor2: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm = 3220;
    uint64_t height_nm = 2720;

    uint64_t nfet_0_width_nm = 650;
    uint64_t nfet_1_width_nm = 650;
    uint64_t nfet_2_width_nm = 650;
    uint64_t nfet_3_width_nm = 650;
    uint64_t nfet_4_width_nm = 650;

    uint64_t pfet_0_width_nm = 1000;
    uint64_t pfet_1_width_nm = 1000;
    uint64_t pfet_2_width_nm = 1000;
    uint64_t pfet_3_width_nm = 1000;
    uint64_t pfet_4_width_nm = 1000;

    // TODO(aryap): Gate lengths are not implemented. The parameters are left
    // here as a reminder of the eternal Sisyphean burden of life.
    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t nfet_3_length_nm = 150;
    uint64_t nfet_4_length_nm = 150;

    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;
    uint64_t pfet_3_length_nm = 150;
    uint64_t pfet_4_length_nm = 150;

    bool label_pins = true;

    // Add the mcon vias on the VPWR/VGND met1 rails. This can be helpful if
    // adjacent cells do not conform to being a multiple of a std. cell site
    // width and so the vias do not line up with those set by cells in adjacent
    // rows.
    bool draw_overflowing_vias_and_pins = true;

    // FIXME(aryap): Implement.
    void ToProto(proto::parameters::Sky130Xor2 *pb) const {}
    void FromProto(const proto::parameters::Sky130Xor2 &pb) {}
  };

  Sky130Xor2(const Parameters &parameters, DesignDatabase *design_db)
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

#endif  // ATOMS_SKY130_XOR2_H_
