#ifndef ATOMS_SKY130_INTERCONNECT_MUX6_H_
#define ATOMS_SKY130_INTERCONNECT_MUX6_H_

#include "atom.h"
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
//    
// TODO(aryap): We can also insert buffers into the vertical routing channel on
// the left in order to buffer long wires travelling through.

class Sky130InterconnectMux6 : public Atom {
 public:
  struct Parameters {
    uint32_t num_inputs = 6;
    std::optional<uint64_t> poly_pitch_nm = 500;

    std::optional<uint64_t> vertical_pitch_nm = 400;

    std::optional<uint64_t> vertical_routing_channel_width_nm =
        ((8 * 340) / 460 + 1) * 460;

    void ToProto(proto::parameters::Sky130InterconnectMux6 *pb) const;
    void FromProto(const proto::parameters::Sky130InterconnectMux6 &pb);
  };

  Sky130InterconnectMux6(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
  }

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly
  // merge outputs directly into parent cell.
  bfg::Cell *Generate() override;

 private:
  static constexpr char kMuxOutputName[] = "Z";

  Sky130TransmissionGateStack::Parameters BuildTransmissionGateParams(
    geometry::Instance *vertical_neighbour) const;

  int64_t FigurePolyBoundarySeparationForMux(
      bfg::Layout *neighbour_layout) const;

  void DrawRoutes(
      const std::vector<geometry::Instance*> &top_memories,
      const std::vector<geometry::Instance*> &bottom_memories,
      int64_t output_port_x,
      geometry::Instance *stack,
      geometry::Instance *output_buffer,
      bfg::Layout *layout) const;

  void DrawScanChain(
      const std::vector<geometry::Instance*> &all_memories,
      int64_t num_ff_bottom,
      int64_t vertical_x_left,
      int64_t vertical_x_right,
      bfg::Layout *layout) const;

  void DrawOutput(geometry::Instance *stack,
                  geometry::Instance *output_buffer,
                  int64_t *output_port_y,
                  int64_t output_port_x,
                  Layout *layout) const;

  void DrawInputs(geometry::Instance *stack,
                  int64_t output_port_y,
                  int64_t vertical_x_left,
                  Layout *layout) const;

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
