#ifndef ATOMS_SKY130_TRANSMISSION_GATE_STACK_H_
#define ATOMS_SKY130_TRANSMISSION_GATE_STACK_H_

#include <cstdint>
#include <memory>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/compass.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../physical_properties_database.h"
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
    // TODO(aryap): Right?
    std::vector<std::string> net_sequence =
        {"A", "B", "C", "B", "D", "E", "F", "G", "H", "I", "J", "K"};

    std::optional<uint64_t> li_width_nm;

    // The height of the transmission gate cells. If not specified, transmission
    // gates are sized to fit PMOS and NMOS FETs as close to each other as
    // possible. TODO(aryap): including room for routing?
    std::optional<uint64_t> height_nm = 2720;

    // Vertical pitch of ports across the gates, if specified.
    std::optional<uint64_t> vertical_pitch_nm = 300;

    // Horizontal pitch of ports across the gates. Will force transistor spacing
    // as well.
    std::optional<uint64_t> horizontal_pitch_nm = 600;
  };

  Sky130TransmissionGateStack(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}
  bfg::Cell *Generate() override;

 private:
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

