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

  bfg::Cell *GenerateIntoDatabase(const std::string &name) override {
    // The name of the generator, which does not necessarily have to be used by
    // the Generate() function. (That's why we have to override it explicitly in
    // the generated cell below.)
    name_ = name;

    bfg::Cell *cell = Generate();
    cell->set_name(name);
    design_db_->ConsumeCell(cell);
    return cell;
  }

  virtual bfg::Cell *Generate() = 0;
 protected:
  std::string name_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
