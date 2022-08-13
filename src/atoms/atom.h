#ifndef ATOMS_ATOM_H_
#define ATOMS_ATOM_H_

#include <utility>

#include "../design_database.h"
#include "../tiles/tile.h"

namespace bfg {

class Cell;

namespace atoms {

class Atom : public tiles::Tile {
 public:
  Atom(const DesignDatabase &design_db)
      : Tile(design_db) {}
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
