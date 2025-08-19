#ifndef ATOMS_SKY130_DFXTP_H_
#define ATOMS_SKY130_DFXTP_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Sky130Dfxtp: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm;
    uint64_t height_nm;

    uint64_t pfet_0_width_nm = 420;
    uint64_t pfet_1_width_nm = 420;
    uint64_t pfet_2_width_nm = 420;
    uint64_t pfet_3_width_nm = 420;
    uint64_t pfet_4_width_nm = 750;
    uint64_t pfet_5_width_nm = 420;
    uint64_t pfet_6_width_nm = 420;
    uint64_t pfet_7_width_nm = 420;
    uint64_t pfet_8_width_nm = 1000;
    uint64_t pfet_9_width_nm = 1000;

    // NOTE(aryap): DO NOT SET WIDTH TO LESS THAN 360. IT BREAKS SPICE MODEL
    // CARD.
    //
    // REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

    uint64_t nfet_0_width_nm = 420;
    uint64_t nfet_1_width_nm = 360;
    uint64_t nfet_2_width_nm = 420;
    uint64_t nfet_3_width_nm = 360;
    uint64_t nfet_4_width_nm = 640;
    uint64_t nfet_5_width_nm = 360;
    uint64_t nfet_6_width_nm = 420;
    uint64_t nfet_7_width_nm = 360;
    uint64_t nfet_8_width_nm = 650;
    uint64_t nfet_9_width_nm = 650;

    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;
    uint64_t pfet_3_length_nm = 150;
    uint64_t pfet_4_length_nm = 150;
    uint64_t pfet_5_length_nm = 150;
    uint64_t pfet_6_length_nm = 150;
    uint64_t pfet_7_length_nm = 150;
    uint64_t pfet_8_length_nm = 150;
    uint64_t pfet_9_length_nm = 150;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t nfet_3_length_nm = 150;
    uint64_t nfet_4_length_nm = 150;
    uint64_t nfet_5_length_nm = 150;
    uint64_t nfet_6_length_nm = 150;
    uint64_t nfet_7_length_nm = 150;
    uint64_t nfet_8_length_nm = 150;
    uint64_t nfet_9_length_nm = 150;
  };

  Sky130Dfxtp(const Parameters &parameters,
              DesignDatabase *design_db)
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

#endif  // ATOMS_SKY130_DFXTP_H_
