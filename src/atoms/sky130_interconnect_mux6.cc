#include "sky130_interconnect_mux6.h"

#include "../utility.h"
#include "../modulo.h"
#include "../cell.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../row_guide.h"
#include "../geometry/compass.h"
#include "../geometry/rectangle.h"
#include "../geometry/instance.h"
#include "../geometry/port.h"
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
  if (power_ground_strap_width_nm) {
    pb->set_power_ground_strap_width_nm(*power_ground_strap_width_nm);
  } else {
    pb->clear_power_ground_strap_width_nm();
  }
}

void Sky130InterconnectMux6::Parameters::FromProto(
    const proto::parameters::Sky130InterconnectMux6 &pb) {
  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  } else {
    poly_pitch_nm.reset();
  }
  if (pb.has_power_ground_strap_width_nm()) {
    power_ground_strap_width_nm = pb.power_ground_strap_width_nm();
  } else {
    power_ground_strap_width_nm.reset();
  }
}

Sky130TransmissionGateStack::Parameters
Sky130InterconnectMux6::BuildTransmissionGateParams(
    geometry::Instance *vertical_neighbour) const {
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
      last_sequence.push_back(kMuxOutputName);
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
    .width_nm = Parameters::kHorizontalTilingUnitNm
  };
  atoms::Sky130Tap tap_generator(tap_params, design_db_);
  Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      PrefixCellName("interconnect_mux6_tap_template"));

  uint32_t num_ff = parameters_.num_inputs;
  uint32_t num_ff_top = num_ff / 2; 
  uint32_t num_ff_bottom = num_ff - num_ff_top;

  // We want the rows immediately below and above the centre row, where the
  // transmission gate mux is, to be rotated. Working backwards, whether or not
  // the first row is rotated or not is determined by whether the number of
  // memories below the centre row is even or odd. If it's odd, we must start
  // rotated, if not don't.
  bool rotate_first_row = num_ff_bottom % 2 != 0;
  MemoryBank bank = MemoryBank(cell->layout(),
                               design_db_,
                               tap_cell,
                               true,      // Rotate alternate rows.
                               rotate_first_row,
                               geometry::Compass::LEFT);

  std::vector<geometry::Instance*> bottom_memories;
  for (size_t i = 0; i < num_ff_bottom; i++) {
    std::string instance_name = PrefixCellName(
        absl::StrFormat("dfxtp_bottom_%d", i));
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
  std::string instance_name = PrefixCellName("gate_stack");
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
    std::string instance_name = PrefixCellName(
        absl::StrFormat("dfxtp_top_%d", i));
    std::string cell_name = absl::StrCat(instance_name, "_template");
    atoms::Sky130Dfxtp::Parameters params;
    atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
    geometry::Instance *instance = bank.InstantiateRight(
        i, instance_name, dfxtp_cell->layout());
    top_memories.push_back(instance);
  }

  // The output buffer goes at the end of the transmission gate stack.
  std::string output_buf_name = PrefixCellName("output_buf");
  int64_t mux_row_height =
      transmission_gate_stack_cell->layout()->GetTilingBounds().Height();
  Sky130Buf::Parameters output_buf_params = {
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(mux_row_height))
  };
  Sky130Buf output_buf_generator(output_buf_params, design_db_);
  Cell *output_buf_cell = output_buf_generator.GenerateIntoDatabase(
      absl::StrCat(output_buf_name, "_template"));
  geometry::Instance *output_buf_instance = bank.InstantiateRight(
      num_ff_bottom,
      output_buf_name,
      output_buf_cell->layout());

  // The input clock buffers go next to the middle flip flop on the top and
  // bottom side.
  std::string clk_buf_name = PrefixCellName("clk_buf");
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

  std::vector<geometry::Instance*> clk_bufs = {clk_buf_top, clk_buf_bottom};

  // Decaps!
  std::string right_decap_name = PrefixCellName("decap_right");
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

  std::string left_decap_name = PrefixCellName("decap_left");
  Sky130Decap::Parameters left_decap_params = {
    .width_nm = parameters_.vertical_routing_channel_width_nm.value_or(1380)
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

  std::string special_decap_name = PrefixCellName("decap_special");
  uint64_t special_decap_width_nm =
      parameters_.vertical_routing_channel_width_nm.value_or(1380) +
      tap_params.width_nm;
  Sky130Decap::Parameters special_decap_params = {
    .width_nm = special_decap_width_nm,
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(mux_row_height))
  };
  Sky130Decap special_decap_generator(special_decap_params, design_db_);
  Cell *special_decap_cell =
      special_decap_generator.GenerateIntoDatabase(special_decap_name);
  geometry::Instance *decap = bank.InstantiateLeft(
      num_ff_bottom,
      absl::StrCat(special_decap_name, "_0"),
      special_decap_cell->layout());

  int64_t tiling_bound_right_x = bank.Row(num_ff_bottom + 1).UpperRight().x();
  size_t middle_row_available_x =  tiling_bound_right_x -
      bank.Row(num_ff_bottom).UpperRight().x();
  if (middle_row_available_x >= db.ToInternalUnits(
        Sky130Decap::Parameters::kMinWidthNm)) {
    std::string optional_decap_name = PrefixCellName("decap_optional");
    Sky130Decap::Parameters optional_decap_params = {
      .width_nm = static_cast<uint64_t>(
          db.ToExternalUnits(middle_row_available_x)),
      .height_nm = static_cast<uint64_t>(db.ToExternalUnits(mux_row_height))
    };
    Sky130Decap optional_decap_generator(optional_decap_params, design_db_);
    Cell *optional_decap_cell =
        optional_decap_generator.GenerateIntoDatabase(
            absl::StrCat(optional_decap_name, "_template"));
    geometry::Instance *decap = bank.InstantiateRight(
        num_ff_bottom,
        optional_decap_name,
        optional_decap_cell->layout());
  }

  // The last step is to add the horizontal routing channel, which is either an
  // odd or even number of rows to make the total number of rows in the layout
  // even. We need an even number of rows to maintain the VPWR/VGND parity,
  // which in turn enables us to tile these muxes without any extra concern.
  // (VPWR has to match VPWR on the vertical neighbour, respectively VGND, etc).
  // There is always a fixed transmission gate mux row (the central one).
  //
  if (parameters_.num_inputs % 2 == 0) {
    atoms::Sky130Tap::Parameters channel_tap_params = {
      .height_nm =
          parameters_.horizontal_routing_channel_height_nm.value_or(2720),
      .width_nm = Parameters::kHorizontalTilingUnitNm
    };
    atoms::Sky130Tap channel_tap_generator(channel_tap_params, design_db_);
    Cell *channel_tap_cell = channel_tap_generator.GenerateIntoDatabase(
        PrefixCellName("channel_tap_template"));

    // This is
    //     1    for the centre transmission gate mux row
    //   + 1    for the new, additional row
    //   - 1    0-based indexing
    //  ----
    //     1
    size_t horizontal_channel_row = num_ff + 1;
    // Let's keep it for now (but have to account for it in the width split
    // below):
    bank.DisableTapInsertionOnRow(horizontal_channel_row);
    bank.EnableTapInsertionOnRow(horizontal_channel_row, channel_tap_cell);

    //
    // Need only one row.
    int64_t total_decap_width = bank.GetTilingBounds()->Width() -
        tap_cell->layout()->GetTilingBounds().Width();
    std::vector<int64_t> decap_widths = SplitIntoUnits(
        total_decap_width,
        db.ToInternalUnits(Sky130Decap::Parameters::kMaxWidthNm),
        db.ToInternalUnits(Parameters::kHorizontalTilingUnitNm));

    for (size_t i = 0; i < decap_widths.size(); ++i) {
      int64_t width = decap_widths[i];
      std::string name = PrefixCellName(
          absl::StrFormat("horizontal_channel_decap_%u", i));
      Sky130Decap::Parameters decap_params = {
        .width_nm = static_cast<uint64_t>(db.ToExternalUnits(width)),
        .height_nm =
            parameters_.horizontal_routing_channel_height_nm.value_or(2720)
      };
      Sky130Decap horizontal_channel_decap_generator(decap_params, design_db_);
      Cell *horizontal_decap_cell =
          horizontal_channel_decap_generator.GenerateIntoDatabase(
              absl::StrCat(name, "_template"));
      geometry::Instance *decap = bank.InstantiateRight(
          horizontal_channel_row,
          name,
          horizontal_decap_cell->layout());
    }
  }

  // TODO(aryap): Not sure if we need to do this, but we don't want the pin
  // layers to conflict and I don't know how LVS works yet:
  // layout->EraseLayerByName("met1.pin");
  // layout->EraseLayerByName("li.pin");

  // Draw all the wires!
  if (parameters_.num_inputs <= 7) {
    DrawRoutes(bank,
               top_memories,
               bottom_memories,
               clk_bufs,
               stack,
               output_buf_instance,
               cell->layout());
  } else {
    LOG(FATAL) << "You have to implement routes with something more "
               << "sophisticated, like the RoutingGrid";
  }

  cell->layout()->SetTilingBounds(*bank.GetTilingBounds());

  return cell.release();
}

