#ifndef ATOMS_SKY130_INTERCONNECT_MUX6_H_
#define ATOMS_SKY130_INTERCONNECT_MUX6_H_

#include "atom.h"
#include "sky130_parameters.h"
#include "../memory_bank.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/instance.h"
#include "proto/parameters/sky130_interconnect_mux6.pb.h"
#include "sky130_transmission_gate_stack.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// The mux6 we will build for interconnect (and the family of components like
// it) will look like:
//
//           +-+------------------------------+-+----+
//  Decap    |D| (Out)      FF                | | D  |
//  cells    +-+------------------------------+-+----+
//  creating |D|            FF          (Out) |T| <---- Tap
//  vertical +-+------------------------------+------+
//  routing  |D| (Out)      FF                | Buf  |  Clock Buffer
//  channel  +-+------------------------------+------+
//      -->  |D|  Transmission gate 6:1 Muxes | Buf  |  Output Buffer
//           | |                              |      |
//           +-+------------------------------+------+
//           |D| (Out)      FF                |T|    |
//           +-+------------------------------+-+----+
//           |D|            FF          (Out) |      |
//           +-+------------------------------+------+
//           |D| (Out)      FF                |T|  D |
//           +-+------------------------------+------+
//           |       D          |T|          D       |  <- horizontal
//           +-+------------------------------+------+     routing
//                                                         channel
//
// The transmission gates are arranged so that the grow from left to right. The
// flip flops are split in half and placed in rows emanating vertically from
// the transmission gates. Following this pattern, we draw connecting wires
// from the outputs of the flip-flops to the gates of of the transmission
// gates, alternating between north and south flip-flops, and between left-most
// and right-most unconnected gates. This aligns the outputs of the flip-flops
// with the nearest transmission gates and avoids congestion:
//
//           +-+------------------------------+-+----+
//           | | (Out)      FF                | |    |
//           +-+------------------------------+-+----+
//           | |            FF          ++    | |    |
//           +-+------------------------||----+------+
//           | |  ++        FF      (3) ||    |      |
//           +-+--||-(1)----------------||----+------+
//           | |  |+ +-    Transmissi.  |+ +- |      |
//           | |  -+ +|       gates     -+ +| |      |
//           +-+-----||-(2)----------------||-+------+
//           | |  +--++     FF         (4) || | |    |
//           +-+---------------------------||-+-+----+
//           | |            FF             ++ |      |
//           +-+------------------------------+------+
//           | | (Out)      FF                | |    |
//           +-+------------------------------+------+
//           |       D          |T|          D       |
//           +-+------------------------------+------+
//    
// TODO(aryap): We can also insert buffers into the vertical routing channel on
// the left in order to buffer long wires travelling through.

