#include "sky130_interconnect_mux6.h"

#include "../cell.h"
#include "../memory_bank.h"
#include "../row_guide.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
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

Sky130TransmissionGateStack::Parameters
Sky130InterconnectMux6::BuildTransmissionGateParams(
    Cell *vertical_neighbour) const {
  static const std::string kOutputName = "Z";

  Sky130TransmissionGateStack::Parameters params = {
    .sequences = {},
    .horizontal_pitch_nm = parameters_.poly_pitch_nm,
    .min_poly_boundary_separation_nm =
        FigurePolyBoundarySeparationForMux(vertical_neighbour)
  };

  uint32_t needed_tracks = parameters_.num_inputs;
  if (parameters_.vertical_pitch_nm) {
    params.min_height_nm = (needed_tracks + 3) * *parameters_.vertical_pitch_nm;
  }
  params.vertical_pitch_nm = parameters_.vertical_pitch_nm;

  // Build the sequences of nets that dictate the arrangement of the
  // transmission gate stack, e.g.
  // {
  //   {"X0", "S0", "Z", "S1", "X1"},
  //   {"X2", "S2", "Z", "S3", "X3"},
  //   {"X4", "S4", "Z", "S5", "X5"},
  //   {"X6", "S6", "Z"}                // For the 7th input.
  // }
  std::vector<std::string> last_sequence;
  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    std::string input_name = absl::StrFormat("X%u", i);
    std::string control_name = absl::StrFormat("S%u", i);

    if (last_sequence.size() == 0) {
      last_sequence.push_back(input_name);
      last_sequence.push_back(control_name);
      last_sequence.push_back(kOutputName);
    } else {
      last_sequence.push_back(control_name);
      last_sequence.push_back(input_name);
      params.sequences.push_back(last_sequence);
      last_sequence.clear();
    }
  }
  // For odd numbers of inputs we have to push the shorter sequence.
  if (!last_sequence.empty()) {
    params.sequences.push_back(last_sequence);
    last_sequence.clear();
  }
  return params;
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

  uint32_t num_ff = parameters_.num_inputs;
  uint32_t num_ff_top = num_ff / 2; 
  uint32_t num_ff_bottom = num_ff - num_ff_top;

  Cell *example_vertical_neighbour;
  for (size_t i = 0; i < num_ff_bottom; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_bottom_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    example_vertical_neighbour = dfxtp_cell;
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
  Sky130TransmissionGateStack::Parameters transmission_gate_mux_params =
      BuildTransmissionGateParams(example_vertical_neighbour);
  Sky130TransmissionGateStack generator = Sky130TransmissionGateStack(
      transmission_gate_mux_params, design_db_);
  std::string instance_name = absl::StrFormat("%s_gate_stack", cell->name());
  std::string template_name = absl::StrCat(instance_name, "_template");
  Cell *transmission_gate_stack = generator.GenerateIntoDatabase(template_name);
  bank.Row(num_ff_bottom).clear_tap_cell();
  // Width of a tap, above.
  bank.Row(num_ff_bottom).AddBlankSpaceAndInsertFront(460);
  bank.InstantiateRight(
      num_ff_bottom, instance_name, transmission_gate_stack->layout());

  for (size_t i = num_ff_bottom + 1; i < num_ff + 1; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_top_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    bank.InstantiateRight(i, instance_name, dfxtp_cell->layout());
  }

  // TODO(aryap): Clock buffer, output buffer, decap fillers.

  // Connect flip-flop outputs to transmission gates. Flip-flops store one bit
  // and output both the bit and its complement, conveniently. Per description
  // in header, start with left-most gates from the

  return cell.release();
}

// We will determine the minimum vertical poly-to-boundary spacing such that any
// rectangle (wire) placed at that inset from the edge on any of the poly, li or
// met1 layers will be far enough away from shapes in the surrounding cell on
// the same layers. We only care how far shapes in the surrounding cell overflow
// its tiling bounds, since that is the incursion into what will be the mux
// cell. We also assume uniformity across the width of the cell.
int64_t Sky130InterconnectMux6::FigurePolyBoundarySeparationForMux(
    bfg::Cell *surrounding_cell) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  bfg::Layout *neighbour = surrounding_cell->layout();

  bfg::geometry::Rectangle tiling_bounds = neighbour->GetTilingBounds();

  static const std::vector kCheckedLayers = {
    "poly.drawing", "met1.drawing", "li.drawing"
  };

  int64_t max_spacing = 0;

  for (const std::string &layer : kCheckedLayers) {
    bfg::geometry::Rectangle layer_bounds =
        neighbour->GetBoundingBoxByNameOrDie(layer);

    // Minimum separation on this layer.
    int64_t layer_min_separation = db.Rules(layer).min_separation;

    // First check the top:
    int64_t overflow =
        layer_bounds.upper_right().y() - tiling_bounds.upper_right().y();
    int64_t required_separation = overflow + layer_min_separation;
    max_spacing = std::max(max_spacing, required_separation);
    LOG(INFO) << layer << " min sep. " << layer_min_separation << " now "
              << max_spacing;

    // Then the bottom:
    overflow = tiling_bounds.lower_left().y() - layer_bounds.lower_left().y();
    required_separation = overflow + layer_min_separation;
    max_spacing = std::max(max_spacing, required_separation);
    LOG(INFO) << layer << " min sep. " << layer_min_separation << " now "
              << max_spacing;
  }
  return max_spacing;
}

}   // namespace atoms
}   // namespace bfg
