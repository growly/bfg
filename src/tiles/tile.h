#ifndef TILES_TILE_H_
#define TILES_TILE_H_

#include <string>
#include <utility>
#include <absl/strings/str_cat.h>

#include "../design_database.h"

namespace bfg {

class Cell;

namespace tiles {

class Tile {
 public:
  Tile(DesignDatabase *design_db)
      : design_db_(design_db) {}

  virtual bfg::Cell *GenerateIntoDatabase(const std::string &name) {
    // The name of the generator, which does not necessarily have to be used by
    // the Generate() function. (That's why we have to override it explicitly in
    // the generated cell below.)
    name_ = name;

    bfg::Cell *cell = Generate();
    cell->set_name(name);
    design_db_->ConsumeCell(cell);
    return cell;
  }

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  const std::string PrefixCellName(const std::string &value) const {
    return absl::StrCat(name_, "_", value);
  }

  virtual bfg::Cell *Generate() = 0;
 protected:
  std::string name_;

  DesignDatabase *design_db_;
};

//std::ostream &operator<<(std::ostream &os, const Line &point);

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_TILE_H_
