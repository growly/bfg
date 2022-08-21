#ifndef TILES_LUT_H_
#define TILES_LUT_H_

#include <utility>

#include "tile.h"
#include "../design_database.h"

namespace bfg {

class Cell;
class Layout;
class Circuit;

namespace tiles {

class Lut : public Tile {
 public:
  Lut(DesignDatabase *design_db)
      : Tile(design_db) {}

  virtual bfg::Cell *GenerateIntoDatabase();

 protected:
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_H_
