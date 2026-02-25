#ifndef ATOMS_SKY130_CARRY1_H_
#define ATOMS_SKY130_CARRY1_H_

#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../geometry/point.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "../cell.h"
#include "proto/parameters/sky130_carry1.pb.h"
#include "atom.h"
#include "sky130_parameters.h"

namespace bfg {

class DesignDatabase;
class RouteManager;

namespace atoms {

// Sky130Carry1 forms 1 bit of the carry chain:
//
//
//                                               +-----+
//                               P  --------+----| XOR |---- S
//                                      +---|----|     |
//                             C_I  ----+   |    +-----+
//                                      |   |     
//                +-------+   +---------|---|--------------- CONFIG_OUT
//     CONFIG_IN -| FF    |   |         |   |
//                |       |---+         |  |\
//    CONFIG_CLK -|>      |   |         +--| |-------------- C_O
//                +-------+   |     +------| |
//                            |     |      |/  2:1 MUX
//                           |\     |
//                      G_0 -| |----+ (i_0)
//                      G_1 -| |
//                           |/  2:1 MUX
//
// Inputs:
//    C_I, carry in, typically from a prior Sky130Carry1 bit
//    P, propagate, typically from a LUT
//    G_0, generate, typically from another LUT
//    G_1, generate, typically from a bypass input
//    CONFIG_IN, scan-chain config in
//    CONFIG_CLK, scan-chain clock
//
// Outputs:
//    S, the sum output
//    C_O, carry out
//    CONFIG_OUT, scan-chain config out
//
class Sky130Carry1 : public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    bool reverse_order = false;

    void ToProto(proto::parameters::Sky130Carry1 *pb) const;
    void FromProto(const proto::parameters::Sky130Carry1 &pb); 
  };

  Sky130Carry1(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
  }

  geometry::Instance *AddConfigMemory(RowGuide *row) const;
  geometry::Instance *AddGenerateSelectMux(RowGuide *row) const;
  geometry::Instance *AddSumXor(RowGuide *row) const;
  geometry::Instance *AddCarrySelectMux(RowGuide *row) const;

  Cell *Generate() override;

 private:
  void GenerateCircuit(const std::vector<geometry::Instance*> &taps,
                       geometry::Instance *config_memory,
                       geometry::Instance *generate_select,
                       geometry::Instance *carry_select,
                       geometry::Instance *sum_xor,
                       bfg::Circuit *circuit) const;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_CARRY1_H_