void Sky130InterconnectMux6::DrawRoutes(
      const MemoryBank &bank,
      const std::vector<geometry::Instance*> &top_memories,
      const std::vector<geometry::Instance*> &bottom_memories,
      const std::vector<geometry::Instance*> &clk_bufs,
      geometry::Instance *stack,
      geometry::Instance *output_buffer,
      bfg::Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
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

  // Scan chain connections on the left side can be connected on metal 2, and
  // this should effectively only take up one channel width over the tap cells
  // and not detract from the routing channels in the left-most block.
  std::vector<geometry::Instance*> all_memories;
  all_memories.insert(
      all_memories.begin(), top_memories.begin(), top_memories.end());
  all_memories.insert(
      all_memories.begin(), bottom_memories.begin(), bottom_memories.end());

  // TODO(aryap): If the layout gets _any_ more complicated than this we will
  // need more sophisticated ways to reuse the control lines for the scan chain.
  // In fact they might already be too big (too much R & C)!
  //std::set<std::pair<geometry::Instance*, geometry::Instance*>>
  //    scan_chain_pairs;
  //for (auto it = all_memories.begin(); it != all_memories.end(); ++it) {
  //  geometry::Instance *current = *it;
  //  geometry::Instance *next = *(it + 1);
  //  scan_chain_pairs.insert({current, next});
  //}

  std::optional<int64_t> left_most_vertical_x;
  std::optional<int64_t> right_most_vertical_x;

  auto update_bounds_fn = [&](int64_t x) {
    UpdateMin(x, &left_most_vertical_x);
    UpdateMax(x, &right_most_vertical_x);
  };

  size_t c = 0;
  for (auto it = bottom_memories.rbegin(); it != bottom_memories.rend(); ++it) {
    geometry::Instance *memory = *it;
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
                      layout);

    update_bounds_fn(vertical_x - met2_pitch);

    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x,
                      layout);

    update_bounds_fn(vertical_x);

    // Add a polycon (licon) and an li pad between the poly tab and the mcon via
    // that connects to the routes we just put down. To avoid the nearest poly
    // tab, these stick outward.
    AddPolyconAndLi(p_tab_centre, true, layout);
    AddPolyconAndLi(n_tab_centre, false, layout);

    // We also use this opportunity to make scan-chain connections from memory Q
    // outputs to the D inputs on the next memory up. We only do this when the
    // output Q is on the left, since they will always line up with a single 

    ++c;
  }

  c = 0;
  for (auto it = top_memories.rbegin(); it != top_memories.rend(); ++it) {
    geometry::Instance *memory = *it;
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
                      layout);
    
    update_bounds_fn(vertical_x);

    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x + met2_pitch,
                      layout);

    update_bounds_fn(vertical_x + met2_pitch);

    AddPolyconAndLi(p_tab_centre, true, layout);
    AddPolyconAndLi(n_tab_centre, false, layout);

    ++c;
  }

  LOG_IF(FATAL, !left_most_vertical_x || !right_most_vertical_x)
      << "Expected vertical_x bounds to be set by this point - are there any "
      << "connections?";

  std::vector<int64_t> columns_right_x;
  for (int64_t x = *right_most_vertical_x + met2_pitch;
       x < bank.GetTilingBounds()->upper_right().x();
       x += met2_pitch) {
    columns_right_x.push_back(x);
  }

  std::vector<int64_t> columns_left_x;
  for (int64_t x = *left_most_vertical_x - met2_pitch;
       x > bank.GetTilingBounds()->lower_left().x();
       x -= met2_pitch) {
    columns_left_x.push_back(x);
  }

  // Allocate left columns so that they don't interfere with each other (or
  // cause problems for met1 connections below):
  constexpr size_t kScanChainLeftIndex = 0;
  constexpr size_t kInterconnectLeftStartIndex = 1;

  // Allocate right columns:
  constexpr size_t kScanChainRightIndex = 4;
  constexpr size_t kClockRightIndex = 1;
  constexpr size_t kClockIRightIndex = 3;
  constexpr size_t kInputClockRightIndex = 6;
  constexpr size_t kVPWRVGNDStartRightIndex = 7;

  // TODO(aryap): We can save a vertical met2 channel by squeezing the scan
  // chain connections on the right in (index 2), possible if the connection to
  // the input port does not occur directly across from the flip flop port but
  // rather through a met1 elbow:
  //
  //  met2 spine
  //     |
  //     +---+ met1 elbow jog
  //     |   |
  //     |   + flip flop D input
  //     |
  DrawScanChain(all_memories,
                bottom_memories.size() - 1,
                columns_left_x[kScanChainLeftIndex],
                columns_right_x[kScanChainRightIndex],
                layout);

  int64_t output_port_x = bank.GetTilingBounds()->upper_right().x();
  int64_t output_port_y = 0;
  DrawOutput(stack,
             output_buffer,
             &output_port_y,
             output_port_x,
             layout);

  DrawInputs(stack,
             output_port_y,
             columns_left_x[kInterconnectLeftStartIndex],
             layout);

  DrawClock(bank,
            top_memories,
            bottom_memories,
            clk_bufs,
            columns_right_x[kInputClockRightIndex],
            columns_right_x[kClockRightIndex],
            columns_right_x[kClockIRightIndex],
            layout);

  DrawPowerAndGround(bank,
                     columns_right_x[kVPWRVGNDStartRightIndex],
                     layout);
}

