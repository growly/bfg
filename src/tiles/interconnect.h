#ifndef TILES_INTERCONNECT_H_
#define TILES_INTERCONNECT_H_

#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../geometry/point.h"
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
  typedef std::vector<std::vector<geometry::Instance*>> MuxCollection;
  typedef std::vector<std::vector<std::vector<geometry::Port*>>>
      InputPortCollection;
  typedef std::vector<std::vector<geometry::Port*>>
      OutputPortCollection;

  struct Parameters {
    size_t num_rows = 8;
    size_t num_columns = 16;
    //size_t num_rows = 2;
    //size_t num_columns = 4;
    //size_t num_rows = 8;
    //size_t num_columns = 16;


    // TODO(aryap): Complete.
    void ToProto(proto::parameters::Interconnect *pb) const;
    void FromProto(const proto::parameters::Interconnect &pb); 
  };

  Interconnect(
      const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {
  }

  Cell *GenerateIntoDatabase(const std::string &name) override;

  void ConfigureRoutingGrid(RoutingGrid *grid, Layout *layout) const;

 private:
  // TODO(aryap): Is this const?
  void Route(
      const MuxCollection &muxes,
      const InputPortCollection &mux_inputs,
      const OutputPortCollection &mux_outputs,
      Layout *layout);
  void RouteComplete(
      const MuxCollection &muxes,
      const InputPortCollection &mux_inputs,
      const OutputPortCollection &mux_outputs,
      Layout *layout);

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_H_
