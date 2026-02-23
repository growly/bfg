#include "sky130_passive_mux3.h"

#include <cmath>
#include <memory>

#include "../row_guide.h"
#include "../cell.h"
#include "sky130_transmission_gate_stack.h"
#include "sky130_buf.h"

namespace bfg {
namespace atoms {

void Sky130PassiveMux3::Parameters::ToProto(
    proto::parameters::Sky130PassiveMux3 *pb) const {
}

void Sky130PassiveMux3::Parameters::FromProto(
    const proto::parameters::Sky130PassiveMux3 &pb) {
}

bfg::Cell *Sky130PassiveMux3::Generate() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(
      new Cell(name_.empty() ? "sky130_passive_mux3": name_));

  Layout *layout = new bfg::Layout(db);
  Circuit *circuit = new bfg::Circuit();

  cell->SetCircuit(circuit);
  cell->SetLayout(layout);

  RowGuide row({0, 0}, layout, cell->circuit(), design_db_);
  row.clear_tap_cell();

  GenerateLayout(&row);

  return cell.release();
}

void Sky130PassiveMux3::GenerateLayout(RowGuide *row) const {
  Sky130TransmissionGateStack::Parameters stack_params = {
    .sequences = BuildNetSequences(),
    .p_width_nm = 420,
    .n_width_nm = 360,
    .expand_wells_to_vertical_bounds = true,
    .expand_wells_to_horizontal_bounds = true
  };
  // TODO(aryap): Make parameters.
  stack_params.power_net = parameters_.power_net,
  stack_params.ground_net = parameters_.ground_net,
  stack_params.min_height_nm = 2720;
  stack_params.num_horizontal_channels = parameters_.num_inputs + 1;
  //stack_params.poly_contact_vertical_pitch_nm = 340;
  //stack_params.poly_contact_vertical_offset_nm = 170;
  //stack_params.input_vertical_pitch_nm = 340;
  //stack_params.input_vertical_offset_nm = 170;
  //stack_params.poly_pitch_nm = 500;

  std::string stack_template_name = PrefixCellName("gate_stack");
  std::string stack_instance_name = absl::StrCat(stack_template_name, "_i");

  Sky130TransmissionGateStack generator =
      Sky130TransmissionGateStack(stack_params, design_db_);
  Cell *stack_cell = generator.GenerateIntoDatabase(stack_template_name);
  row->InstantiateBack(stack_instance_name, stack_cell);

  int64_t actual_height = row->GetTilingBounds()->Height();

  std::string buf_template_name = PrefixCellName("buf");
  Sky130Buf::Parameters buf_params = {
    .height_nm = static_cast<uint64_t>(
        design_db_->physical_db().ToExternalUnits(actual_height))
  };
  buf_params.power_net = parameters_.power_net,
  buf_params.ground_net = parameters_.ground_net,
  buf_params.draw_vpwr_vias = true;
  buf_params.draw_vgnd_vias = true;
  Sky130Buf buf_generator(buf_params, design_db_);
  bfg::Cell *buf_cell = buf_generator.GenerateIntoDatabase(buf_template_name);

  // FIXME(aryap): Since we have to do some crazy routing, the best way to do
  // this is to actually connect 1x buf to a 2-input transmission gate mux and
  // then copy flip that (horizontally) for double the number of inputs.
  size_t num_buffers = std::ceil(
      static_cast<double>(parameters_.num_inputs) / 2.0);
  for (size_t i = 0; i < num_buffers; i++) {
    std::string buf_instance_name = absl::StrCat(buf_template_name, "_i", i);

    if (i % 2 == 0) {
      row->InstantiateFront(buf_instance_name, buf_cell);
    } else {
      row->InstantiateBack(buf_instance_name, buf_cell);
    }
  }
}

std::vector<std::vector<std::string>>
Sky130PassiveMux3::BuildNetSequences() const {
  std::vector<std::vector<std::string>> sequences;
  std::vector<std::string> last_sequence;
  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    std::string input_name = absl::StrFormat("X%u", i);
    std::string control_name = absl::StrFormat("S%u", i);

    if (last_sequence.size() == 0) {
      last_sequence.push_back(input_name);
      last_sequence.push_back(control_name);
      last_sequence.push_back(kStackOutputName);
    } else {
      last_sequence.push_back(control_name);
      last_sequence.push_back(input_name);
      sequences.push_back(last_sequence);
      last_sequence.clear();
    }
  }
  // For odd numbers of inputs we have to push the shorter sequence.
  if (!last_sequence.empty()) {
    sequences.push_back(last_sequence);
    last_sequence.clear();
  }
  return sequences;
}

}   // namespace atoms
}   // namespace bfg
