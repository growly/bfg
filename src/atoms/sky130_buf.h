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
    uint64_t width;
    uint64_t height;
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
