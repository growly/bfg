#include "sky130_interconnect_mux6.h"

#include "../modulo.h"
#include "../cell.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../row_guide.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
#include "../geometry/instance.h"
#include "sky130_buf.h"
#include "sky130_tap.h"
#include "sky130_transmission_gate_stack.h"
#include "sky130_dfxtp.h"
#include "sky130_decap.h"

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
    geometry::Instance *vertical_neighbour) const {
  static const std::string kOutputName = "Z";

  Sky130TransmissionGateStack::Parameters params = {
    .sequences = {},
    .poly_pitch_nm = parameters_.poly_pitch_nm,
    .min_poly_boundary_separation_nm = FigurePolyBoundarySeparationForMux(
        vertical_neighbour->template_layout())
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
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(2720U)),
    .width_nm = static_cast<uint64_t>(db.ToExternalUnits(460U))
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

  std::vector<geometry::Instance*> bottom_memories;
  for (size_t i = 0; i < num_ff_bottom; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_bottom_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    geometry::Instance *instance = bank.InstantiateRight(
        i, instance_name, dfxtp_cell->layout());
    bottom_memories.push_back(instance);
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
      BuildTransmissionGateParams(bottom_memories.back());
  Sky130TransmissionGateStack generator = Sky130TransmissionGateStack(
      transmission_gate_mux_params, design_db_);
  std::string instance_name = absl::StrFormat("%s_gate_stack", cell->name());
  std::string template_name = absl::StrCat(instance_name, "_template");
  Cell *transmission_gate_stack_cell =
      generator.GenerateIntoDatabase(template_name);
  bank.Row(num_ff_bottom).clear_tap_cell();
  // Width of a tap, above.
  //bank.Row(num_ff_bottom).AddBlankSpaceAndInsertFront(460);
  geometry::Instance *stack = bank.InstantiateRight(
      num_ff_bottom, instance_name, transmission_gate_stack_cell->layout());

  std::vector<geometry::Instance*> top_memories;
  for (size_t i = num_ff_bottom + 1; i < num_ff + 1; i++) {
    std::string instance_name = absl::StrFormat("imux6_dfxtp_top_%d", i);
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    geometry::Instance *instance = bank.InstantiateRight(
        i, instance_name, dfxtp_cell->layout());
    top_memories.push_back(instance);
  }

  // The output buffer goes at the end of the transmission gate stack.
  std::string output_buf_name = "output_buf";
  int64_t mux_row_height =
      transmission_gate_stack_cell->layout()->GetTilingBounds().Height();
  Sky130Buf::Parameters output_buf_params = {
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(mux_row_height))
  };
  Sky130Buf output_buf_generator(output_buf_params, design_db_);
  Cell *output_buf_cell = output_buf_generator.GenerateIntoDatabase(
      output_buf_name);
  geometry::Instance *output_buf_instance = bank.InstantiateRight(
      num_ff_bottom,
      absl::StrCat(output_buf_name, "_instance"),
      output_buf_cell->layout());

  // The input clock buffers go next to the middle flip flop on the top and
  // bottom side.
  std::string clk_buf_name = "clk_buf";
  Sky130Buf::Parameters clk_buf_params = {};
  Sky130Buf clk_buf_generator(clk_buf_params, design_db_);
  Cell *clk_buf_cell = clk_buf_generator.GenerateIntoDatabase(clk_buf_name);
  geometry::Instance *clk_buf_top = bank.InstantiateRight(
      num_ff_bottom + 1 + (num_ff_top / 2),   // The middle row on top.
      absl::StrCat(clk_buf_name, "_top"),
      clk_buf_cell->layout());
  geometry::Instance *clk_buf_bottom = bank.InstantiateRight(
      num_ff_bottom / 2,   // The middle row on the bottom.
      absl::StrCat(clk_buf_name, "_bottom"),
      clk_buf_cell->layout());

  // Decaps!
  std::string right_decap_name = "decap_right";
  Sky130Decap::Parameters right_decap_params;
  Sky130Decap right_decap_generator(right_decap_params, design_db_);
  Cell *right_decap_cell =
      right_decap_generator.GenerateIntoDatabase(right_decap_name);
  std::set<size_t> skip_rows = {
      num_ff_bottom + 1 + (num_ff_top / 2),   // The middle row on top.
      num_ff_bottom / 2,   // The middle row on the bottom.
      num_ff_bottom,  // The transmission gate row (~middle).
  };
  for (size_t i = 0; i < num_ff + 1; ++i) {
    // Skip transmission gate row.
    if (skip_rows.find(i) != skip_rows.end()) {
      continue;
    }
    geometry::Instance *decap = bank.InstantiateRight(
        i,
        absl::StrCat(right_decap_name, "_", i),
        right_decap_cell->layout());
  }

  std::string left_decap_name = "decap_left";
  Sky130Decap::Parameters left_decap_params = {
    .width_nm = 8 * 460   // TODO(aryap): Parameterise.
  };
  Sky130Decap left_decap_generator(left_decap_params, design_db_);
  Cell *left_decap_cell =
      left_decap_generator.GenerateIntoDatabase(left_decap_name);
  for (size_t i = 0; i < num_ff + 1; ++i) {
    if (i == num_ff_bottom) {
      // Skip transmission gate row. It needs its own.
      continue;
    }
    geometry::Instance *decap = bank.InstantiateLeft(
        i,
        absl::StrCat(left_decap_name, "_", i),
        left_decap_cell->layout());
  }

  std::string special_decap_name = "decap_special";
  Sky130Decap::Parameters special_decap_params = {
    .width_nm = 9 * 460,   // TODO(aryap): Parameterise.
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(mux_row_height))
  };
  Sky130Decap special_decap_generator(special_decap_params, design_db_);
  Cell *special_decap_cell =
      special_decap_generator.GenerateIntoDatabase(special_decap_name);
  geometry::Instance *decap = bank.InstantiateLeft(
      num_ff_bottom,
      absl::StrCat(special_decap_name, "_0"),
      special_decap_cell->layout());

  // Connect flip-flop outputs to transmission gates. Flip-flops store one bit
  // and output both the bit and its complement, conveniently. Per description
  // in header, start with left-most gates from the

  //      <------ poly pitch ---->
  //     v poly 1                 v poly 2
  //  ---+---->|<--->|<-----|<----+----->
  //     |  ^    ^       ^        |  ^ met1 via encap
  //     |  |    |     max offset |
  //     |  |    |     for next   |
  //     |  |    |     met1 encap |
  //     |  |    min met1 sep.    |
  //     |  met1 via encap
  //
  int64_t poly_pitch = db.ToInternalUnits(*parameters_.poly_pitch_nm);
  int64_t max_offset_from_first_poly_x =
      poly_pitch - (
          std::max(
              db.TypicalViaEncap("met1.drawing", "via1.drawing").length,
              db.TypicalViaEncap("met1.drawing", "mcon.drawing").length) +
          db.Rules("met1.drawing").min_separation
      );
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;

  // Check met2 spacing. We're putting four vertical lines down, the two outer
  // pairs are 1 met2 pitch apart, and the middle pair we just figured out:
  int64_t met2_x_span = met2_pitch  + (
      poly_pitch - 2 * max_offset_from_first_poly_x) + met2_pitch +
      db.TypicalViaEncap("met2.drawing", "via1.drawing").width;
  int64_t horizontal_gap = poly_pitch - (met2_x_span % poly_pitch);
  LOG_IF(WARNING, horizontal_gap < db.Rules("met2.drawing").min_separation)
      << "Vertical met2 are probably too close to those in adjacent "
      << "transmission gates";

  size_t c = 0;
  for (geometry::Instance *memory : bottom_memories) {
    size_t gate_number = 2 * c;

    geometry::Point p_tab_centre = stack->GetPointOrDie(
        absl::StrFormat("gate_%u_p_tab_centre", gate_number));
    geometry::Point n_tab_centre = stack->GetPointOrDie(
        absl::StrFormat("gate_%u_n_tab_centre", gate_number));

    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_QI = memory->GetFirstPortNamed("QI");

    int64_t vertical_x = p_tab_centre.x() + max_offset_from_first_poly_x;

    // The Q port is always the outer port. We know that from the layout of the
    // flip-flop, but we could also sort by their x positions if we had to.
    ConnectVertically(mem_Q->centre(),
                      p_tab_centre,
                      vertical_x - met2_pitch,
                      cell->layout());
    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x,
                      cell->layout());

    // Add a polycon (licon) and an li pad between the poly tab and the mcon via
    // that connects to the routes we just put down. To avoid the nearest poly
    // tab, these stick outward.
    AddPolyconAndLi(p_tab_centre, true, cell->layout());
    AddPolyconAndLi(n_tab_centre, false, cell->layout());

    ++c;
  }

  c = 0;
  for (geometry::Instance *memory : top_memories) {
    size_t gate_number = 2 * c + 1;

    geometry::Point p_tab_centre = stack->GetPointOrDie(
        absl::StrFormat("gate_%u_p_tab_centre", gate_number));
    geometry::Point n_tab_centre = stack->GetPointOrDie(
        absl::StrFormat("gate_%u_n_tab_centre", gate_number));

    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_QI = memory->GetFirstPortNamed("QI");

    int64_t vertical_x = p_tab_centre.x() - max_offset_from_first_poly_x;

    // The Q port is always the outer port. We know that from the layout of the
    // flip-flop, but we could also sort by their x positions if we had to.
    ConnectVertically(mem_Q->centre(),
                      p_tab_centre,
                      vertical_x,
                      cell->layout());
    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x + met2_pitch,
                      cell->layout());

    AddPolyconAndLi(p_tab_centre, true, cell->layout());
    AddPolyconAndLi(n_tab_centre, false, cell->layout());

    ++c;
  }

  return cell.release();
}

