#ifndef ATOMS_SKY130_MUX_H_
#define ATOMS_SKY130_MUX_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Sky130Mux: public Atom {
 public:
  struct Parameters {
    uint32_t n = 2;

    uint64_t pfet_0_width_nm = 420;
    uint64_t pfet_1_width_nm = 420;
    uint64_t pfet_2_width_nm = 420;
    uint64_t pfet_3_width_nm = 420;
    uint64_t pfet_4_width_nm = 750;
    uint64_t pfet_5_width_nm = 420;

    uint64_t nfet_0_width_nm = 420;
    uint64_t nfet_1_width_nm = 355;
    uint64_t nfet_2_width_nm = 420;
    uint64_t nfet_3_width_nm = 355;
    uint64_t nfet_4_width_nm = 640;
    uint64_t nfet_5_width_nm = 360;

    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;
    uint64_t pfet_3_length_nm = 150;
    uint64_t pfet_4_length_nm = 150;
    uint64_t pfet_5_length_nm = 150;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t nfet_3_length_nm = 150;
    uint64_t nfet_4_length_nm = 150;
    uint64_t nfet_5_length_nm = 150;
  };

  Sky130Mux(const Parameters &parameters,
            DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters),
        internal_units_per_nm_(1.0) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  bfg::Layout *GenerateMux2Layout();
  bfg::Circuit *GenerateMux2Circuit();

  Parameters parameters_;

  double internal_units_per_nm_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_MUX_H_
