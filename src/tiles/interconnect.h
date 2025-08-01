#ifndef TILES_INTERCONNECT_H_
#define TILES_INTERCONNECT_H_

#include <string>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "proto/parameters/interconnect.pb.h"
#include "tile.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

class Interconnect : public Tile {
 public:
  struct Parameters {
    void ToProto(proto::parameters::Interconnect *pb) const;
    void FromProto(const proto::parameters::Interconnect &pb); 
  };

  Interconnect(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {
  }

  Cell *GenerateIntoDatabase(const std::string &name) override;

  // TODO(aryap): Is this const?
  void Route(
      const std::vector<std::vector<geometry::Instance*>> muxes,
      Layout *layout);

  void ConfigureRoutingGrid(RoutingGrid *grid, Layout *layout) const;

 private:
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_H_
