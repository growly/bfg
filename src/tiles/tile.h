#ifndef TILES_TILE_H_
#define TILES_TILE_H_

#include <utility>

#include "../design_database.h"

namespace bfg {

class Cell;

namespace tiles {

class Tile {
 public:
  Tile(DesignDatabase *design_db)
      : design_db_(design_db) {}
  virtual bfg::Cell *GenerateIntoDatabase() = 0;

 protected:
  DesignDatabase *design_db_;
};

//std::ostream &operator<<(std::ostream &os, const Line &point);

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_TILE_H_
