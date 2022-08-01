#ifndef TILES_TILE_H_
#define TILES_TILE_H_

#include <utility>

#include "../physical_properties_database.h"

namespace bfg {

class Cell;

namespace tiles {

class Tile {
 public:
  Tile(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}
  virtual bfg::Cell *Generate() = 0;

 protected:
  const PhysicalPropertiesDatabase &physical_db_;
};

//std::ostream &operator<<(std::ostream &os, const Line &point);

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_TILE_H_
