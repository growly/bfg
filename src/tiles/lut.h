#ifndef TILES_LUT_H_
#define TILES_LUT_H_

#include <utility>

#include "tile.h"
#include "../physical_properties_database.h"

namespace bfg {

class Cell;
class Layout;
class Circuit;

namespace tiles {

class Lut : public Tile {
 public:
  Lut(const PhysicalPropertiesDatabase &physical_db)
      : Tile(physical_db) {}

  virtual bfg::Cell *Generate();

 protected:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_H_
