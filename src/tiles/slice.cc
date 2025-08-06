#include "slice.h"

#include <string>

#include <absl/strings/str_format.h>
#include "interconnect.h"
#include "lut_b.h"
#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "proto/parameters/interconnect.pb.h"
#include "proto/parameters/lut_b.pb.h"
#include "proto/parameters/slice.pb.h"

namespace bfg {
namespace tiles {

void Slice::Parameters::ToProto(proto::parameters::Slice *pb) const {
  // TODO(aryap): Complete.
}

void Slice::Parameters::FromProto(const proto::parameters::Slice &pb) {
  // TODO(aryap): Complete.
}

Cell *Slice::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  std::vector<std::vector<geometry::Instance*>> muxes;

  std::unique_ptr<bfg::Layout> left_layout(new bfg::Layout(db));
  MemoryBank left_luts = MemoryBank(left_layout.get(),
                                    design_db_,
                                    nullptr,    // No tap cells.
                                    false,      // Rotate alternate rows.
                                    false,      // Rotate first row.
                                    geometry::Compass::LEFT);

  LutB::Parameters default_lut_params = {
      .lut_size = 4
  };
  LutB default_lut_gen(default_lut_params, design_db_);
  std::string lut_name = "lut";
  Cell *default_lut_cell = default_lut_gen.GenerateIntoDatabase(lut_name);

  for (size_t i = 0; i < parameters_.num_luts_left; ++i) {
    geometry::Instance *instance = left_luts.InstantiateRight(
        i / 2, absl::StrCat(lut_name, "_i"), default_lut_cell->layout());
  }

  std::unique_ptr<bfg::Layout> middle_layout(new bfg::Layout(db));
  Interconnect::Parameters interconnect_params;
  Interconnect interconnect_gen(interconnect_params, design_db_);
  Cell *interconnect_cell = interconnect_gen.GenerateIntoDatabase("interconnect");
  geometry::Instance interconnect_instance(
      interconnect_cell->layout(), {0, 0});
  middle_layout->AddInstance(interconnect_instance);
  middle_layout->MoveTo({left_layout->GetTilingBounds().upper_right().x(), 0});

  // FIXME(aryap): This is dumb.
  cell->layout()->AddLayout(*left_layout);
  cell->layout()->AddLayout(*middle_layout);

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
