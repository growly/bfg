#include "sky130_interconnect_mux6.h"

#include "../cell.h"
#include "../row_guide.h"
#include "sky130_transmission_gate_stack.h"

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

  RowGuide row(geometry::Point(0, 0),
               cell->layout(),
               cell->circuit(),
               design_db_);

  Sky130TransmissionGateStack::Parameters transmission_gate_mux_params = {
    .net_sequence = {"left", "left_ctrl", "Z", "right_ctrl", "right"},
    .horizontal_pitch_nm = parameters_.poly_pitch_nm
  };

  Sky130TransmissionGateStack generator = Sky130TransmissionGateStack(
      transmission_gate_mux_params, design_db_);
  std::string template_name = absl::StrFormat(
      "%s_gate_stack_template", cell->name());
  Cell *transmission_gate_stack = generator.GenerateIntoDatabase(template_name);
  for (size_t i = 0; i < 3; i++) {
    std::string instance_name = absl::StrFormat(
        "%s_gate_stack_template_%d", cell->name(), i);
    geometry::Instance *instance = row.InstantiateBack(
        instance_name, transmission_gate_stack->layout());

    row.AddBlankSpaceBack(200);
  }

  return cell.release();
}

}   // namespace atoms
}   // namespace bfg
