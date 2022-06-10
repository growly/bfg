#include "atom.h"
#include "../cell.h"

namespace bfg {

class Cell;

namespace atoms {

class Sky130Buf: public Atom {
 public:
  Sky130Buf(const PhysicalPropertiesDatabase &physical_db)
      : Atom(physical_db) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;
};

}  // namespace atoms
}  // namespace bfg
