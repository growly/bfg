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
    std::string fet_model_length_parameter = "l";
    std::string fet_model_width_parameter = "w";

    uint64_t pfet_0_width_nm = 420;
    uint64_t pfet_1_width_nm = 420;
    uint64_t pfet_2_width_nm = 420;
    uint64_t pfet_3_width_nm = 420;
    uint64_t pfet_4_width_nm = 420;
    uint64_t pfet_5_width_nm = 1000;

    uint64_t nfet_0_width_nm = 420;
    uint64_t nfet_1_width_nm = 420;
    uint64_t nfet_2_width_nm = 420;
    uint64_t nfet_3_width_nm = 420;
    uint64_t nfet_4_width_nm = 420;
    uint64_t nfet_5_width_nm = 650;

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
