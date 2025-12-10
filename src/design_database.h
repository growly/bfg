#ifndef DESIGN_DATABASE_H_
#define DESIGN_DATABASE_H_

#include <map>
#include <unordered_map>
#include <ostream>
#include <string>

#include "abstract_via.h"
#include "cell.h"
#include "geometry/layer.h"
#include "geometry/rectangle.h"
#include "circuit_primitives_database.h"
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

  bool ConsumeCell(Cell *cell);

  Cell *FindCellOrDie(const std::string &domain, const std::string &name) const;
  Cell *FindCellOrDie(const std::string &name) const;
  Cell *FindCell(const std::string &domain, const std::string &name) const;

  const std::unordered_map<std::string,
      std::unordered_map<std::string, std::unique_ptr<Cell>>> &cells() const {
      return cells_;
  }

  PhysicalPropertiesDatabase &physical_db() { return physical_db_; }
  const PhysicalPropertiesDatabase &physical_db() const { return physical_db_; }

  CircuitPrimitivesDatabase &primitives_db() { return primitives_db_; }
  const CircuitPrimitivesDatabase &primitives_db() const {
    return primitives_db_;
  }

  void WriteTop(
      const std::string &top_name,
      const std::string &library_path,
      const std::string &package_path,
      bool include_text_format) const;
  void WriteTop(
      const Cell &top,
      const std::string &library_path,
      const std::string &package_path,
      bool include_text_format) const;

  std::string Describe() const;

 private:
  static void WriteCellsToVLSIRLibrary(
      const std::vector<Cell*> cells,
      const std::string &file_name,
      bool include_text_format);

  static void WriteCellsToVLSIRPackage(
      const std::vector<Cell*> cells,
      const std::string &file_name,
      bool include_text_format);


  // Find an ordering that places dependencies first.
  static void OrderCells(
      const std::vector<Cell*> &unordered_cells,
      std::vector<Cell*> *ordered_cells);

  // PartitionAndOrderCells(std::vector<Cell*> *ordered_cells);
  // PruneAndOrderCells(...);

  void LoadModule(const vlsir::circuit::Module &module_pb);
  void LoadExternalModule(const vlsir::circuit::ExternalModule &module_pb);

  Cell *FindOrCreateCell(const std::string &domain, const std::string &name);

  PhysicalPropertiesDatabase physical_db_;

  CircuitPrimitivesDatabase primitives_db_;

  // Known cells, indexed by domain (outer) and then name (inner). Where a
  // domain is not present, the key is "".
  std::unordered_map<std::string,
      std::unordered_map<std::string, std::unique_ptr<Cell>>> cells_;
};


}  // namespace bfg

#endif  // DESIGN_DATABASE_H_
