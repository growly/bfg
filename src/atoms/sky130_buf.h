#include <cstdint>

#include "atom.h"
#include "../layout.h"

namespace bfg {

class Cell;
class Circuit;
class Layout;

namespace atoms {

class Sky130Buf: public Atom {
 public:
  struct Parameters {
    // In internal units:
    uint64_t width;
    uint64_t height;

    double internal_units_per_nm;

    // In real units (e.g. nm = nanometres):
    double x0_width_nm;
    double x1_width_nm;
    double x2_width_nm;
    double x3_width_nm;
  };

  Sky130Buf(const PhysicalPropertiesDatabase &physical_db,
            const Parameters &parameters)
      : Atom(physical_db),
        parameters_(parameters) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  Layout *GenerateLayout();
  Circuit *GenerateCircuit();

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg
