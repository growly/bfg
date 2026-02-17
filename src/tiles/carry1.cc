#include "carry1.h"

#include "../cell.h"
#include "proto/parameters/carry1.pb.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_tap.h"
#include "../atoms/sky130_xor2.h"

namespace bfg {
namespace tiles {

void Carry1::Parameters::ToProto(proto::parameters::Carry1 *pb) const {
}

void Carry1::Parameters::FromProto(const proto::parameters::Carry1 &pb) {
}

Cell *Carry1::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  RowGuide row({0, 0}, cell->layout(), cell->circuit(), design_db_);

  // Set the tap cell:
  { 
    atoms::Sky130Tap::Parameters tap_params = {
      .height_nm = 2720,
      .width_nm = atoms::Sky130Parameters::kStandardCellUnitWidthNm
    };
    atoms::Sky130Tap tap_generator(tap_params, design_db_);
    Cell *tap_cell = tap_generator.GenerateIntoDatabase("tap");
    row.set_tap_cell(*tap_cell);
  };

  geometry::Instance *config_memory = AddConfigMemory(&row);
  geometry::Instance *generate_select = AddGenerateSelectMux(&row);
  geometry::Instance *sum_xor = AddSumXor(&row);
  geometry::Instance *carry_select = AddCarrySelectMux(&row);

  return cell.release();
}

geometry::Instance * Carry1::AddConfigMemory(RowGuide *row) const {
  std::string template_name = "config_memory";
  std::string instance_name = absl::StrCat(template_name, "_i");
  atoms::Sky130Dfxtp::Parameters params = {
    .input_clock_buffer = true
  };
  atoms::Sky130Dfxtp generator(params, design_db_);
  Cell *register_cell = generator.GenerateIntoDatabase(template_name);
  register_cell->layout()->DeletePorts("QI");
  geometry::Instance *installed = row->InstantiateBack(
      instance_name, register_cell);
  return installed;
}

geometry::Instance* Carry1::AddGenerateSelectMux(RowGuide *row) const {
  std::string template_name = "generate_select";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130HdMux21 mux_generator({}, design_db_);
  Cell *mux_cell = mux_generator.GenerateIntoDatabase(template_name);
  mux_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, mux_cell);
  return installed;
}

geometry::Instance* Carry1::AddSumXor(RowGuide *row) const {
  std::string template_name = "sum_xor2";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130Xor2 xor_generator({}, design_db_);
  Cell *xor_cell = xor_generator.GenerateIntoDatabase(template_name);
  xor_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, xor_cell);
  return installed;
}

geometry::Instance* Carry1::AddCarrySelectMux(RowGuide *row) const {
  std::string template_name = "carry_select";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130HdMux21 mux_generator({}, design_db_);
  Cell *mux_cell = mux_generator.GenerateIntoDatabase(template_name);
  mux_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, mux_cell);
  return installed;
}

}   // namespace tiles
}   // namespace bfg