void Sky130InterconnectMux6::DrawPowerAndGround(
    const MemoryBank &bank,
    int64_t start_column_x,
    Layout *layout) const {
  //int64_t max_y = bank.GetTilingBounds()->upper_right().y();
  //int64_t min_y = bank.GetTilingBounds()->lower_left().y();

  // First figure out where the ground/power ports are:
  std::set<int64_t> power_y;
  std::set<int64_t> ground_y;
  for (const RowGuide &row : bank.rows()) {
    for (geometry::Instance *instance : row.instances()) {
      std::vector<geometry::Port*> power_ports;
      instance->GetInstancePorts("VPWR", &power_ports);
      for (geometry::Port *port : power_ports) {
        power_y.insert(port->centre().y());
      }
      std::vector<geometry::Port*> ground_ports;
      instance->GetInstancePorts("VGND", &ground_ports);
      for (geometry::Port *port : ground_ports) {
        ground_y.insert(port->centre().y());
      }
    }
  }
 
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &met2_rules = db.Rules("met2.drawing");
  const auto &met3_rules = db.Rules("met3.drawing");
  auto encap_rules = db.TypicalViaEncap("met2.drawing", "via1.drawing");

  int64_t strap_width = parameters_.power_ground_strap_width_nm ?
      db.ToInternalUnits(*parameters_.power_ground_strap_width_nm) :
      met2_rules.min_width;

  int64_t met2_boundary_left = start_column_x - met2_rules.min_width / 2;
  int64_t vpwr_x = met2_boundary_left + strap_width / 2;
  // Remember that min_pitch includes the width of via-encap bulges, whereas we
  // otherwise do not bother with that here.
  int64_t vgnd_x = vpwr_x + std::max(
      strap_width + met2_rules.min_separation,
      met2_rules.min_pitch);

  // Then assume that each power/ground rail extends from the left to the right
  // limit of the MemoryBank layout, draw rails over the top, and connect:
  auto draw_strap_fn = [&](
      const std::set<int64_t> y_values, int64_t x) -> geometry::Polygon* {
    std::vector<geometry::Point> points;
    // power_y should be sorted in y by virtue of being a set; min at front, max
    // at back.
    for (int64_t y : y_values) {
      points.emplace_back(x, y);
    }
    // TODO(aryap): This might be a nice general function for Layout too. It's
    // slightly different to the MakeVerticalSpineWithFingers, but not much:
    geometry::PolyLine power_line({points.front(), points.back()});
    power_line.SetWidth(strap_width);
    power_line.set_min_separation(met2_rules.min_separation);
    for (const geometry::Point &point : points) {
      power_line.InsertBulge(point, encap_rules.width, encap_rules.length);
      layout->MakeVia("via1.drawing", point);
    }
    {
      ScopedLayer sl(layout, "met2.drawing");
      return layout->AddPolyLine(power_line);
    }
  };

  geometry::Polygon *vpwr_strap = draw_strap_fn(power_y, vpwr_x);
  geometry::Polygon *vgnd_strap = draw_strap_fn(ground_y, vgnd_x);

  geometry::PortSet clock_ports;
  layout->GetPorts("CLK", &clock_ports);
  int64_t vpwr_port_y = 0;
  int64_t vgnd_port_y = 0;
  if (clock_ports.size() == 1) {
    int64_t clk_port_y = (*clock_ports.begin())->centre().y();
    vpwr_port_y = clk_port_y - 2 * met3_rules.min_pitch;
    vgnd_port_y = clk_port_y + 2 * met3_rules.min_pitch;
  } else {
    LOG(WARNING) << "Expected to find only one CLK port, but found "
                 << clock_ports.size();
  }


  layout->MakePin("VPWR", {vpwr_x, vpwr_port_y}, "met2.pin");
  layout->MakePin("VGND", {vgnd_x, vgnd_port_y}, "met2.pin");
}

