#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"

namespace bfg {
namespace atoms {

class Sky130Dfxtp: public Atom {
 public:
  struct Parameters {
    uint64_t width_nm;
    uint64_t height_nm;

//    uint64_t x0_width_nm;
//    uint64_t x1_width_nm;
//    uint64_t x2_width_nm;
//    uint64_t x3_width_nm;
  };

  Sky130Dfxtp(const PhysicalPropertiesDatabase &physical_db,
              const Parameters &parameters)
      : Atom(physical_db),
        parameters_(parameters),
        internal_units_per_nm_(1.0) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  Parameters parameters_;

  double internal_units_per_nm_;
};

}  // namespace atoms
}  // namespace bfg
