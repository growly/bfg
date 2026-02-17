#ifndef TILES_CARRY1_H_
#define TILES_CARRY1_H_

#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../geometry/point.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "../cell.h"
#include "proto/parameters/carry1.pb.h"
#include "tile.h"

namespace bfg {

class DesignDatabase;
class RouteManager;

namespace tiles {

// Carry1 forms 1 bit of the carry chain:
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
//                +-------+   |     +---- -| |
//                            |     |      |/  2:1 MUX
//                           |\     |
//                      G_0 -| |----+
//                      G_1 -| |
//                           |/  2:1 MUX
//
// Inputs:
//    C_I, carry in, typically from a prior Carry1 bit
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
class Carry1 : public Tile {
 public:
  struct Parameters {
    void ToProto(proto::parameters::Carry1 *pb) const;
    void FromProto(const proto::parameters::Carry1 &pb); 
  };

  Carry1(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {
  }

  geometry::Instance* AddConfigMemory(RowGuide *row) const;
  geometry::Instance* AddGenerateSelectMux(RowGuide *row) const;
  geometry::Instance* AddSumXor(RowGuide *row) const;
  geometry::Instance* AddCarrySelectMux(RowGuide *row) const;

  Cell *GenerateIntoDatabase(const std::string &name) override;

 private:
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_CARRY1_H_