void Sky130InterconnectMux6::DrawClock(
    const MemoryBank &bank,
    const std::vector<geometry::Instance*> &top_memories,
    const std::vector<geometry::Instance*> &bottom_memories,
    const std::vector<geometry::Instance*> &clk_bufs,
    int64_t input_clk_x,
    int64_t clk_x,
    int64_t clk_i_x,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // To connect the outputs of the clk bufs to the clk straps we use a met1 wire
  // that connects on an existing horizontal track. This will avoids vias and
  // bulges in what is a pretty congested spot.

  size_t b = 0;

  // We expect one buf per group of memories:
  for (const auto memories : {top_memories, bottom_memories}) {
    std::vector<geometry::Point> memory_CLK_centres;
    std::vector<geometry::Point> clk_spine_connections;
    std::vector<geometry::Point> memory_CLKI_centres;
    std::vector<geometry::Point> clk_i_spine_connections;

    for (geometry::Instance *memory : memories) {
      geometry::Port *port = memory->GetNearestPortNamed({clk_x, 0}, "CLK");
      LOG_IF(FATAL, !port) << "No port named CLK on memory " << memory->name();
      memory_CLK_centres.push_back(port->centre());
      clk_spine_connections.push_back({clk_x, port->centre().y()});

      port = memory->GetNearestPortNamed({clk_i_x, 0}, "CLKI");
      LOG_IF(FATAL, !port) << "No port named CLKI on memory " << memory->name();
      memory_CLKI_centres.push_back(port->centre());
      clk_i_spine_connections.push_back({clk_i_x, port->centre().y()});
    }

    // TODO(aryap): The clk_i connection is made directly on met1 (below)
    // because it's easy, and the clk connection is made across to the existing
    // spine, becaues it's easy.  This is bad because it delays clk further
    // relative to clk_i, and there's also one inverter's delay between them.
    geometry::Instance *buf = clk_bufs[b];
    geometry::Port *top_X = buf->GetNearestPortNamed(
        bank.GetTilingBounds()->upper_right(), "X");
    memory_CLK_centres.push_back(top_X->centre());

    // Manually create the via and encap from met1 to li.
    // TODO(aryap): See note in Layout::MakeVerticalSpineWithFingers. This
    // should be an automatic option of that function.
    layout->MakeVia("mcon.drawing", top_X->centre());
    {
      auto via_encap = db.TypicalViaEncap("met1.drawing", "mcon.drawing");
      ScopedLayer sl(layout, "met1.drawing");
      layout->AddRectangle(geometry::Rectangle::CentredAt(
          top_X->centre(), via_encap.length, via_encap.width));
    }

    layout->MakeVerticalSpineWithFingers(
        "met2.drawing",
        "met1.drawing",
        "clk_internal",
        memory_CLK_centres,
        clk_x,
        db.Rules("met2.drawing").min_width,
        layout);

    layout->MakeVerticalSpineWithFingers(
        "met2.drawing",
        "met1.drawing",
        "clk_i_internal",
        memory_CLKI_centres,
        clk_i_x,
        db.Rules("met2.drawing").min_width,
        layout);
  
    // TODO(aryap): This sucks. I want the highest port from a collection. With
    // the current API this is cumbersome, so instead I'm using a fake point at
    // the top-right of the layout and ordering by proximity. Eurgh.
    geometry::Port *top_P = buf->GetNearestPortNamed(
        bank.GetTilingBounds()->upper_right(), "P");

    // Connect buf output to clk_internal:
    geometry::Point on_spine = geometry::Point::ClosestTo(
        clk_i_spine_connections, top_P->centre());

    //          + top P port on clk buf
    //          |
    //   +------+
    //   ^
    // spine connection
    layout->MakeWire({top_P->centre(),
                      {top_P->centre().x(), on_spine.y()},
                      on_spine},
                     "met1.drawing",
                     "li.drawing",
                     "met2.drawing",
                     true,
                     false);
    layout->MakeVia("mcon.drawing", top_P->centre());

    ++b;
  }

  std::vector<geometry::Point> buf_A_centres;
  for (geometry::Instance *buf : clk_bufs) {
    geometry::Port *port = buf->GetNearestPortNamed({input_clk_x, 0}, "A");
    LOG_IF(FATAL, !port) << "No port named A on buf " << buf->name();
    buf_A_centres.push_back(port->centre());
  }

  layout->MakeVerticalSpineWithFingers(
        "met2.drawing",
        "met1.drawing",
        "CLK",            // TODO(aryap): Parameterise.
        buf_A_centres,
        input_clk_x,
        db.Rules("met2.drawing").min_width,
        layout);

  // Lastly, we want a pad around a via for met3 to be the CLK port for this
  // mux. Put it right in the middle, between the other two connection points.
  // TODO(aryap): This is where it would again be nice to have a more flexible
  // "MakeVerticalSpineWithFingers" function.
  geometry::Point clock_port_centre = {
      input_clk_x, (buf_A_centres.front().y() + buf_A_centres.back().y()) / 2};

  // Assume met2 is vertical, as we have everywhere. Note that we're after met2
  // encap of via2, not via1, this time:
  auto encap_rules = db.TypicalViaEncap("met2.drawing", "via2.drawing");
  {
    ScopedLayer sl(layout, "met2.drawing");
    layout->AddRectangle(
        geometry::Rectangle::CentredAt(
            clock_port_centre, encap_rules.width, encap_rules.length));
  }
  layout->MakePin("CLK", clock_port_centre, "met2.pin");
}

void Sky130InterconnectMux6::DrawOutput(
    geometry::Instance *stack,
    geometry::Instance *output_buffer,
    int64_t *output_port_y,
    int64_t output_port_x,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // TODO(aryap): This bit sucks. I'm not sure if the
  // Sky130TransmissionGateStack should be in charge of creating and
  // distributing ports (in which case it should know about special ports like
  // "Z", which we want to go through the middle or something), or if it's up
  // to this client class to distribute the wires over the ports. Having the
  // ports does at very least associate the x coordinates needed with their
  // nets. Likely we need to generate the input nets and output net in advance
  // of what we currently are doing.
  //
  // Connect the transmission gate mux outputs to the buf. Use the default
  // position of the ports created by the transmission gate mux.
  std::vector<geometry::Port*> outputs;
  stack->GetInstancePorts(kMuxOutputName, &outputs);

  std::vector<geometry::Point> wire_points;
  std::vector<Layout::ViaToSomeLayer> connection_points;
  for (geometry::Port *port : outputs) {
    connection_points.push_back({
        .centre = port->centre(),
        .layer_name = "li.drawing"
    });
    wire_points.push_back(port->centre());
  }

  geometry::Port *buf_A = output_buffer->GetFirstPortNamed("A");

  wire_points.push_back({buf_A->centre().x(), wire_points.back().y()});

  wire_points.push_back(buf_A->centre());
  connection_points.push_back({
      .centre = buf_A->centre(),
      .layer_name = "li.drawing"
  });
  layout->MakeWire(wire_points, "met1.drawing", connection_points);

  *output_port_y = wire_points.front().y();

  // Connect the buff output to the edge of the design:
  geometry::Port *buf_X = output_buffer->GetFirstPortNamed("X");

  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch;
  int64_t vertical_x = buf_X->centre().x() + met1_pitch;

  std::vector<geometry::Point> output_wire = {
      buf_X->centre(),
      {vertical_x, buf_X->centre().y()},
      {vertical_x, *output_port_y},
      geometry::Point {output_port_x, *output_port_y}
  };

  layout->MakeWire(output_wire,
                   "met1.drawing",  // Wire layer.
                   "li.drawing",    // Start layer.
                   std::nullopt);
  layout->MakePin(kMuxOutputName, output_wire.back(), "met1.pin");
}

void Sky130InterconnectMux6::DrawInputs(
    geometry::Instance *stack,
    int64_t output_port_y,
    int64_t vertical_x_left,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch;
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;

  // Compute the horizontal channels we have:
  int64_t horizontal_y_max = stack->GetPointOrDie(
      absl::StrFormat("net_%s_via_top_0", kMuxOutputName)).y();
  int64_t horizontal_y_min = stack->GetPointOrDie(
      absl::StrFormat("net_%s_via_bottom_0", kMuxOutputName)).y();

  int64_t num_tracks = (horizontal_y_max - horizontal_y_min) / met1_pitch;

  LOG_IF(WARNING, num_tracks < parameters_.num_inputs)
      << "The number of tracks available (" << num_tracks
      << ") is less that the number of inputs (" << parameters_.num_inputs
      << ")";

  std::vector<int64_t> input_channels_x;
  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    int64_t channel_x = vertical_x_left - i * met2_pitch;
    input_channels_x.push_back(channel_x);
  }

  // Connect the inputs.
  bool up = false;
  size_t j = 1;
  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    int64_t y_offset = j * met1_pitch;
    int64_t y = output_port_y + (up ? y_offset : -y_offset);
    if ((i + 1) % 2 == 0) {
      j++;
    }
    up = !up;

    std::string input_name = absl::StrFormat("X%d", i);
    int64_t x = stack->GetFirstPortNamed(input_name)->centre().x();

    geometry::Point start = {
        input_channels_x[input_channels_x.size() - 1 - i], y};
    geometry::Point end = {x, y};

    layout->MakeWire(
        {start, end},
        "met1.drawing",
        "met2.drawing",
        "li.drawing",
        true);    // Pad only at the start.

    ScopedLayer sl(layout, "met1.pin");
    layout->MakePin(input_name, start, "met1.pin");
  }
}

