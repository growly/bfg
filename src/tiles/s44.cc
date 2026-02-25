#include "s44.h"

#include "../atoms/sky130_carry1.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_decap.h"
#include "../circuit.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../tiles/lut_b.h"

namespace bfg {
namespace tiles {

Cell *S44::Generate() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name_));

  bfg::Circuit *circuit = new bfg::Circuit();
  cell->SetCircuit(circuit);
  bfg::Layout *layout = new bfg::Layout(db);
  cell->SetLayout(layout);

  MemoryBank bank = MemoryBank(layout,
                               circuit,
                               design_db_,
                               nullptr,    // No tap cells.
                               false,      // Don't rotate alternate rows.
                               false,      // Don't rotate first row.
                               geometry::Compass::LEFT);

  static constexpr size_t kBottom = 0;
  static constexpr size_t kMiddle = 1;
  static constexpr size_t kTop = 2;

  std::string root_lut_name = "lut4";

  // Add 2 4-LUTs.
  //
  // Because we add a row to the top LUT, we have a VPWR/VGND parity difference.
  // The first and second row must be rotated.
  {
    // The bottom one goes first. It has the s2 input selection mux for the
    // soft-S44.
    std::string lut_name = absl::StrCat(root_lut_name, "_B");
    LutB::Parameters bottom_lut_params = {
        .lut_size = 4,
        .add_s2_input_mux = true
        // TODO(aryap): Enable input-sharing 2:1 mux.
    };
    LutB bottom_lut4_gen(bottom_lut_params, design_db_);
    Cell *bottom_lut4_cell = bottom_lut4_gen.GenerateIntoDatabase(lut_name);

    bank.Row(kBottom).set_rotate_instances(true);
    bank.InstantiateRight(
        kBottom, absl::StrCat(lut_name, "_i"), bottom_lut4_cell);
  }

  {
    // The top one goes second. It has an additional input on the output and
    // registered output selection muxes.
    std::string lut_name = absl::StrCat(root_lut_name, "_A");
    LutB::Parameters top_lut_params = {
        .lut_size = 4,
        .add_third_input_to_output_muxes = true
        // TODO(aryap):: Enable additional input option for registered and
        // combinational outputs.
    };
    LutB top_lut4_gen(top_lut_params, design_db_);
    Cell *top_lut4_cell = top_lut4_gen.GenerateIntoDatabase(lut_name);
    bank.InstantiateRight(
        kTop, absl::StrCat(lut_name, "_i_top"), top_lut4_cell);
  }

  bank.Row(kMiddle).set_rotate_instances(true);
  geometry::Instance *carry = nullptr;
  {
    std::string carry_name = "carry1";
    atoms::Sky130Carry1 carry1_generator(
        { .reverse_order = true }, design_db_);
    Cell *carry_cell = carry1_generator.GenerateIntoDatabase(carry_name);

    carry = bank.InstantiateRight(
        kMiddle, absl::StrCat(carry_name, "_i"), carry_cell);
  }

  {
    // We also need 1 more config memory for the soft S44 selector:
    std::string dfxtp_template_name = "s44_select_dfxtp";
    atoms::Sky130Dfxtp::Parameters dfxtp_params = {
      .add_inverted_output_port = false  // No QI.
    };
    atoms::Sky130Dfxtp dfxtp_generator(dfxtp_params, design_db_);
    Cell *dfxtp_cell = dfxtp_generator.GenerateIntoDatabase(
        PrefixCellName(dfxtp_template_name));
    bank.InstantiateRight(
        kMiddle, absl::StrCat(dfxtp_template_name, "_i"), dfxtp_cell);
  }

  uint64_t top_row_width = bank.Row(kTop).Width();
  uint64_t bottom_row_width = bank.Row(kBottom).Width();

  LOG_IF(WARNING, top_row_width != bottom_row_width)
      << "Top and bottom rows (LUTs) of S-44 should be the same width.";

  int64_t empty_span = std::max(bottom_row_width, top_row_width) -
      bank.Row(kMiddle).Width();

  atoms::Sky130Decap::Parameters base_params;
  base_params.power_net = "VPWR";
  base_params.ground_net = "VGND";
  base_params.draw_vpwr_vias = true;
  base_params.draw_vgnd_vias = true;
  atoms::Sky130Decap::FillDecapsRight(
      base_params, empty_span, &bank.Row(kMiddle));

  {
    std::string net = "C_O";
    // Continue the carry-out port on the carry to the end of the row.
    geometry::Point start =
        (*carry->GetInstancePortSet("C_O").begin())->centre();
    geometry::Point end = {
        bank.Row(kMiddle).GetTilingBounds()->upper_right().x(),
        start.y()};
    layout->MakeWire({start, end},
                     "met1.drawing", std::nullopt, std::nullopt,
                     false, false, net, false,
                     RoutingTrackDirection::kTrackHorizontal, std::nullopt);
    layout->MakePin("C_O", end, "met1.pin");
  }

  {
    // Elevate Sky130Carry1 pins.
    // TODO(aryap): Is there an easier way to do this?
    std::vector<std::string> elevated_pins = {
      "CONFIG_IN",
      "CONFIG_OUT",
      "CONFIG_CLK",
      "C_I",
      "G_0",
      "G_1",
      "S",
      "P"
    };

    for (const std::string &pin : elevated_pins) {
      geometry::Port *port = *carry->GetInstancePortSet(pin).begin();
      LOG(INFO) << "Recreating " << pin << " at " << port->centre()
                << " on layer " << port->layer();
      layout->MakePin(pin, port->centre(), port->layer());
    }
  }

  // FIXME(aryap):
  // We need to route connections to the two luts now, including the brokwn scan
  // chain (also TODO).

  // Add input and output ports.

  // Connect circuit elements.

  //int64_t lut_width = lut4_cell->layout()->GetTilingBounds().Width();

  //// In between every row of LUTs we add a horizontal channel, both for routing
  //// wires and for matching the rails at the top/bottom of the LUT cell.
  //int num_lut_rows = std::ceil(
  //    static_cast<double>(parameters_.kNumLUTs) /
  //    static_cast<double>(kLutsPerRow));
  //for (size_t i = 0; i < num_lut_rows - 1; ++i) {
  //  size_t row = 2 * i + 1;
  //  std::vector<int64_t> decap_widths = Utility::StripInUnits(
  //      lut_width * kLutsPerRow,
  //      atoms::Sky130Decap::Parameters::kMaxWidthNm,
  //      atoms::Sky130Parameters::kStandardCellUnitWidthNm,
  //      atoms::Sky130Decap::Parameters::kMinWidthNm);

  //  for (size_t j = 0; j < decap_widths.size(); ++j) {
  //    int64_t decap_width = decap_widths[j];
  //    std::string template_name = absl::StrFormat(
  //        "lut_decap_%d_%d_%d", i, j, decap_width);
  //    std::string instance_name = absl::StrCat(template_name, "_i");
  //    atoms::Sky130Decap::Parameters decap_params = {
  //      .width_nm = static_cast<uint64_t>(db.ToExternalUnits(decap_width))
  //      // Default height should be fine.
  //    };
  //    atoms::Sky130Decap decap(decap_params, design_db_);
  //    Cell *decap_cell = decap.GenerateIntoDatabase(template_name);
  //    luts.InstantiateRight(row, instance_name, decap_cell);
  //  }
  //}
  return cell.release();
}

}  // namespace tiles
}  // namespace bfg
