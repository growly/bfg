#ifndef ATOMS_ATOM_H_
#define ATOMS_ATOM_H_

#include <utility>

#include "../physical_properties_database.h"

namespace bfg {

class Cell;

namespace atoms {

class Atom {
 public:
  Atom(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}
  virtual bfg::Cell *Generate() = 0;

 protected:
  const PhysicalPropertiesDatabase &physical_db_;
};

//std::ostream &operator<<(std::ostream &os, const Line &point);

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_ATOM_H_
