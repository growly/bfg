#include "lut.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>

#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_mux.h"

namespace bfg {
namespace tiles {

bfg::Cell *Lut::GenerateIntoDatabase(const std::string &name) {
  std::unique_ptr<bfg::Cell> lut_cell(new bfg::Cell("lut"));
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  // Let's assume N = 16 for now.
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      std::string instance_name = absl::StrFormat("lut_dfxtp_%d_%d", i, j);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      bfg::atoms::Sky130Dfxtp::Parameters params;
      bfg::atoms::Sky130Dfxtp generator(params, design_db_);
      bfg::Cell *cell = generator.Generate();
      cell->set_name(cell_name);
      design_db_->ConsumeCell(cell);
      circuit->AddInstance(instance_name, cell->circuit());
      geometry::Rectangle bounding_box = cell->layout()->GetBoundingBox();
      int64_t x_pos = static_cast<int64_t>(i * bounding_box.Width());
      int64_t y_pos = static_cast<int64_t>(j * bounding_box.Height());
      geometry::Instance geo_instance(
          cell->layout(), geometry::Point { x_pos, y_pos });
      geo_instance.set_name(instance_name);
      layout->AddInstance(geo_instance);
    }
  }

  bfg::atoms::Sky130Mux::Parameters mux_params;
  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *mux_cell = mux.GenerateIntoDatabase("mux_template");

  //circuit->AddInstance("mux", mux_cell->circuit());
  geometry::Rectangle bounding_box = layout->GetBoundingBox();
  int64_t x_pos = static_cast<int64_t>(bounding_box.Width());
  int64_t y_pos = static_cast<int64_t>(bounding_box.Height());
  geometry::Instance geo_instance(
      mux_cell->layout(), geometry::Point { x_pos + 500, 0 });
  geo_instance.set_name("mux");
  layout->AddInstance(geo_instance);


  lut_cell->SetLayout(layout.release());
  lut_cell->SetCircuit(circuit.release());
  bfg::Cell *cell = lut_cell.release();
  cell->set_name(name);
  design_db_->ConsumeCell(cell);
  return cell;
}

}  // namespace atoms
}  // namespace bfg
