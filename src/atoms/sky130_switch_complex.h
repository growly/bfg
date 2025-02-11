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
    uint64_t nw_pfet_width_nm = 640;  // North-West
    uint64_t ne_pfet_width_nm = 640;  // North-East
    uint64_t es_pfet_width_nm = 640;  // East-West
    uint64_t sw_pfet_width_nm = 640;  // South-West
    uint64_t ew_pfet_width_nm = 640;  // East-West
    uint64_t ns_pfet_width_nm = 640;  // North-South

    uint64_t nw_nfet_width_nm = 480;
    uint64_t ne_nfet_width_nm = 480;
    uint64_t es_nfet_width_nm = 480;
    uint64_t sw_nfet_width_nm = 480;
    uint64_t ew_nfet_width_nm = 480;
    uint64_t ns_nfet_width_nm = 480;

    uint64_t nw_pfet_length_nm = 150;
    uint64_t ne_pfet_length_nm = 150;
    uint64_t es_pfet_length_nm = 150;
    uint64_t sw_pfet_length_nm = 150;
    uint64_t ew_pfet_length_nm = 150;
    uint64_t ns_pfet_length_nm = 150;

    uint64_t nw_nfet_length_nm = 150;
    uint64_t ne_nfet_length_nm = 150;
    uint64_t es_nfet_length_nm = 150;
    uint64_t sw_nfet_length_nm = 150;
    uint64_t ew_nfet_length_nm = 150;
    uint64_t ns_nfet_length_nm = 150;
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
