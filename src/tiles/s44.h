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
//    --|          |--------------------------- O_MUX1
//    --|  4-LUT   |--+------------------------ O1
//    --|    A     |  |  |   
//    --|          |  |  |   
//      +----------+  | |\
//                    +-| |     +----------+
//                  ----| |-----|          |--- O_MUX2
//                      |/    --|  4-LUT   |--- O2
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

  Cell *GenerateIntoDatabase(const std::string &name) override;

 private:
  Parameters parameters_;
};

}  // namespace tiles
}  // namespace bfg

#endif  // TILES_S44_H_
