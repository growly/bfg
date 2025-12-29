#include "sky130_interconnect_mux1.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../cell.h"
#include "../geometry/compass.h"
#include "../geometry/instance.h"
#include "../geometry/port.h"
#include "../geometry/rectangle.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../poly_line_inflator.h"
#include "../row_guide.h"
#include "../utility.h"
#include "sky130_buf.h"
#include "sky130_tap.h"
#include "sky130_transmission_gate_stack.h"
#include "sky130_dfxtp.h"
#include "sky130_decap.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_cat.h>

namespace bfg {
namespace atoms {

void Sky130InterconnectMux1::Parameters::ToProto(
    proto::parameters::Sky130InterconnectMux1 *pb) const {
  pb->set_num_inputs(num_inputs);
  pb->set_num_outputs(num_outputs);
  if (poly_pitch_nm) {
    pb->set_poly_pitch_nm(*poly_pitch_nm);
  } else {
    pb->clear_poly_pitch_nm();
  }
  if (vertical_pitch_nm) {
    pb->set_vertical_pitch_nm(*vertical_pitch_nm);
  } else {
    pb->clear_vertical_pitch_nm();
  }
  if (vertical_offset_nm) {
    pb->set_vertical_offset_nm(*vertical_offset_nm);
  } else {
    pb->clear_vertical_offset_nm();
  }
  if (horizontal_pitch_nm) {
    pb->set_horizontal_pitch_nm(*horizontal_pitch_nm);
  } else {
    pb->clear_horizontal_pitch_nm();
  }
  if (power_ground_strap_width_nm) {
    pb->set_power_ground_strap_width_nm(*power_ground_strap_width_nm);
  } else {
    pb->clear_power_ground_strap_width_nm();
  }
}

// TODO(aryap): Empty fields in the proto should yield default values of fields
// in the Parameters struct. We can't enforce that here, so we hope that the
// existing values are the default values and leave them unchanged if they don't
// appear in the input file. Make this consistent across implementations of
// Parameters structs.
void Sky130InterconnectMux1::Parameters::FromProto(
    const proto::parameters::Sky130InterconnectMux1 &pb) {
  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  }
  if (pb.has_vertical_pitch_nm()) {
    vertical_pitch_nm = pb.vertical_pitch_nm();
  }
  if (pb.has_vertical_offset_nm()) {
    vertical_offset_nm = pb.vertical_offset_nm();
  }
  if (pb.has_horizontal_pitch_nm()) {
    horizontal_pitch_nm = pb.horizontal_pitch_nm();
  }
  if (pb.has_power_ground_strap_width_nm()) {
    power_ground_strap_width_nm = pb.power_ground_strap_width_nm();
  }
  if (pb.has_num_inputs()) {
    num_inputs = pb.num_inputs();
  }
  if (pb.has_num_outputs()) {
    num_outputs = pb.num_outputs();
  }
}

std::vector<std::vector<std::string>>
Sky130InterconnectMux1::BuildNetSequences() const {
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

Sky130TransmissionGateStack::Parameters
Sky130InterconnectMux1::BuildTransmissionGateParams(
    geometry::Instance *vertical_neighbour) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  Sky130TransmissionGateStack::Parameters params = {
    .sequences = {},
    .min_poly_boundary_separation_nm = FigurePolyBoundarySeparationForMux(
        vertical_neighbour->template_layout())
  };
  ConfigureSky130Parameters(&params);

  uint32_t needed_tracks = parameters_.num_inputs;
  if (parameters_.vertical_pitch_nm) {
    params.min_height_nm = (needed_tracks + 3) * *parameters_.vertical_pitch_nm;
  }
  // FIXME(aryap): This doesn't do waht I expect?!
  if (parameters_.min_transmission_gate_stack_height_nm) {
    params.min_height_nm = std::max(
        *parameters_.min_transmission_gate_stack_height_nm,
        *params.min_height_nm);
  }
  params.poly_contact_vertical_pitch_nm = parameters_.vertical_pitch_nm;
  params.poly_contact_vertical_offset_nm = parameters_.vertical_offset_nm;
  params.input_vertical_pitch_nm = parameters_.vertical_pitch_nm;
  params.input_vertical_offset_nm = parameters_.vertical_offset_nm;
  params.expand_wells_to_vertical_bounds = true;
  params.expand_wells_to_horizontal_bounds = true;
  params.poly_pitch_nm = parameters_.poly_pitch_nm;

  // Build the sequences of nets that dictate the arrangement of the
  // transmission gate stack, e.g. for 1 output:
  // {
  //   {"X0", "S0", "Z", "S1", "X1"},
  //   {"X2", "S2", "Z", "S3", "X3"},
  //   {"X4", "S4", "Z", "S5", "X5"},
  //   {"X6", "S6", "Z"}                // For the 7th input.
  // }
  //
  // and for 5 inputs, 2 outputs (with simple control names):
  // {
  //   {"X0", "S0", "Z0", "S1", "X1", "S2", "Z1",
  //        "S3", "X2", "S4", "Z0", "S5", "X3", "S6", "Z1", "S7", "X4"}
  // }
  // ... where X0, X4 are unique inputs, and X1, X2, X3 are shared.
  //
  // With more useful control names, indicating which input is being connected
  // to which output:
  // {
  //   {"X0", "S0_0", "Z0", "S1_0", "X1", "S1_1", "Z1", "S2_1",
  //        "X2", "S2_0", "Z0", "S3_0", "X3", "S3_1", "Z1", "S4_1", "X4"}
  // }
  // Control i connect input
  //    int((i + 1) / 2)
  // to output
  //    (int((i / 2) % 2) + int((i % 2) / 2)
  // (Trust me bro.)
  params.sequences = BuildNetSequences();
  params.top_metal_channel_net = StackTopLiChannelNet();
  params.bottom_metal_channel_net = StackBottomLiChannelNet();
  return params;
}

void Sky130InterconnectMux1::AddOutputBuffers(
    size_t row,
    int64_t row_height,
    MemoryBank *bank,
    std::vector<geometry::Instance*> *output_bufs) {
  uint32_t num_outputs = NumOutputs();
  for (uint32_t i = 0; i < num_outputs; ++i) {
    output_bufs->push_back(
        AddOutputBufferRight(
            absl::StrFormat("%d", i), row_height, row, bank));
  }
}

std::vector<geometry::Instance*> Sky130InterconnectMux1::AddMemoriesVertically(
    size_t first_row,
    uint32_t num_rows,
    uint32_t columns,
    MemoryBank *bank,
    bool alternate_scan) {
  std::vector<geometry::Instance*> memories(num_rows * columns, nullptr);
  for (size_t i = first_row; i < first_row + num_rows; ++i) {
    for (size_t j = 0; j < columns; ++j) {
      std::string cell_name = PrefixCellName(
          absl::StrFormat("dfxtp_%d", i * columns + j));
      std::string instance_name = absl::StrCat(cell_name, "_i");
      atoms::Sky130Dfxtp::Parameters params;
      ConfigureSky130Parameters(&params);
      params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
      params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
      atoms::Sky130Dfxtp dfxtp_generator(params, design_db_);
      Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(cell_name);
      geometry::Instance *layout_instance = bank->InstantiateRight(
          i, instance_name, dfxtp_cell);
      // Append in scan order.
      size_t k = i % 2 == (alternate_scan ? 1 : 0) ?
          (i - first_row + 1)  * columns - (j + 1) :
          (i - first_row) * columns + j;
      memories[k] = layout_instance;
    }
  }
  return memories;
}

geometry::Instance *Sky130InterconnectMux1::AddClockBufferRight(
    const std::string &suffix, size_t row, MemoryBank *bank) {
  // The input clock buffers go next to the middle flip flop on the top and
  // bottom side.
  std::string clk_buf_name = PrefixCellName("clk_buf");
  Cell *clk_buf_cell = design_db_->FindCell("", clk_buf_name);
  if (clk_buf_cell == nullptr) {
    Sky130Buf::Parameters clk_buf_params = {};
    ConfigureSky130Parameters(&clk_buf_params);
    clk_buf_params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
    clk_buf_params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
    Sky130Buf clk_buf_generator(clk_buf_params, design_db_);
    clk_buf_cell = clk_buf_generator.GenerateIntoDatabase(clk_buf_name);
  }
  geometry::Instance *instance = bank->InstantiateRight(
      row, absl::StrCat(clk_buf_name, "_", suffix), clk_buf_cell);
  return instance;
}

Cell *Sky130InterconnectMux1::MakeDecapCell(
    uint32_t width_nm, uint32_t height_nm) {
  // TODO(aryap): This is a cheap way of hashing based on parameters so that we
  // don't re-generate the same cell many times. It would be nice if the
  // database did this for us! Perhaps silently as part of the contract for
  // GenerateIntoDatabase()?
  // TODO(aryap): It would also be nice if we could ask the design database for
  // a new instance of any cell that would be automatically, uniquely named.
  std::string name = PrefixCellName(
      absl::StrCat("decap_", width_nm, "x", height_nm));
  Cell *cell = design_db_->FindCell("", name);
  if (cell == nullptr) {
    Sky130Decap::Parameters params = {
      .width_nm = width_nm,
      .height_nm = height_nm,
    };
    ConfigureSky130Parameters(&params);
    params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
    params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
    Sky130Decap decap_generator(params, design_db_);
    cell = decap_generator.GenerateIntoDatabase(name);
  }
  return cell;
}

geometry::Instance *Sky130InterconnectMux1::AddOutputBufferRight(
    const std::string &suffix, uint32_t height, size_t row, MemoryBank *bank) {
  std::string output_buf_name = PrefixCellName("output_buf");
  std::string cell_name = absl::StrCat(output_buf_name, "_template");
  Cell *output_buf_cell = design_db_->FindCell("", cell_name);
  if (output_buf_cell == nullptr) {
    Sky130Buf::Parameters output_buf_params = {
      .height_nm = static_cast<uint64_t>(
          design_db_->physical_db().ToExternalUnits(height))
    };
    ConfigureSky130Parameters(&output_buf_params);
    output_buf_params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
    output_buf_params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
    Sky130Buf output_buf_generator(output_buf_params, design_db_);
    output_buf_cell = output_buf_generator.GenerateIntoDatabase(cell_name);
  }
  return bank->InstantiateRight(
      row,
      suffix == "" ?
          output_buf_name : absl::StrCat(output_buf_name, "_", suffix),
      output_buf_cell);
}

geometry::Instance *Sky130InterconnectMux1::AddTransmissionGateStackRight(
    geometry::Instance *vertical_neighbour, size_t row, MemoryBank *bank) {
  Sky130TransmissionGateStack::Parameters transmission_gate_mux_params =
      BuildTransmissionGateParams(vertical_neighbour);
  Sky130TransmissionGateStack generator = Sky130TransmissionGateStack(
      transmission_gate_mux_params, design_db_);
  std::string instance_name = PrefixCellName("gate_stack");
  std::string template_name = absl::StrCat(instance_name, "_template");
  Cell *transmission_gate_stack_cell =
      generator.GenerateIntoDatabase(template_name);
  return bank->InstantiateRight(
      row, instance_name, transmission_gate_stack_cell);
}

bfg::Cell *Sky130InterconnectMux1::Generate() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_interconnect_mux6": name_));

  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  atoms::Sky130Tap::Parameters tap_params = {
    .height_nm = static_cast<uint64_t>(db.ToExternalUnits(2720U)),
    .width_nm = Parameters::kHorizontalTilingUnitNm
  };
  tap_params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
  tap_params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
  atoms::Sky130Tap tap_generator(tap_params, design_db_);
  Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      PrefixCellName("interconnect_mux6_tap_template"));

  uint32_t num_ff = NumMemories();
  uint32_t num_ff_columns = NumMemoryColumns();

  uint32_t num_ff_rows = std::ceil(
      static_cast<double>(num_ff) / static_cast<double>(num_ff_columns));
  uint32_t num_ff_rows_top = num_ff_rows / 2; 
  uint32_t num_ff_rows_bottom = num_ff_rows - num_ff_rows_top;

  // We want the rows immediately below and above the centre row, where the
  // transmission gate mux is, to be rotated. Working backwards, whether or not
  // the first row is rotated or not is determined by whether the number of
  // memories below the centre row is even or odd. If it's odd, we must start
  // rotated, if not don't.
  bool rotate_first_row = num_ff_rows_bottom % 2 != 0;
  MemoryBank bank = MemoryBank(cell->layout(),
                               cell->circuit(),
                               design_db_,
                               tap_cell,
                               true,      // Rotate alternate rows.
                               rotate_first_row,
                               geometry::Compass::LEFT);

  // Add bottom memories:
  std::vector<geometry::Instance*> bottom_memories = AddMemoriesVertically(
      0,
      num_ff_rows_bottom,
      num_ff_columns,
      &bank);

  //atoms::Sky130Tap::Parameters tap_params = {
  //  .height_nm = 2720,
  //  .width_nm = 460
  //};
  //atoms::Sky130Tap tap_generator(tap_params, design_db_);
  //Cell *tap_cell = tap_generator.GenerateIntoDatabase(
  //    "interconnect_mux6_tap_template");
  //
  // Width of a tap, above.
  //bank.Row(num_ff_rows_bottom).AddBlankSpaceAndInsertFront(460);
  // Disable the tap cell on this row.
  bank.Row(num_ff_rows_bottom).clear_tap_cell();
  geometry::Instance *stack_layout = AddTransmissionGateStackRight(
      bottom_memories.back(), num_ff_rows_bottom, &bank);

  std::vector<geometry::Instance*> top_memories = AddMemoriesVertically(
      num_ff_rows_bottom + 1,
      num_ff_rows_top,
      num_ff_columns,
      &bank);

  // TODO(aryap): Document elsewhere:
  // right i remember now. to decode an address up to 6 needs 3 bits, so you pay
  // the same price decoding 5-8; takes fewer memories (= rows) for 6 than 8.
  // possibly a win at 8 inputs (= 3 memories + decoder for each). decoder is a
  // NAND3+inverter for each control line?
  //
  // also i think this layout is going to be sufficiently different that we need
  // a new class. perhaps a derived class so we can reuse the decap, buffer, etc
  // insertion. but the routing will need to be vastly different.

  int64_t mux_row_height =
      stack_layout->template_layout()->GetTilingBounds().Height();

  // The output buffer goes at the end of the transmission gate stack.
  std::vector<geometry::Instance*> output_bufs;
  AddOutputBuffers(num_ff_rows_bottom, mux_row_height, &bank, &output_bufs);

  // The input clock buffers go next to the middle flip flop on the top and
  // bottom side.
  geometry::Instance *clk_buf_top_layout = AddClockBufferRight(
      "top",
      // The middle row on top.
      num_ff_rows_bottom + 1 + (num_ff_rows_top / 2),
      &bank);
  geometry::Instance *clk_buf_bottom_layout = AddClockBufferRight(
      "bottom",
      num_ff_rows_bottom / 2,   // The middle row on the bottom.
      &bank);

  std::vector<geometry::Instance*> clk_bufs = {
      clk_buf_top_layout, clk_buf_bottom_layout
  };

  // Decaps!
  Cell *right_decap_cell = MakeDecapCell(1380U, 2720U);
  std::set<size_t> skip_rows = {
      // The middle row on top.
      num_ff_rows_bottom + 1 + (num_ff_rows_top / 2),
      num_ff_rows_bottom / 2,   // The middle row on the bottom.
      num_ff_rows_bottom,  // The transmission gate row (~middle).
  };
  for (size_t i = 0; i < num_ff_rows + 1; ++i) {
    // Skip transmission gate row.
    if (skip_rows.find(i) != skip_rows.end()) {
      continue;
    }
    geometry::Instance *decap_layout = bank.InstantiateRight(
        i,
        absl::StrCat(right_decap_cell->name(), "_i", i),
        right_decap_cell);
  }

  std::string left_decap_name = PrefixCellName("decap_left");

  // Size the routing channel to make the overall mux meet the pitch
  // requirement:
  uint64_t fixed_row_width = bank.Row(num_ff_rows_bottom - 1).Width();
  uint64_t horizontal_pitch_nm = parameters_.horizontal_pitch_nm.value_or(
      Parameters::kHorizontalTilingUnitNm);
  uint64_t vertical_channel_width_nm = Utility::NextMultiple(
      parameters_.vertical_routing_channel_width_nm.value_or(1380) +
          fixed_row_width,
      horizontal_pitch_nm) - fixed_row_width;

  Cell *left_decap_cell =
      MakeDecapCell(vertical_channel_width_nm, 2720U);
  for (size_t i = 0; i < num_ff_rows + 1; ++i) {
    if (i == num_ff_rows_bottom) {
      // Skip transmission gate row. It needs its own.
      continue;
    }
    geometry::Instance *decap = bank.InstantiateLeft(
        i,
        absl::StrCat(left_decap_cell->name(), "_i", i),
        left_decap_cell);
  }

  uint64_t special_decap_width_nm =
      vertical_channel_width_nm + tap_params.width_nm;
  Cell *special_decap_cell = MakeDecapCell(
      special_decap_width_nm, 
      static_cast<uint64_t>(db.ToExternalUnits(mux_row_height)));
  geometry::Instance *decap = bank.InstantiateLeft(
      num_ff_rows_bottom,
      absl::StrCat(cell->name(), "_i0"),
      special_decap_cell);

  int64_t tiling_bound_right_x =
      bank.Row(num_ff_rows_bottom + 1).UpperRight().x();
  int64_t middle_row_available_x =  tiling_bound_right_x -
      bank.Row(num_ff_rows_bottom).UpperRight().x();
  if (middle_row_available_x >= db.ToInternalUnits(
        Sky130Decap::Parameters::kMinWidthNm)) {
    Cell *optional_decap_cell = MakeDecapCell(
        std::min(
            static_cast<uint64_t>(db.ToExternalUnits(middle_row_available_x)),
            Sky130Decap::Parameters::kMaxWidthNm),
        static_cast<uint64_t>(db.ToExternalUnits(mux_row_height)));
    geometry::Instance *decap = bank.InstantiateRight(
        num_ff_rows_bottom,
        absl::StrCat(optional_decap_cell->name(), "_i0"),
        optional_decap_cell);
  }

  // The last step is to add the horizontal routing channel, which is either an
  // odd or even number of rows to make the total number of rows in the layout
  // even. We need an even number of rows to maintain the VPWR/VGND parity,
  // which in turn enables us to tile these muxes without any extra concern.
  // (VPWR has to match VPWR on the vertical neighbour, respectively VGND, etc).
  // There is always a fixed transmission gate mux row (the central one).
  //
  // This is actually an option: if parity flips, we can tile this module by
  // rotating the tiles above and below, as we do for standard cells.
  if ((parameters_.num_inputs * NumOutputs()) % 2 == 0) {
    // Because the routing channel is an unusual height, we need to create a
    // special tap cell for it:
    atoms::Sky130Tap::Parameters channel_tap_params = {
      .height_nm =
          parameters_.horizontal_routing_channel_height_nm.value_or(2720),
      .width_nm = Utility::NextMultiple(
          Parameters::kHorizontalTilingUnitNm,
          horizontal_pitch_nm)
    };
    ConfigureSky130Parameters(&channel_tap_params);
    channel_tap_params.draw_vpwr_vias = !parameters_.redraw_rail_vias;
    channel_tap_params.draw_vgnd_vias = !parameters_.redraw_rail_vias;
    atoms::Sky130Tap channel_tap_generator(channel_tap_params, design_db_);
    Cell *channel_tap_cell = channel_tap_generator.GenerateIntoDatabase(
        PrefixCellName("channel_tap_template"));

    // This is
    //     1    for the centre transmission gate mux row
    //   + 1    for the new, additional row
    //   - 1    0-based indexing
    //  ----
    //     1
    size_t horizontal_channel_row = num_ff_rows + 1;

    bank.DisableTapInsertionOnRow(horizontal_channel_row);
    bank.EnableTapInsertionOnRow(horizontal_channel_row, channel_tap_cell);

    RowGuide &row = bank.Row(horizontal_channel_row);

    // In the regime where there is only one column of memories, we expect there
    // to be at most 1 tap, and so we can account for the total available width
    // up front. But if there are two columns or more than 1 tap for any other
    // reason, this becomes difficult. The most general approach is therefore to
    // 'strip mine' the available width, in unit-sized chunks:

    int64_t remaining_channel_width = row.AvailableRightSpanUpTo(
        bank.GetTilingBounds()->Width());
    size_t count = 0;
    while (remaining_channel_width > 0) {
      int64_t decap_width = Utility::LastMultiple(
          std::min(remaining_channel_width,
                   db.ToInternalUnits(Sky130Decap::Parameters::kMaxWidthNm)),
          db.ToInternalUnits(horizontal_pitch_nm));

      if (decap_width < db.ToInternalUnits(
              Sky130Decap::Parameters::kMinWidthNm)) {
          break;
      }

      Cell *horizontal_decap_cell =
          MakeDecapCell(
              static_cast<uint64_t>(db.ToExternalUnits(decap_width)),
              parameters_.horizontal_routing_channel_height_nm.value_or(2720));
      geometry::Instance *decap = bank.InstantiateRight(
          horizontal_channel_row,
          absl::StrCat(horizontal_decap_cell->name(), "_i", count),
          horizontal_decap_cell);
      count++;

      remaining_channel_width = row.AvailableRightSpanUpTo(
          bank.GetTilingBounds()->Width());
    }
  }

  geometry::Rectangle tiling_bounds = *bank.GetTilingBounds();
  if (parameters_.redraw_rail_vias) {
    const auto &rows = bank.rows();
    std::vector<int64_t> y_values(rows.size() + 1);
    for (size_t i = 0; i < rows.size(); ++i) {
      y_values[i] = rows[i].GetTilingBounds()->lower_left().y();
    }
    y_values[y_values.size() - 1] =
        rows.back().GetTilingBounds()->upper_right().y();

    const int64_t mcon_pitch = db.ToInternalUnits(
        Parameters::kHorizontalTilingUnitNm);

    int64_t x_min = tiling_bounds.lower_left().x();
    int64_t x_max = tiling_bounds.upper_right().x();
    for (int64_t y : y_values) {
      // Guess that the power rails are centred over the top and bottom edges of
      // the tiling bounds:
      cell->layout()->StampVias(
          "mcon.drawing", {x_min, y}, {x_max, y}, mcon_pitch);
    }
  }

  // TODO(aryap): Not sure if we need to do this, but we don't want the pin
  // layers to conflict and I don't know how LVS works yet:
  // layout->EraseLayerByName("met1.pin");
  // layout->EraseLayerByName("li.pin");

  // Draw all the wires!
  LOG_IF(WARNING, parameters_.num_inputs > 7)
      << "More than 7 inputs is known to not generate correctly. "
      << "Try using RoutingGrid.";
  DrawRoutes(bank,
             top_memories,
             bottom_memories,
             clk_bufs,
             output_bufs,
             stack_layout,
             cell->layout(),
             cell->circuit());
  cell->layout()->SetTilingBounds(tiling_bounds);

  return cell.release();
}


