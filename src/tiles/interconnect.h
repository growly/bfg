#ifndef TILES_INTERCONNECT_H_
#define TILES_INTERCONNECT_H_

#include "atom.h"
#include "../memory_bank.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/instance.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

class Interconnect : public Atom {
 public:
  struct Parameters {
  };

  Interconnect(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
  }

  bfg::Cell *Generate() override;

 private:
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_INTERCONNECT_H_
