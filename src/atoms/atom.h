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
  Atom(DesignDatabase *design_db)
      : Tile(design_db) {}

  bfg::Cell *GenerateIntoDatabase() override { return nullptr; }
  virtual bfg::Cell *Generate() = 0;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