void Sky130InterconnectMux1::DrawRoutes(
    const MemoryBank &bank,
    const std::vector<geometry::Instance*> &top_memories,
    const std::vector<geometry::Instance*> &bottom_memories,
    const std::vector<geometry::Instance*> &clk_bufs,
    const std::vector<geometry::Instance*> &output_buffers,
    geometry::Instance *stack,
    Layout *layout,
    Circuit *circuit) const {
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

  // Scan chain connections on the left side can be connected on metal 2, and
  // this should effectively only take up one channel width over the tap cells
  // and not detract from the routing channels in the left-most block.
  std::vector<geometry::Instance*> scan_order;
  scan_order.insert(
      scan_order.begin(), top_memories.begin(), top_memories.end());
  scan_order.insert(
      scan_order.begin(), bottom_memories.begin(), bottom_memories.end());

  // TODO(aryap): If the layout gets _any_ more complicated than this we will
  // need more sophisticated ways to reuse the control lines for the scan chain.
  // In fact they might already be too big (too much R & C)!
  //std::set<std::pair<geometry::Instance*, geometry::Instance*>>
  //    scan_chain_pairs;
  //for (auto it = scan_order.begin(); it != scan_order.end(); ++it) {
  //  geometry::Instance *current = *it;
  //  geometry::Instance *next = *(it + 1);
  //  scan_chain_pairs.insert({current, next});
  //}

  std::optional<int64_t> left_most_vertical_x;
  std::optional<int64_t> right_most_vertical_x;

  auto update_bounds_fn = [&](int64_t x) {
    Utility::UpdateMin(x, &left_most_vertical_x);
    Utility::UpdateMax(x, &right_most_vertical_x);
  };

  // Track the names used for wires connecting the memories to each other (in
  // the scan chain) and the mux control inputs.
  std::map<geometry::Instance*, std::string> memory_output_nets;

  auto connect_memory_to_control_fn = [&](
      geometry::Instance *memory, size_t gate_number, bool complement) {
    // To associate these points with the control signals they require, consider
    // that for gate n, the positive control signal connects to the NMOS FET
    // and the inverted control signal connects to the PMOS FET. Then follow the
    // naming convention in Sky130TransmissionGateStack.
    //
    // TODO(aryap): We could probably make this easier by making the port
    // association an explicit feature of the (TransmissionGateStack) Cell?
    std::string control_name = absl::StrCat(
        "S", gate_number, complement ? "_B" : "");
    std::string memory_port = complement ? "QI" : "Q";
    std::string wire_name = absl::StrCat(
        memory->name(), "_", memory_port, "_out");
    // For the scan chain, later:
    if (!complement) {
      memory_output_nets.insert({memory, wire_name});
    }
    circuit::Wire control_wire = circuit->AddSignal(wire_name);
    stack->circuit_instance()->Connect(control_name, control_wire);
    memory->circuit_instance()->Connect(memory_port, control_wire);
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
                      layout,
                      absl::StrCat(memory->name(), ".Q"));

    update_bounds_fn(vertical_x - met2_pitch);

    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x,
                      layout,
                      absl::StrCat(memory->name(), ".QI"));

    update_bounds_fn(vertical_x);

    // Add a polycon (licon) and an li pad between the poly tab and the mcon via
    // that connects to the routes we just put down. To avoid the nearest poly
    // tab, these stick outward.
    AddPolyconAndLi(p_tab_centre, true, layout);
    AddPolyconAndLi(n_tab_centre, false, layout);

    // We also use this opportunity to make scan-chain connections from memory Q
    // outputs to the D inputs on the next memory up. We only do this when the
    // output Q is on the left, since they will always line up with a single 
    connect_memory_to_control_fn(memory, gate_number, true);
    connect_memory_to_control_fn(memory, gate_number, false);

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
                      layout,
                      absl::StrCat(memory->name(), ".Q"));
    
    update_bounds_fn(vertical_x);

    ConnectVertically(mem_QI->centre(),
                      n_tab_centre,
                      vertical_x + met2_pitch,
                      layout,
                      absl::StrCat(memory->name(), ".QI"));

    update_bounds_fn(vertical_x + met2_pitch);

    AddPolyconAndLi(p_tab_centre, true, layout);
    AddPolyconAndLi(n_tab_centre, false, layout);

    connect_memory_to_control_fn(memory, gate_number, true);
    connect_memory_to_control_fn(memory, gate_number, false);

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
  DrawScanChain(scan_order,
                memory_output_nets,
                bottom_memories.size() - 1,
                columns_left_x[kScanChainLeftIndex],
                columns_right_x[kScanChainRightIndex],
                layout,
                circuit);

  int64_t output_port_x = bank.GetTilingBounds()->upper_right().x();
  int64_t mux_pre_buffer_y = 0;

  DrawOutput(output_buffers,
             stack,
             &mux_pre_buffer_y,
             output_port_x,
             layout,
             circuit);
  DrawInputs(stack,
             mux_pre_buffer_y,
             columns_left_x[kInterconnectLeftStartIndex],
             false,
             layout,
             circuit);

  DrawClock(bank,
            top_memories,
            bottom_memories,
            clk_bufs,
            columns_right_x[kInputClockRightIndex],
            columns_right_x[kClockRightIndex],
            columns_right_x[kClockIRightIndex],
            layout,
            circuit);

  DrawPowerAndGround(bank,
                     columns_right_x[kVPWRVGNDStartRightIndex],
                     layout,
                     circuit);
}