void Sky130InterconnectMux6::DrawScanChain(
    const std::vector<geometry::Instance*> &all_memories,
    int64_t num_ff_bottom,
    int64_t vertical_x_left,
    int64_t vertical_x_right,
    bfg::Layout *layout) const {
  size_t i = 0;
  for (auto it = all_memories.begin(); it < all_memories.end() - 1; ++it) {
    geometry::Instance *memory = *it;
    geometry::Instance *next = *(it + 1);

    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_D = memory->GetFirstPortNamed("D");

    geometry::Port *next_D = next->GetFirstPortNamed("D");

    // We check to see which way around the FF is. If input is left of output,
    // it's oriented normally, and we connect using a metal bar on the left of
    // the previous connections. If it's the other way we use a metal bar on
    // the right. This test means we don't have to rely on a particular
    // orientation pattern when the memories are laid out.
    //
    //
    // FIXME(aryap): A more robust way to do the scan chain (without doing
    // anything intelligent) will be to dedicate a vertical channel on the left
    // and right sides of the flip flops that avoids other routes we are
    // planning, like the control wires. This limits vertical channel usage to 2
    // and gives us more room for the clk, vdd, vss met2 lines.
    //
    // Ok no problem we just have to push the right-most vertical out more to
    // avoid using met1 too close to other met1!
    int64_t  vertical_x = mem_Q->centre().IsStrictlyLeftOf(mem_D->centre()) &&
        i != num_ff_bottom ?  vertical_x_left : vertical_x_right;
    //if (i != num_ff_bottom) {
    //  vertical_x = mem_Q->centre().IsStrictlyLeftOf(mem_D->centre()) ?
    //      vertical_x_left : std::max(vertical_x_right, mem_Q->centre().x());
    //} else {
    //  // Except to dodge the routing aligned to the left of the row where the
    //  // transmission gate mux lives.
    //  vertical_x = next_D->centre().x();
    //}

    ConnectVertically(mem_Q->centre(),
                      next_D->centre(),
                      vertical_x,
                      layout);
    ++i;
  }

  layout->MakePin("SCAN_IN",
                  all_memories.front()->GetFirstPortNamed("D")->centre(),
                  "li.pin");
  layout->MakePin("SCAN_OUT",
                  all_memories.back()->GetFirstPortNamed("D")->centre(),
                  "li.pin");
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

std::vector<int64_t> Sky130InterconnectMux6::SplitIntoUnits(
    int64_t length, int64_t max, int64_t unit) {
  // Rely on truncating (floor) behaviour.
  int64_t real_max = (max / unit) * unit;

  std::vector<int64_t> lengths;

  // Stripmining!
  int64_t unallocated = length;
  while (unallocated >= unit) {
    int64_t remainder = unallocated - real_max;
    if (remainder >= 0) {
      lengths.push_back(real_max);
    } else {
      // Again we rely on truncating (floor) behaviour:
      lengths.push_back((unallocated / unit) * unit);
    }
    unallocated = remainder;
  }
  return lengths;
}

}   // namespace atoms
}   // namespace bfg
