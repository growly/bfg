#include "fabric.h"

#include "../circuit.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "reduced_tile.h"

namespace bfg {
namespace tiles {


Cell *Fabric::Generate() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name_));

  bfg::Circuit *circuit = new bfg::Circuit();
  cell->SetCircuit(circuit);
  bfg::Layout *layout = new bfg::Layout(db);
  cell->SetLayout(layout);

  MemoryBank bank = MemoryBank(layout,
                               circuit,
                               design_db_,
                               nullptr,    // No tap cells.
                               false,      // Rotate alternate rows.
                               false,      // Rotate first row.
                               geometry::Compass::LEFT);


  ReducedTile tile_generator({}, design_db_);
  Cell *tile_cell = tile_generator.GenerateIntoDatabase("reduced_tile");

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      std::string name = absl::StrCat(tile_cell->name(), "_i", i, "-", j);
      bank.InstantiateRight(i, name, tile_cell);
    }
  }

  return cell.release();
}

}  // namespace tiles
}  // namespace bfg