void Sky130InterconnectMux1::DrawPowerAndGround(
    const MemoryBank &bank,
    int64_t start_column_x,
    Layout *layout,
    Circuit *circuit) const {
  // Update circuit model.
  circuit::Wire power_wire = circuit->AddSignal(parameters_.power_net);
  circuit::Wire ground_wire = circuit->AddSignal(parameters_.ground_net);
  circuit->AddPort(power_wire);
  circuit->AddPort(ground_wire);

  //int64_t max_y = bank.GetTilingBounds()->upper_right().y();
  //int64_t min_y = bank.GetTilingBounds()->lower_left().y();

  // First figure out where the ground/power ports are:
  std::set<int64_t> power_y;
  std::set<int64_t> ground_y;
  for (const RowGuide &row : bank.rows()) {
    for (geometry::Instance *instance : row.instances()) {
      std::vector<geometry::Port*> power_ports;

      instance->GetInstancePorts(parameters_.power_net, &power_ports);
      for (geometry::Port *port : power_ports) {
        power_y.insert(port->centre().y());
      }
      std::vector<geometry::Port*> ground_ports;
      instance->GetInstancePorts(parameters_.ground_net, &ground_ports);
      for (geometry::Port *port : ground_ports) {
        ground_y.insert(port->centre().y());
      }

      instance->circuit_instance()->Connect({
          {parameters_.power_net, power_wire},
          {parameters_.ground_net, ground_wire},
          {"VPB", power_wire},
          {"VNB", ground_wire}
      });
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
  vpwr_strap->SetConnectableNet(parameters_.power_net);

  geometry::Polygon *vgnd_strap = draw_strap_fn(ground_y, vgnd_x);
  vgnd_strap->SetConnectableNet(parameters_.ground_net);

  geometry::PortSet clock_ports;
  layout->GetPorts("CLK", &clock_ports);
  int64_t vpwr_port_y = 0;
  int64_t vgnd_port_y = 0;
  if (clock_ports.size() == 1) {
    int64_t clk_port_y = (*clock_ports.begin())->centre().y();
    vpwr_port_y = clk_port_y - 6 * met3_rules.min_pitch;
    vgnd_port_y = clk_port_y + 6 * met3_rules.min_pitch;
  } else {
    LOG(WARNING) << "Expected to find only one CLK port, but found "
                 << clock_ports.size();
  }

  layout->MakePin(parameters_.power_net, {vpwr_x, vpwr_port_y}, "met2.pin");
  layout->MakePin(parameters_.ground_net, {vgnd_x, vgnd_port_y}, "met2.pin");

}

void Sky130InterconnectMux1::DrawClock(
    const MemoryBank &bank,
    const std::vector<geometry::Instance*> &top_memories,
    const std::vector<geometry::Instance*> &bottom_memories,
    const std::vector<geometry::Instance*> &clk_bufs,
    int64_t input_clk_x,
    int64_t clk_x,
    int64_t clk_i_x,
    Layout *layout,
    Circuit *circuit) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  circuit::Wire clock_wire = circuit->AddSignal("CLK");
  circuit->AddPort(clock_wire);

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

    std::string clk_i_internal_name = absl::StrCat("clk_i_internal_", b);
    std::string clk_internal_name = absl::StrCat("clk_internal_", b);

    circuit::Wire clk_i_internal_wire = circuit->AddSignal(clk_i_internal_name);
    circuit::Wire clk_internal_wire = circuit->AddSignal(clk_internal_name);

    for (geometry::Instance *memory : memories) {
      geometry::Port *port = memory->GetNearestPortNamed({clk_x, 0}, "CLK");
      LOG_IF(FATAL, !port) << "No port named CLK on memory " << memory->name();
      memory_CLK_centres.push_back(port->centre());
      clk_spine_connections.push_back({clk_x, port->centre().y()});

      port = memory->GetNearestPortNamed({clk_i_x, 0}, "CLKI");
      LOG_IF(FATAL, !port) << "No port named CLKI on memory " << memory->name();
      memory_CLKI_centres.push_back(port->centre());
      clk_i_spine_connections.push_back({clk_i_x, port->centre().y()});

      memory->circuit_instance()->Connect("CLK", clk_internal_wire);
      memory->circuit_instance()->Connect("CLKI", clk_i_internal_wire);
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
        clk_internal_name,
        memory_CLK_centres,
        clk_x,
        db.Rules("met2.drawing").min_width,
        layout);

    layout->MakeVerticalSpineWithFingers(
        "met2.drawing",
        "met1.drawing",
        clk_i_internal_name,
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

    buf->circuit_instance()->Connect("X", clk_internal_wire);
    buf->circuit_instance()->Connect("P", clk_i_internal_wire);

    ++b;
  }

  std::vector<geometry::Point> buf_A_centres;
  for (geometry::Instance *buf : clk_bufs) {
    geometry::Port *port = buf->GetNearestPortNamed({input_clk_x, 0}, "A");
    LOG_IF(FATAL, !port) << "No port named A on buf " << buf->name();
    buf_A_centres.push_back(port->centre());

    buf->circuit_instance()->Connect("A", clock_wire);
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

void Sky130InterconnectMux1::DrawOutput(
    const std::vector<geometry::Instance*> &output_buffers,
    geometry::Instance *stack,
    int64_t *mux_pre_buffer_y,
    int64_t output_port_x,
    Layout *layout,
    Circuit *circuit) const {
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
  stack->GetInstancePorts(kStackOutputName, &outputs);

  std::vector<geometry::Point> wire_points;
  std::vector<Layout::ViaToSomeLayer> connection_points;
  for (geometry::Port *port : outputs) {
    connection_points.push_back({
        .centre = port->centre(),
        .layer_name = "li.drawing"
    });
    wire_points.push_back(port->centre());
  }

  geometry::Port *buf_A = output_buffers[0]->GetFirstPortNamed("A");

  wire_points.push_back({buf_A->centre().x(), wire_points.back().y()});

  wire_points.push_back(buf_A->centre());
  connection_points.push_back({
      .centre = buf_A->centre(),
      .layer_name = "li.drawing"
  });
  layout->MakeWire(wire_points, "met1.drawing", connection_points);

  circuit::Wire stack_to_buf = circuit->AddSignal("stack_to_buf");
  stack->circuit_instance()->Connect(kStackOutputName, stack_to_buf);
  output_buffers[0]->circuit_instance()->Connect("A", stack_to_buf);

  *mux_pre_buffer_y = wire_points.front().y();

  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;
  
  // Because DrawInputs will allocate parameters_.num_inputs-many inputs
  // vertically starting below the mux_pre_buffer_y line, we halve and round
  // down to find the number expected above that line, and then align the final
  // output to the top input:
  int64_t num_below = parameters_.num_inputs / 2;
  int64_t final_output_y = *mux_pre_buffer_y - num_below * met2_pitch;

  // Connect the buff output to the edge of the design:
  geometry::Port *buf_X = output_buffers[0]->GetFirstPortNamed("X");

  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch;
  int64_t vertical_x = buf_X->centre().x() + met1_pitch;

  std::vector<geometry::Point> output_wire = {
      buf_X->centre(),
      {vertical_x, buf_X->centre().y()},
      {vertical_x, final_output_y},
      geometry::Point {output_port_x, final_output_y}
  };

  // It is very important that the output wire be labelled with its net so that
  // the RoutingGrid can make exceptions for blockages when connecting to it!
  geometry::Polygon *out_wire = layout->MakeWire(
      output_wire,
      "met1.drawing",  // Wire layer.
      "li.drawing",    // Start layer.
      std::nullopt,    // End layer.
      false,
      false,
      kMuxOutputName);
  out_wire->set_is_connectable(true);

  layout->MakePin(kMuxOutputName, output_wire.back(), "met1.pin");

  circuit::Wire output_signal = circuit->AddSignal(kMuxOutputName);
  circuit->AddPort(output_signal);
  output_buffers[0]->circuit_instance()->Connect("X", output_signal);

  // To keep VLSIR happy, connect port P to a floating net (it is disconnected).
  // TODO(aryap): This should be automatically emitted by our circuit model for
  // explicitly disconnected ports!
  circuit::Wire disconnected_P = circuit->AddSignal("disconnected_P");
  output_buffers[0]->circuit_instance()->Connect("P", disconnected_P);
}

void Sky130InterconnectMux1::DrawInputs(
    geometry::Instance *stack,
    int64_t mux_pre_buffer_y,
    int64_t vertical_x_left,
    bool allow_mux_pre_buffer_y_use,
    Layout *layout,
    Circuit *circuit) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch;
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;

  // Compute the horizontal channels we have:
  int64_t horizontal_y_max = stack->GetPointOrDie("net_X0_via_top_0").y();
  int64_t horizontal_y_min = stack->GetPointOrDie("net_X0_via_bottom_0").y();

  int64_t num_tracks = (horizontal_y_max - horizontal_y_min) / met1_pitch;

  LOG_IF(WARNING, num_tracks < parameters_.num_inputs)
      << "The number of tracks available (" << num_tracks
      << ") is less that the number of inputs (" << parameters_.num_inputs
      << ")";

  // Compute the x positions of the vertical-channel pins. Align the pins so
  // that they are multiples of met2_pitch from the left hand side of the cell.
  vertical_x_left = Utility::NextMultiple(
      vertical_x_left, met2_pitch) - met2_pitch;
 
  std::vector<int64_t> input_channels_x;

  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    int64_t channel_x = vertical_x_left - i * met2_pitch;
    if (i == parameters_.num_inputs - 1 && allow_mux_pre_buffer_y_use) {
      channel_x -= met2_pitch;
    }
    input_channels_x.push_back(channel_x);
  }

  // Connect the inputs.
  bool up = false;
  size_t j = 1;
  for (size_t i = 0; i < parameters_.num_inputs; ++i) {
    int64_t k = allow_mux_pre_buffer_y_use && !up ? j - 1: j;
    int64_t y_offset = k * met1_pitch;
    int64_t y = mux_pre_buffer_y + (up ? y_offset : -y_offset);
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
        true,    // Pad-only at the start.
        false,
        input_name);

    ScopedLayer sl(layout, "met1.pin");
    layout->MakePin(input_name, start, "met1.pin");

    // Add this to the circuit model.
    circuit::Wire input = circuit->AddSignal(input_name);
    circuit->AddPort(input);
    stack->circuit_instance()->Connect(input_name, input);
  }
}