class Sky130InterconnectMux6 : public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    // This is the default for the PDK.
    static constexpr uint64_t kHorizontalTilingUnitNm = 460;

    uint32_t num_inputs = 6;
    std::optional<uint64_t> poly_pitch_nm = 500;

    std::optional<uint64_t> vertical_pitch_nm = 340;

    std::optional<uint64_t> vertical_offset_nm = 170;

    std::optional<uint64_t> horizontal_pitch_nm = 460;

    std::optional<uint64_t> vertical_routing_channel_width_nm =
        ((8 * 340) / 460 + 1) * 460;

    // For N tracks we need (N - 1) pitches and additional two half-pitches for
    // the top and bottom. But given the 235 um met1 VPWR/VGND rail overhang we
    // need an extra at the top and an extra at the bottom too. So we need
    //    (N - 1) + 0.5 + 0.5 + 2 = N + 2
    std::optional<uint64_t> horizontal_routing_channel_height_nm = 
        (8 + 2) * 340;

    std::optional<uint64_t> power_ground_strap_width_nm = 300;

    bool redraw_rail_vias = true;

    // Either 1 or 2.
    //
    // If 2, poly_pitch_nm will be increased to at least 2x met2 pitch.
    uint16_t num_outputs = 1;

    void ToProto(proto::parameters::Sky130InterconnectMux6 *pb) const;
    void FromProto(const proto::parameters::Sky130InterconnectMux6 &pb);
  };

  static constexpr char kMuxOutputName[] = "Z";

  Sky130InterconnectMux6(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
    const PhysicalPropertiesDatabase &db = design_db_->physical_db();
    // Correct any input parameters as needed:
    if (parameters_.num_outputs == 2) {
      int64_t min_poly_pitch = std::max({
          // Closest pitch from rulebook.
          db.Rules("poly.drawing").min_pitch,
          // Closest they can to accommodate two horizontal met1 via encaps.
          db.Rules("met1.drawing").min_separation +
              db.Rules("met2.drawing").min_pitch / 2 +
              std::max(
                  db.TypicalViaEncap("met1.drawing", "via1.drawing").length,
                  db.TypicalViaEncap("met1.drawing", "mcon.drawing").length)
      });
      uint64_t min_poly_pitch_nm = static_cast<uint64_t>(
          db.ToExternalUnits(min_poly_pitch));
      parameters_.poly_pitch_nm = std::max(
          parameters_.poly_pitch_nm.value_or(0), min_poly_pitch_nm);
    }
  }

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly
  // merge outputs directly into parent cell.
  bfg::Cell *Generate() override;

 private:
  void ConfigureSky130Parameters(Sky130Parameters *base_params) const {
    base_params->power_net = parameters_.power_net;
    base_params->ground_net = parameters_.ground_net;
  }

  std::vector<std::vector<std::string>> BuildSingleOutputNetSequences() const;
  std::vector<std::vector<std::string>> BuildDualOutputNetSequences() const;
  Sky130TransmissionGateStack::Parameters BuildTransmissionGateParams(
    geometry::Instance *vertical_neighbour) const;

  int64_t FigurePolyBoundarySeparationForMux(
      bfg::Layout *neighbour_layout) const;

  std::vector<geometry::Instance*> AddMemoriesVertically(
      size_t first_row, uint32_t count, uint32_t columns, MemoryBank *bank);
  geometry::Instance *AddClockBufferRight(
      const std::string &suffix, size_t row, MemoryBank *bank);
  geometry::Instance *AddTransmissionGateStackRight(
      geometry::Instance *vertical_neighbour, size_t row, MemoryBank *bank);
  geometry::Instance *AddOutputBufferRight(
      const std::string &suffix, uint32_t height, size_t row, MemoryBank *bank);

  Cell *MakeDecapCell(uint32_t width_nm, uint32_t height_nm);

  void DrawRoutesForSingleOutput(
      const MemoryBank &bank,
      const std::vector<geometry::Instance*> &top_memories,
      const std::vector<geometry::Instance*> &bottom_memories,
      const std::vector<geometry::Instance*> &clk_bufs,
      geometry::Instance *stack,
      geometry::Instance *output_buffer,
      Layout *layout,
      Circuit *circuit) const;
  void DrawRoutesForDualOutputs(
      const MemoryBank &bank,
      const std::vector<geometry::Instance*> &top_memories,
      const std::vector<geometry::Instance*> &bottom_memories,
      const std::vector<geometry::Instance*> &clk_bufs,
      const std::vector<geometry::Instance*> &output_buffers,
      geometry::Instance *stack,
      Layout *layout,
      Circuit *circuit) const;

  void DrawScanChain(
      const std::vector<geometry::Instance*> &all_memories,
      const std::map<geometry::Instance*, std::string> &memory_output_nets,
      int64_t num_ff_bottom,
      int64_t vertical_x_left,
      int64_t vertical_x_right,
      Layout *layout,
      Circuit *circuit) const;

  void DrawOutput(geometry::Instance *stack,
                  geometry::Instance *output_buffer,
                  int64_t *mux_pre_buffer_y,
                  int64_t output_port_x,
                  Layout *layout,
                  Circuit *circuit) const;

  void DrawInputs(geometry::Instance *stack,
                  int64_t mux_pre_buffer_y,
                  int64_t vertical_x_left,
                  Layout *layout,
                  Circuit *circuit) const;

  void DrawPowerAndGround(const MemoryBank &bank,
                          int64_t start_column_x,
                          Layout *layout,
                          Circuit *circuit) const;

  void DrawClock(const MemoryBank &bank,
                 const std::vector<geometry::Instance*> &top_memories,
                 const std::vector<geometry::Instance*> &bottom_memories,
                 const std::vector<geometry::Instance*> &clk_bufs,
                 int64_t input_clk_x,
                 int64_t clk_x,
                 int64_t clk_i_x,
                 Layout *layout,
                 Circuit *circuit) const;

  void ConnectVertically(const geometry::Point &top,
                         const geometry::Point &bottom,
                         int64_t vertical_x,
                         bfg::Layout *layout) const;
  void AddPolyconAndLi(const geometry::Point tab_centre,
                       bool bulges_up,
                       bfg::Layout *layout) const;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_INTERCONNECT_MUX6_H_
