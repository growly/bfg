#include "sky130_interconnect_mux6.h"

#include "../cell.h"
#include "../memory_bank.h"
#include "../row_guide.h"
#include "../geometry/compass.h"
#include "sky130_tap.h"
#include "sky130_transmission_gate_stack.h"
#include "sky130_dfxtp.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_cat.h>

namespace bfg {
namespace atoms {

void Sky130InterconnectMux6::Parameters::ToProto(
    proto::parameters::Sky130InterconnectMux6 *pb) const {
  if (poly_pitch_nm) {
    pb->set_poly_pitch_nm(*poly_pitch_nm);
  } else {
    pb->clear_poly_pitch_nm();
  }
}

void Sky130InterconnectMux6::Parameters::FromProto(
    const proto::parameters::Sky130InterconnectMux6 &pb) {
  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  } else {
    poly_pitch_nm.reset();
  }
}

bfg::Cell *Sky130InterconnectMux6::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_interconnect_mux6": name_));

  // FIXME(aryap): Do this! Make the circuit!
  //cell->SetCircuit(GenerateCircuit());

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  cell->SetLayout(new bfg::Layout(db));

  atoms::Sky130Tap::Parameters tap_params = {
    .height_nm = 2720,
    .width_nm = 460
  };
  atoms::Sky130Tap tap_generator(tap_params, design_db_);
  Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      "interconnect_mux6_tap_template");

  MemoryBank bank = MemoryBank(cell->layout(),
                               design_db_,
                               tap_cell,
                               true,      // Rotate alternate rows.
                               true,      // Rotate the first row.
                               geometry::Compass::LEFT);

  for (size_t i = 0; i < 3; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_bottom_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    bank.InstantiateRight(i, instance_name, dfxtp_cell->layout());
  }

  //atoms::Sky130Tap::Parameters tap_params = {
  //  .height_nm = 2720,
  //  .width_nm = 460
  //};
  //atoms::Sky130Tap tap_generator(tap_params, design_db_);
  //Cell *tap_cell = tap_generator.GenerateIntoDatabase(
  //    "interconnect_mux6_tap_template");
  //
  Sky130TransmissionGateStack::Parameters transmission_gate_mux_params = {
    .sequences = {
      {"X0", "S0", "Z", "S1", "X1"},
      {"X2", "S2", "Z", "S3", "X3"},
      {"X4", "S4", "Z", "S5", "X5"}
    },
    .horizontal_pitch_nm = parameters_.poly_pitch_nm
  };
  Sky130TransmissionGateStack generator = Sky130TransmissionGateStack(
      transmission_gate_mux_params, design_db_);
  std::string instance_name = absl::StrFormat("%s_gate_stack", cell->name());
  std::string template_name = absl::StrCat(instance_name, "_template");
  Cell *transmission_gate_stack = generator.GenerateIntoDatabase(template_name);
  bank.Row(3).clear_tap_cell();
  bank.Row(3).AddBlankSpaceAndInsertFront(460);   // Width of a tap, above.
  bank.InstantiateRight(3, instance_name, transmission_gate_stack->layout());

  for (size_t i = 4; i < 7; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_top_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    bank.InstantiateRight(i, instance_name, dfxtp_cell->layout());
  }

  return cell.release();
}

}   // namespace atoms
}   // namespace bfg
