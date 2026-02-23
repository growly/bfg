#ifndef ATOMS_SKY130_PASSIVE_MUX3_H_
#define ATOMS_SKY130_PASSIVE_MUX3_H_

#include <string>
#include <vector>

#include "../circuit.h"
#include "../geometry/instance.h"
#include "../geometry/point.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../routing_grid.h"
#include "../cell.h"
#include "sky130_parameters.h"
#include "proto/parameters/sky130_passive_mux3.pb.h"
#include "atom.h"
#include "../row_guide.h"

namespace bfg {

class DesignDatabase;
class RouteManager;

namespace atoms {

// A passive transmission-gate mux.
class Sky130PassiveMux3 : public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    // We can probably support 2-4.
    int num_inputs = 3;

    void ToProto(proto::parameters::Sky130PassiveMux3 *pb) const;
    void FromProto(const proto::parameters::Sky130PassiveMux3 &pb); 
  };

  static constexpr char kStackOutputName[] = "Z";
  static constexpr char kMuxOutputName[] = "OUT";

  Sky130PassiveMux3(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
  }

  Cell *Generate() override;

 private:
  void GenerateLayout(RowGuide *row) const;
  void GenerateCircuit(bfg::Circuit *circuit) const;

  std::vector<std::vector<std::string>> BuildNetSequences() const;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_PASSIVE_MUX3_H_
