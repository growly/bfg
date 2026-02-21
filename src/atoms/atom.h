#ifndef ATOMS_ATOM_H_
#define ATOMS_ATOM_H_

#include <utility>

#include "../design_database.h"
#include "../tiles/tile.h"

namespace bfg {

class Cell;

namespace atoms {

// FIXME(aryap): There is a difference between "Tiles" and "Atoms" but the
// code isn't it. Atoms are PDK-specific generators for different structures
// and circuits, while Tiles are meant to be more PDK-agnostic, able to
// switch between the atoms they use depending on higher level design
// concerns. But the code for the two is very similar.
class Atom : public tiles::Tile {
 public:
  Atom(DesignDatabase *design_db)
      : Tile(design_db) {}
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