// We will determine the minimum vertical poly-to-boundary spacing such that any
// rectangle (wire) placed at that inset from the edge on any of the poly, li or
// met1 layers will be far enough away from shapes in the surrounding cell on
// the same layers. We only care how far shapes in the surrounding cell overflow
// its tiling bounds, since that is the incursion into what will be the mux
// cell. We also assume uniformity across the width of the cell.
int64_t Sky130InterconnectMux6::FigurePolyBoundarySeparationForMux(
    bfg::Layout *neighbour_layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  bfg::geometry::Rectangle tiling_bounds = neighbour_layout->GetTilingBounds();

  static const std::vector kCheckedLayers = {
    "poly.drawing", "met1.drawing", "li.drawing"
  };

  int64_t max_spacing = 0;

  for (const std::string &layer : kCheckedLayers) {
    bfg::geometry::Rectangle layer_bounds =
        neighbour_layout->GetBoundingBoxByNameOrDie(layer);

    // Minimum separation on this layer.
    int64_t layer_min_separation = db.Rules(layer).min_separation;

    // First check the top:
    int64_t overflow =
        layer_bounds.upper_right().y() - tiling_bounds.upper_right().y();
    int64_t required_separation = overflow + layer_min_separation;
    max_spacing = std::max(max_spacing, required_separation);

    // Then the bottom:
    overflow = tiling_bounds.lower_left().y() - layer_bounds.lower_left().y();
    required_separation = overflow + layer_min_separation;
    max_spacing = std::max(max_spacing, required_separation);
  }
  return max_spacing;
}

