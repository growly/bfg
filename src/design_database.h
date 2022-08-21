#ifndef DESIGN_DATABASE_H_
#define DESIGN_DATABASE_H_

#include <map>
#include <unordered_map>
#include <ostream>

#include "abstract_via.h"
#include "cell.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"
#include "physical_properties_database.h"
#include "vlsir/circuit.pb.h"
#include "vlsir/tech.pb.h"

namespace bfg {

// Stores (probably external) circuit and layout available to generators in BFG.
class DesignDatabase {
 public:
  DesignDatabase() {}

  void LoadTechnology(const vlsir::tech::Technology &pdk) {}
  void LoadPackage(const vlsir::circuit::Package &package);

  // TODO(growly): Add the cell, and take ownership?
  void AddCell(bfg::Cell *cell);

  const std::unordered_map<
      std::string, std::unique_ptr<bfg::Cell>> &cells() const { return cells_; }

  PhysicalPropertiesDatabase &physical_db() { return physical_db_; }
  const PhysicalPropertiesDatabase &physical_db() const { return physical_db_; }

 private:
  void LoadModule(const vlsir::circuit::Module &module_pb);
  void LoadExternalModule(const vlsir::circuit::ExternalModule &module_pb);

  PhysicalPropertiesDatabase physical_db_;

  // Known cells, indexed by (possibly fully-qualified, with a domain) name.
  std::unordered_map<std::string, std::unique_ptr<bfg::Cell>> cells_;
};


}  // namespace bfg

#endif  // DESIGN_DATABASE_H_
