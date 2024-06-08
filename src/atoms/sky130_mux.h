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

    uint64_t pfet_0_width_nm = 640;
    uint64_t pfet_1_width_nm = 640;
    uint64_t pfet_2_width_nm = 640;
    uint64_t pfet_3_width_nm = 640;
    uint64_t pfet_4_width_nm = 640;
    uint64_t pfet_5_width_nm = 640;

    uint64_t nfet_0_width_nm = 480;
    uint64_t nfet_1_width_nm = 480;
    uint64_t nfet_2_width_nm = 480;
    uint64_t nfet_3_width_nm = 480;
    uint64_t nfet_4_width_nm = 480;
    uint64_t nfet_5_width_nm = 480;

    uint64_t pfet_0_length_nm = 170;
    uint64_t pfet_1_length_nm = 170;
    uint64_t pfet_2_length_nm = 170;
    uint64_t pfet_3_length_nm = 170;
    uint64_t pfet_4_length_nm = 170;
    uint64_t pfet_5_length_nm = 170;

    uint64_t nfet_0_length_nm = 170;
    uint64_t nfet_1_length_nm = 170;
    uint64_t nfet_2_length_nm = 170;
    uint64_t nfet_3_length_nm = 170;
    uint64_t nfet_4_length_nm = 170;
    uint64_t nfet_5_length_nm = 170;

    // Output mux sizing.
    uint64_t pfet_6_width_nm = 640;
    uint64_t pfet_7_width_nm = 640;

    uint64_t nfet_6_width_nm = 480;
    uint64_t nfet_7_width_nm = 480;

    uint64_t pfet_6_length_nm = 170;
    uint64_t pfet_7_length_nm = 170;

    uint64_t nfet_6_length_nm = 170;
    uint64_t nfet_7_length_nm = 170;

    // TODO(aryap): Maybe these are queried from the PDK database?
    std::string fet_model_length_parameter = "l";
    std::string fet_model_width_parameter = "w";
  };

  struct Mux2CircuitParameters {
    // TODO(aryap): Delete these?
    enum FetType { N, P };
    FetType fet_type = FetType::N;

    const bfg::Circuit *fet_model = nullptr;

    // TODO(aryap): Maybe these are queried from the PDK database?
    std::string fet_model_length_parameter = "l";
    std::string fet_model_width_parameter = "w";

    uint64_t fet_0_width_nm = 640;
    uint64_t fet_1_width_nm = 640;
    uint64_t fet_2_width_nm = 640;
    uint64_t fet_3_width_nm = 640;
    uint64_t fet_4_width_nm = 640;
    uint64_t fet_5_width_nm = 640;

    uint64_t fet_0_length_nm = 150;
    uint64_t fet_1_length_nm = 150;
    uint64_t fet_2_length_nm = 150;
    uint64_t fet_3_length_nm = 150;
    uint64_t fet_4_length_nm = 150;
    uint64_t fet_5_length_nm = 150;

    std::optional<circuit::Wire> vb_wire = std::nullopt;
    std::optional<circuit::Wire> x0_wire = std::nullopt;
    std::optional<circuit::Wire> x1_wire = std::nullopt;
    std::optional<circuit::Wire> x2_wire = std::nullopt;
    std::optional<circuit::Wire> x3_wire = std::nullopt;
    std::optional<circuit::Wire> s0_wire = std::nullopt;
    std::optional<circuit::Wire> s0_b_wire = std::nullopt;
    std::optional<circuit::Wire> s1_wire = std::nullopt;
    std::optional<circuit::Wire> s1_b_wire = std::nullopt;
    std::optional<circuit::Wire> y_wire = std::nullopt;
  };

  struct Mux2LayoutParameters {
    std::string diff_layer_name = "ndiff.drawing";
    std::string diff_contact_layer_name = "ndiff.drawing";

    // To avoid C++'s automatic arithmetic type conversions, make these all the
    // same int64_t type (also note that these are all in internal units so
    // don't inherently mean anything).
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

    // Additional padding for the inputs.
    int64_t input_x_padding = 0;
    int64_t input_y_padding = 0;
  };

  Sky130Mux(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters),
        internal_units_per_nm_(1.0) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 protected:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  bfg::Layout *GenerateMux2Layout(const Mux2LayoutParameters &parameters);
  bfg::Circuit *GenerateMux2Circuit(const Mux2CircuitParameters &parameters);

  Parameters parameters_;

  double internal_units_per_nm_;

 private:
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_MUX_H_
