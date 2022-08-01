#ifndef ATOMS_ATOM_H_
#define ATOMS_ATOM_H_

#include <utility>

#include "../physical_properties_database.h"
#include "../tiles/tile.h"

namespace bfg {

class Cell;

namespace atoms {

class Atom : public tiles::Tile {
 public:
  Atom(const PhysicalPropertiesDatabase &physical_db)
      : Tile(physical_db) {}
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