void Sky130InterconnectMux1::DrawScanChain(
    const std::vector<geometry::Instance*> &scan_order,
    const std::map<geometry::Instance*, std::string> &memory_output_nets,
    int64_t num_ff_rows_bottom,
    int64_t vertical_x_left,
    int64_t vertical_x_right,
    Layout *layout,
    Circuit *circuit) const {
  size_t row = 0;
  for (auto it = scan_order.begin(); it < scan_order.end() - 1; ++it) {
    // As a reminder, the flip flop latched the value at input D on a clock
    // edge, and then it appears at output Q.
 
    geometry::Instance *memory = *it;
    geometry::Instance *next = *(it + 1);

    std::string net = absl::StrCat(memory->name(), ".Q");

    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_D = memory->GetFirstPortNamed("D");

    geometry::Port *next_D = next->GetFirstPortNamed("D");

    layout->MakePin(memory->name() + "/Q", mem_Q->centre(), "li.pin");
    layout->MakePin(memory->name() + "/D", mem_D->centre(), "li.pin");
  
    // We check to see which way around the FF is. If input is left of output,
    // it's oriented normally, and we connect using a metal bar on the left of
    // the previous connections. If it's the other way we use a metal bar on
    // the right. This test means we don't have to rely on a particular
    // orientation pattern when the memories are laid out.
    //
    // FIXME(aryap): A more robust way to do the scan chain (without doing
    // anything intelligent) will be to dedicate a vertical channel on the
    // left and right sides of the flip flops that avoids other routes we are
    // planning, like the control wires. This limits vertical channel usage to
    // 2 and gives us more room for the clk, vdd, vss met2 lines.
    //
    // Ok no problem we just have to push the right-most vertical out more to
    // avoid using met1 too close to other met1!
    int64_t vertical_x = mem_Q->centre().IsStrictlyLeftOf(mem_D->centre()) &&
        row != num_ff_rows_bottom ? vertical_x_left : vertical_x_right;
    ++row;
    //if (i != num_ff_rows_bottom) {
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
                      layout,
                      net);

    LOG(INFO) << memory->name() << " -> " << next->name() << " "
              << mem_Q->centre() << " -> " << next_D->centre();

    // This better exist!
    auto out_name_it = memory_output_nets.find(memory);
    DCHECK(out_name_it != memory_output_nets.end());
    const std::string &wire_name = out_name_it->second;
    circuit::Wire wire(*circuit->GetSignal(wire_name), 0);
    DCHECK(memory->circuit_instance()->GetConnection("Q"));
    next->circuit_instance()->Connect("D", wire);
  }

  layout->MakePin("SCAN_IN",
                  scan_order.front()->GetFirstPortNamed("D")->centre(),
                  "li.pin");
  layout->MakePin("SCAN_OUT",
                  scan_order.back()->GetFirstPortNamed("D")->centre(),
                  "li.pin");

  circuit::Wire scan_in = circuit->AddSignal("SCAN_IN");
  circuit::Wire scan_out = circuit->AddSignal("SCAN_OUT");

  scan_order.front()->circuit_instance()->Connect("D", scan_in);

  DCHECK(scan_order.back()->circuit_instance()->GetConnection("Q"));

  circuit->AddPort(scan_in);
  circuit->AddPort(scan_out);
}


