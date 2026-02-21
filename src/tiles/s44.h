#ifndef TILES_S44_H_
#define TILES_S44_H_

#include "tile.h"

#include "proto/parameters/s44.pb.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

// A (soft) S44 LUT is two 4-LUTs with a configurable path from the input of the
// first to the output of the second. When acting as independent 4-LUTs, the two
// LUTs can benefit from carry logic to implement arithmetic.
//
// This generator stacks two S44 vertically (or horizontally, one day) with this
// additional logic and adds a Carry1 cell.
//
//               +-----+
//               |     |-+
//               |>    | |
//               +-----+ |
//                       |
//      +----------+     |
//    --|          |--------------------------- A_MUX
//    --|  4-LUT   |--+------------------------ A_O
//    --|    A     |  |  |   
//    --|          |  |  |   
//      +----------+  | |\
//                    +-| |     +----------+
//                  ----| |-----|          |--- B_MUX
//                      |/    --|  4-LUT   |--- B_O
//                            --|    B     |
//                            --|          |
//                              +----------+
class S44 : public Tile {
 public:
  struct Parameters {
    void ToProto(proto::parameters::S44 *pb) const {}
    void FromProto(const proto::parameters::S44 &pb) {}
  };

  S44(const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {}

  Cell *Generate() override;

 private:
  Parameters parameters_;
};

}  // namespace tiles
}  // namespace bfg

#endif  // TILES_S44_H_
