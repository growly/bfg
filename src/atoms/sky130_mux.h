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

  struct Mux2Parameters {
    std::string diff_layer_name = "ndiff.drawing";
    std::string diff_contact_layer_name = "ndiff.drawing";

    // To avoid C++'s automatic arithmetic type conversions, make these all the
    // same int64_t type:
    int64_t fet_0_width = 640;
    int64_t fet_1_width = 640;
    int64_t fet_2_width = 640;
    int64_t fet_3_width = 640;
    int64_t fet_4_width = 640;
    int64_t fet_5_width = 640;

    int64_t fet_0_length = 150;
    int64_t fet_1_length = 150;
    int64_t fet_2_length = 150;
    int64_t fet_3_length = 150;
    int64_t fet_4_length = 150;
    int64_t fet_5_length = 150;

    int64_t fet_4_5_offset_y = 0;

    bool add_input_wires = true;

    std::optional<int64_t> col_0_poly_overhang_top;
    std::optional<int64_t> col_0_poly_overhang_bottom;
    std::optional<int64_t> col_1_poly_overhang_top;
    std::optional<int64_t> col_1_poly_overhang_bottom;
    std::optional<int64_t> col_2_poly_overhang_top;
    std::optional<int64_t> col_2_poly_overhang_bottom;
    std::optional<int64_t> col_3_poly_overhang_top;
    std::optional<int64_t> col_3_poly_overhang_bottom;

    // Pointers emitted to created input objects.
    std::optional<bfg::geometry::Polygon**> input_0;
    std::optional<bfg::geometry::Polygon**> input_1;
    std::optional<bfg::geometry::Polygon**> input_2;
    std::optional<bfg::geometry::Polygon**> input_3;
  };

  Sky130Mux(const Parameters &parameters,
            DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters),
        internal_units_per_nm_(1.0) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 protected:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  bfg::Layout *GenerateMux2Layout(const Mux2Parameters &params);
  bfg::Circuit *GenerateMux2Circuit();

  Parameters parameters_;

  double internal_units_per_nm_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_MUX_H_
