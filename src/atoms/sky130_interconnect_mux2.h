#ifndef ATOMS_SKY130_INTERCONNECT_MUX2_H_
#define ATOMS_SKY130_INTERCONNECT_MUX2_H_

#include "sky130_interconnect_mux1.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Sky130InterconnectMux2 is a two-output version of Sky130InterconnectMux1
// which assumes that, of M inputs, (M - 1) are shared. This device logically
// functions as two (M - 1):1 muxes.
//
// There is a small area saving in having interdigitated transmission gates for
// the two muxes, but the area is still dominated by configuration memory and
// wiring. It is laid out a lot like in the single-output case, but with
// memories spread across columns, and more annoying control wire routing.
//
// The mux2 will look something likelike:
//
//           +-+-------------------------+-------------------------++------+
//  Decap    |D| (Out)      FF           | (Out)      FF           ||   D  |
//  cells    +-+-------------------------+-------------------------++------+
//  creating |D|            FF     (Out) |            FF     (Out) ||T     |
//  vertical +-+-------------------------+-------------------------++------+
//  routing  |D| (Out)      FF           | (Out)      FF           || Buf  |
//  channel  +-+-------------------------+-----------+-----+-----+-++------+
//      -->  |D|  Transmission gates                 | Buf | Buf |  Decap  |
//           | |                                     |     |     |         |
//           +-+-------------------------+-----------+-----+-----+-++------+
//           |D| (Out)      FF           | (Out)      FF           ||T     |
//           +-+-------------------------+-------------------------++------+
//           |D|            FF     (Out) |            FF     (Out) ||      |
//           +-+-------------------------+-------------------------++------+
//           |D| (Out)      FF           | (Out)      FF           ||T     |
//           +-+------------+-+-------+--------------+-+-----------++------+
//           |       D      |T|          D           |T|          D        |
//           +-+------------+-+-------+--------------+-+-------------------+
//
class Sky130InterconnectMux2 : public Sky130InterconnectMux1 {
 public:
  Sky130InterconnectMux2(
      const Parameters &parameters, DesignDatabase *design_db)
      : Sky130InterconnectMux1(parameters, design_db) {}

 private:
  struct GateContacts {
    size_t number;
    geometry::Point p_contact;
    geometry::Point n_contact;
  };
  struct GateAssignment {
    GateContacts gate;
    int64_t p_vertical_x;
    int64_t p_gate_x;
    int64_t n_vertical_x;
    int64_t n_gate_x;
  };

  static bool CompareInstancesByQPortX(
      geometry::Instance *const lhs,
      geometry::Instance *const rhs);

  uint32_t NumOutputs() const override {
    return kNumOutputs;
  }
  uint32_t NumMemories() const override {
    // For the dual-output mux, there is one control line per input-output path,
    // and since all but two inputs are shared, that makes:
    return  (parameters_.num_inputs - 1) * 2;
  }
  // TODO(aryap): What's the name of that optimisation where the return values
  // of virtual functions which are constant get stored directly in the vtable
  // to avoid a call? Virtual constant propagation? "Constant propagation with
  // devirtualization" (that's what Claude said)?
  uint32_t NumMemoryColumns() const override {
    return 2;
  }
  // These should match what BuildNetSequences() does. It would be better to
  // combine the two concerns somehow.
  std::optional<std::string> StackTopLiChannelNet() const override {
    return absl::StrCat(kMuxOutputName, 0);
  }
  std::optional<std::string> StackBottomLiChannelNet() const override {
    return absl::StrCat(kMuxOutputName, 1);
  }

  std::vector<std::vector<std::string>> BuildNetSequences() const override;

  std::vector<GateAssignment> AssignRow(
      const std::vector<geometry::Instance*> &row_memories,
      int64_t max_offset_from_first_poly_x,
      std::vector<GateContacts> *gates) const;

  std::optional<std::vector<std::vector<GateAssignment>>>
  FindGateAssignment(
      const std::vector<geometry::Instance*> scan_order,
      size_t num_rows,
      size_t num_columns,
      int64_t max_offset_from_first_poly_x,
      std::vector<std::vector<geometry::Instance*>> *sorted_memories_per_row,
      std::vector<GateContacts> *gates) const;

  bool ConnectMemoryRowToStack(
      const std::vector<geometry::Instance*> &sorted_memories,
      const std::vector<GateAssignment> &gate_assignments,
      int64_t max_offset_from_first_poly_x,
      std::vector<GateContacts> *gates,
      geometry::Instance *stack,
      std::optional<int64_t> *left_most_vertical_x,
      std::optional<int64_t> *right_most_vertical_x,
      std::map<geometry::Instance*, std::string> *memory_output_nets,
      Layout *layout,
      Circuit *circuit) const;

  void ConnectControlWiresWithEffort(
      const std::vector<geometry::Instance*> scan_order,
      size_t num_rows,
      size_t num_columns,
      int64_t max_offset_from_first_poly_x,
      geometry::Instance *stack,
      std::vector<GateContacts> *gates,
      std::optional<int64_t> *left_most_vertical_x,
      std::optional<int64_t> *right_most_vertical_x,
      std::map<geometry::Instance*, std::string> *memory_output_nets,
      Layout *layout,
      Circuit *circuit) const;

  bool VerticalWireWouldCollideWithOthers(
      const std::string &net,
      int64_t vertical_x,
      int64_t first_y,
      int64_t second_y,
      Layout *layout) const;

  void DrawRoutes(
      const MemoryBank &bank,
      const std::vector<geometry::Instance*> &top_memories,
      const std::vector<geometry::Instance*> &bottom_memories,
      const std::vector<geometry::Instance*> &clk_bufs,
      const std::vector<geometry::Instance*> &output_buffers,
      geometry::Instance *stack,
      Layout *layout,
      Circuit *circuit) const override;

  void DrawScanChain(
      const std::vector<geometry::Instance*> &all_memories,
      const std::map<geometry::Instance*, std::string> &memory_output_nets,
      int64_t num_ff_bottom,
      int64_t vertical_x_left,
      int64_t vertical_x_right,
      Layout *layout,
      Circuit *circuit) const override;

  void DrawOutput(
      const std::vector<geometry::Instance*> &output_buffers,
      geometry::Instance *stack,
      int64_t *mux_pre_buffer_y,
      int64_t output_port_x,
      Layout *layout,
      Circuit *circuit) const override;

  void DrawPowerAndGround(const MemoryBank &bank,
                          int64_t start_column_x,
                          Layout *layout,
                          Circuit *circuit) const override;

  void DrawClock(const MemoryBank &bank,
                 const std::vector<geometry::Instance*> &top_memories,
                 const std::vector<geometry::Instance*> &bottom_memories,
                 const std::vector<geometry::Instance*> &clk_bufs,
                 int64_t input_clk_x,
                 int64_t clk_x,
                 int64_t clk_i_x,
                 Layout *layout,
                 Circuit *circuit) const override;

 private:
  // This is fixed for this implementation.
  static constexpr int kNumOutputs = 2;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_INTERCONNECT_MUX2_H_
