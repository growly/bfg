#ifndef ATOMS_SKY130_TRANSMISSION_GATE_STACK_H_
#define ATOMS_SKY130_TRANSMISSION_GATE_STACK_H_

#include <cstdint>
#include <memory>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "../row_guide.h"
#include "../geometry/compass.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../physical_properties_database.h"
#include "proto/parameters/sky130_transmission_gate_stack.pb.h"
#include "sky130_simple_transistor.h"
#include "sky130_transmission_gate.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Sky130TransmissionGateStack : public Atom {
 public:
  struct Parameters {
    // Minimal configuration is the list of port names for each diff and poly.

    // Sequence of ports, [diff, poly, source, poly, diff, ...] etc. Expect an
    // odd number of values.
    //
    // ["A", "B", "C", "B", "D"] will create (N - 1)/2 transmission gates
    // connected as follows:
    //
    //       _         _
    //       B         B
    //      -o-       -o-
    // A   +---+  C  +---+   D
    // ----+   +-----+   +----
    //     +---+     +----
    //      ---       ---
    //       B         B
    //
    // This is one sequence. If multiple sequences are provided, they will be
    // joined according to the horizontal pitch/minimum spacing rules.
    std::vector<std::vector<std::string>> sequences = {
        {"A", "B", "C", "B", "D", "E", "D", "G", "E", "I", "J", "K", "L", "M",
         "N", "O", "P", "O"}
    };

    uint64_t p_width_nm = 1000;
    uint64_t p_length_nm = 150;

    uint64_t n_width_nm = 650;
    uint64_t n_length_nm = 150;

    std::optional<uint64_t> li_width_nm;

    // The height of the transmission gate cells. If not specified, transmission
    // gates are sized to fit PMOS and NMOS FETs as close to each other as
    // possible. TODO(aryap): including room for routing?
    std::optional<uint64_t> min_height_nm = 2720;

    // Vertical pitch of ports across the gates, if specified.
    std::optional<uint64_t> vertical_pitch_nm = 400;

    // Horizontal pitch of cell. Usually the width of the smallest site in the
    // std. cell library.
    std::optional<uint64_t> horizontal_pitch_nm = 460;

    // Horizontal pitch of ports across the gates. Will force transistor spacing
    // as well.
    std::optional<uint64_t> poly_pitch_nm = 600;

    std::optional<uint64_t> min_p_tab_diff_separation_nm;
    std::optional<uint64_t> min_n_tab_diff_separation_nm;

    std::optional<uint64_t> min_poly_boundary_separation_nm;

    bool insert_dummy_poly = true;

    bool expand_wells_to_vertical_bounds = true;

    void ToProto(proto::parameters::Sky130TransmissionGateStack *pb) const;
    void FromProto(const proto::parameters::Sky130TransmissionGateStack &pb);
  };

  Sky130TransmissionGateStack(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}
  bfg::Cell *Generate() override;

 private:
  static constexpr char kMetalLayer[] = "li.drawing";

  void BuildSequence(
      const std::vector<std::string> &net_sequence,
      size_t *gates_so_far,
      bfg::Cell *cell,
      RowGuide *row,
      std::optional<geometry::Rectangle> *pdiff_cover,
      std::optional<geometry::Rectangle> *ndiff_cover,
      std::optional<geometry::Rectangle> *p_poly_via_cover,
      std::optional<geometry::Rectangle> *n_poly_via_cover);

  void ConnectDiffs(const Sky130TransmissionGate &generator,
                    const geometry::Point &top,
                    const geometry::Point &bottom,
                    const std::string &net,
                    Layout *layout);

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_TRANSMISSION_GATE_STACK_H_