//  top  p1
//   +---+
//       |
//       |
//       |
//       |
//       |   bottom
//    p2 +----+
//       ^
//       vertical_x
//
void Sky130InterconnectMux6::ConnectVertically(
    const geometry::Point &top,
    const geometry::Point &bottom,
    int64_t vertical_x,
    bfg::Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  geometry::Point p1 = {vertical_x, top.y()};
  geometry::Point p2 = {vertical_x, bottom.y()};

  layout->MakeVia("mcon.drawing", top);
  layout->MakeAlternatingWire(
      {top, p1, p2, bottom}, "met1.drawing", "met2.drawing");
  layout->MakeVia("mcon.drawing", bottom);
}

void Sky130InterconnectMux6::AddPolyconAndLi(
    const geometry::Point tab_centre,
    bool bulges_up,
    bfg::Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  geometry::Rectangle *via = layout->MakeVia("polycon.drawing", tab_centre);

  int64_t max_overhang = std::max(
      db.Rules("polycon.drawing", "li.drawing").via_overhang,
      db.Rules("polycon.drawing", "li.drawing").via_overhang_wide);
  int64_t min_overhang = std::min(
      db.Rules("polycon.drawing", "li.drawing").via_overhang,
      db.Rules("polycon.drawing", "li.drawing").via_overhang_wide);

  int64_t width = 2 * max_overhang + via->Width();
  int64_t remaining_side = std::ceil(static_cast<double>(
      db.Rules("li.drawing").min_area) / static_cast<double>(width)) -
      via->Height();

  geometry::Rectangle li_pour = via->WithPadding(
      max_overhang,
      bulges_up ? remaining_side : min_overhang,
      max_overhang,
      bulges_up ? min_overhang : remaining_side);
  ScopedLayer scoped_layer(layout, "li.drawing");
  layout->AddRectangle(li_pour);
}

}   // namespace atoms
}   // namespace bfg