// We will determine the minimum vertical poly-to-boundary spacing such that any
// rectangle (wire) placed at that inset from the edge on any of the poly, li or
// met1 layers will be far enough away from shapes in the surrounding cell on
// the same layers. We only care how far shapes in the surrounding cell overflow
// its tiling bounds, since that is the incursion into what will be the mux
// cell. We also assume uniformity across the width of the cell.
int64_t Sky130InterconnectMux1::FigurePolyBoundarySeparationForMux(
    bfg::Layout *neighbour_layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  bfg::geometry::Rectangle tiling_bounds = neighbour_layout->GetTilingBounds();

  // TODO(aryap):
  // 1) We should be able to turn off the vertical poly pitch being used for
  // cell height spacing calculation - in this case we are not stacking another
  // one of these transmission gate stacks on top. Or maybe it should be
  // explicit. The height should actually be a multiple of the standard-cell
  // height unit, i.e. an 8-track cell is 8*340 (met1 pitch) = 2720 um.
  static const std::vector kCheckedLayers = {
    "poly.drawing", "met1.drawing", "li.drawing"
  };

  // Since the quantity we return will be used as a spacing from the top-most
  // poly edge, we correct our spacings with an amount we call "underflow",
  // which measures how much extra room there is between the poly edge and each
  // of the other metal edges we're checking spacings on.
  //
  // TODO(aryap): It is annoying that this class has to worry about this. But we
  // can't get the details of poly tab heights and such from the child
  // generators until they are generated. This is a more general problem to
  // tackle but, it is annoying here. Maybe we just give the generate the layout
  // of the neighbour to avoid?
  //
  // OR MAYBE the min-spacing is specified for each layer, and the generator has
  // to apply it according to its construction.
  //
  // Anyway. This could be a lot smoother.
  std::map<std::string, int64_t> underflow_by_layer;
  
  // li.drawing is:
  underflow_by_layer["poly.drawing"] = 0;
  underflow_by_layer["li.drawing"] = (
      db.TypicalViaEncap("li.drawing", "licon.drawing").width -
      db.TypicalViaEncap("poly.drawing", "pcon.drawing").length) / 2;
  underflow_by_layer["met1.drawing"] = (
      db.TypicalViaEncap("met1.drawing", "mcon.drawing").width -
      db.TypicalViaEncap("poly.drawing", "pcon.drawing").length) / 2;

  int64_t max_spacing = 0;

  for (const std::string &layer : kCheckedLayers) {
    bfg::geometry::Rectangle layer_bounds =
        neighbour_layout->GetBoundingBoxByNameOrDie(layer);

    // Minimum separation on this layer.
    int64_t layer_min_separation = db.Rules(layer).min_separation;

    // First check the top. We'll call the gap between the layer and the edge of
    // the adjacent cell the "overflow".
    int64_t overflow =
        layer_bounds.upper_right().y() - tiling_bounds.upper_right().y();

    int64_t underflow = underflow_by_layer[layer];

    int64_t required_separation = overflow + layer_min_separation + underflow;
    max_spacing = std::max(max_spacing, required_separation);

    // Then the bottom:
    overflow = tiling_bounds.lower_left().y() - layer_bounds.lower_left().y();
    required_separation = overflow + layer_min_separation + underflow;
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
std::vector<geometry::Point> Sky130InterconnectMux1::ConnectVertically(
    const geometry::Point &top,
    const geometry::Point &bottom,
    int64_t vertical_x,
    bfg::Layout *layout,
    const std::string &net) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  geometry::Point p1 = {vertical_x, top.y()};
  geometry::Point p2 = {vertical_x, bottom.y()};

  std::vector<geometry::Point> points = {top, p1, p2, bottom};

  layout->MakeVia("mcon.drawing", top);
  layout->MakeAlternatingWire(
      points, "met1.drawing", "met2.drawing", net, true);
  layout->MakeVia("mcon.drawing", bottom);

  return points;
}

void Sky130InterconnectMux1::AddPolyconAndLi(
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
