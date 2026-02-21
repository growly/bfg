#ifndef TILES_SLICE_H_
#define TILES_SLICE_H_

#include <string>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "proto/parameters/slice.pb.h"
#include "tile.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

// A slice should be something like:
//
// +---------------+--------------------------------+---------------+
// |               |                                |               |
// |               |                                |               |
// |               |                                |               |
// |     LUTs      |          Interconnect          |     LUTs      |
// |               |                                |               |
// |               |                                |               |
// |               |                                |               |
// |               |                                |               |
// +---------------+--------------------------------+---------------+
//
class Slice : public Tile {
 public:
  struct Parameters {
    size_t num_luts_left = 12;
    size_t num_luts_right = 12;

    // TODO(aryap): Complete.

    void ToProto(proto::parameters::Slice *pb) const;
    void FromProto(const proto::parameters::Slice &pb); 
  };

  Slice(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {
  }

  Cell *Generate() override;

  //void Route(
  //    const std::vector<std::vector<geometry::Instance*>> muxes,
  //    Layout *layout);

 private:
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_SLICE_H_
