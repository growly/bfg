#include "sky130_interconnect_mux2.h"

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
#include "../modulo.h"
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
#include <absl/strings/str_join.h>

namespace bfg {
namespace atoms {

bool Sky130InterconnectMux2::CompareInstancesByQPortX(
    geometry::Instance *const lhs,
    geometry::Instance *const rhs) {
  geometry::Port *lhs_port = lhs->GetFirstPortNamed("Q");
  geometry::Port *rhs_port = rhs->GetFirstPortNamed("Q");
  return lhs_port->centre().x() < rhs_port->centre().x();
}

std::vector<std::vector<std::string>>
Sky130InterconnectMux2::BuildNetSequences() const {
  // There's only one sequence.
  std::vector<std::string> sequence;

  size_t input_num = 0;
  size_t control_num = 0;
  size_t output_num = 0;

  size_t i = 0;
  while (input_num < parameters_.num_inputs) {
    // The odd entries are always control signals.
    if (i % 2 == 1) {
      // Simple output naming:
      //std::string control_name = absl::StrCat("S", control_num);
      // Descriptive output naming tells us which input is being connected to
      // which output:
      std::string control_name = absl::StrFormat(
          "S%d_%d",
          (control_num + 1) / 2,  // Expect integer truncation here and below.
          (control_num / 2) % 2 + (control_num % 2) / 2);
      sequence.push_back(control_name);
      control_num++;
    } else if (i % 4 == 0) {
      // Every 4-th entry starting from 0 is an input.
      std::string input_name = absl::StrCat("X", input_num);
      sequence.push_back(input_name);
      input_num++;
    } else {
      std::string output_name = absl::StrCat(kStackOutputName, output_num);
      sequence.push_back(output_name);
      output_num = (output_num + 1) % 2;
    }
    i++;
  }

  return {sequence};
}


void Sky130InterconnectMux2::DrawRoutes(
    const MemoryBank &bank,
    const std::vector<geometry::Instance*> &top_memories,
    const std::vector<geometry::Instance*> &bottom_memories,
    const std::vector<geometry::Instance*> &clk_bufs,
    const std::vector<geometry::Instance*> &output_buffers,
    geometry::Instance *stack,
    bfg::Layout *layout,
    bfg::Circuit *circuit) const {
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

  // Track the names used for wires connecting the memories to each other (in
  // the scan chain) and the mux control inputs.
  std::map<geometry::Instance*, std::string> memory_output_nets;

  // We want to separate the paths taken by wires from memories now on the same
  // rows, since for multiple outputs the are 2 columns of memories. We also
  // want to separate wires taken by the top and bottom memories. So we
  // alternate between both:
  //
  //      4           5
  //      3           2
  //      0       +---1
  //      |       |               top memories
  //      |       |
  //  0   1   2   3   4   5   6   gates
  //  |       |
  //  +---4   +-------5           bottom memories
  //      3           2
  //      0           1
  //
  //

  // This is the function for dual outputs, and elsewhere we force the number of
  // columns to 2 in that case:
  size_t num_columns = 2;
  size_t rows = static_cast<size_t>(std::ceil(
        static_cast<double>(bottom_memories.size()) /
        static_cast<double>(num_columns)));

  std::vector<GateContacts> top_gates;
  std::vector<GateContacts> bottom_gates;
  // Allocate even gates to the bottom memories. We expect there to be as many
  // gates as (input, output) paths.
  size_t num_gates = (parameters_.num_inputs - 1) * parameters_.num_outputs;
  for (size_t g = 0; g < num_gates; ++g) {
    GateContacts gate = {
      .number = g,
      .p_contact = stack->GetPointOrDie(
          absl::StrFormat("gate_%u_p_tab_centre", g)),
      .n_contact = stack->GetPointOrDie(
          absl::StrFormat("gate_%u_n_tab_centre", g))
    };
    if (g % 2 == 0) {
      bottom_gates.push_back(gate);
    } else {
      top_gates.push_back(gate);
    }
  }


  ConnectControlWiresWithEffort(bottom_memories, 
                                rows,
                                num_columns,
                                max_offset_from_first_poly_x,
                                stack,
                                &bottom_gates,
                                &left_most_vertical_x,
                                &right_most_vertical_x,
                                &memory_output_nets,
                                layout,
                                circuit);
  ConnectControlWiresWithEffort(top_memories,
                                rows,
                                num_columns,
                                max_offset_from_first_poly_x,
                                stack,
                                &top_gates,
                                &left_most_vertical_x,
                                &right_most_vertical_x,
                                &memory_output_nets,
                                layout,
                                circuit);

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
  constexpr size_t kScanChainRightIndex = 11;
  constexpr size_t kClockRightIndex = 5;
  constexpr size_t kClockIRightIndex = 7;
  constexpr size_t kInputClockRightIndex = 12;
  constexpr size_t kVPWRVGNDStartRightIndex = 13;

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
             true,
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

void Sky130InterconnectMux2::DrawScanChain(
    const std::vector<geometry::Instance*> &scan_order,
    const std::map<geometry::Instance*, std::string> &memory_output_nets,
    int64_t num_ff_rows_bottom,
    int64_t vertical_x_left,
    int64_t vertical_x_right,
    Layout *layout,
    Circuit *circuit) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  size_t row = 0;
  for (auto it = scan_order.begin(); it < scan_order.end() - 1; ++it) {
    // As a reminder, the flip flop latched the value at input D on a clock
    // edge, and then it appears at output Q.
 
    geometry::Instance *memory = *it;
    geometry::Instance *next = *(it + 1);

    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_D = memory->GetFirstPortNamed("D");

    geometry::Port *next_D = next->GetFirstPortNamed("D");

    std::string net = absl::StrCat(memory->name(), ".Q");
  
    int64_t vertical_x = 0;
    // There are three cases for scan chain links:
    // 1. They connect to their immediate neighbour on the same row, very close
    // by. These are routed on li.drawing.
    // 2. They connect to their neighbour vertically on the row above.
    // 3. They connect to their neighbour diagonally on the row TWO above.
    geometry::Rectangle memory_tiling_bounds = memory->GetTilingBounds();
    int64_t y_diff = std::abs(memory_tiling_bounds.lower_left().y() -
        next->GetTilingBounds().lower_left().y());
    int64_t row_height = memory_tiling_bounds.Height();
    if (y_diff == 0) {
      // TODO(aryap): Would be nice to have a generic function here to find the
      // best way to connect shapes from one collection to another. We could
      // build it up one unit-square at a time.
      //
      // For now it's easier to modify the source generators and add the points
      // we care about.

      geometry::Point source = memory->GetPointOrDie("li_Q_abutment");
      geometry::Point dest = next->GetPointOrDie("li_D_abutment");
      std::vector<geometry::Point> points = {source};
      if (std::abs(dest.y() - source.y()) <= db.Rules(
             "li.drawing").min_width / 2) {
        points.emplace_back(dest.x(), source.y());
      } else {
        points.emplace_back(source.x(), dest.y());
        points.push_back(dest);
      }
   
      layout->MakeWire(points, "li.drawing", std::nullopt, std::nullopt,
                       false, false, net, true);

    } else if (y_diff == row_height) {
      LOG(INFO) << "y_diff " << y_diff << " " << net << " ";
      vertical_x = mem_Q->centre().IsStrictlyLeftOf(mem_D->centre()) &&
          row != num_ff_rows_bottom ? vertical_x_left : vertical_x_right;

      bool problem = VerticalWireWouldCollideWithOthers(
          net, vertical_x, mem_Q->centre().y(), next_D->centre().y(), layout);
      if (problem) {
        vertical_x += (row != num_ff_rows_bottom ? -1 : 1) * db.Rules(
            "met2.drawing").min_separation;
      }

      // TODO(aryap): Test if any connectable on-net shape is within ~2 pitches
      // of the destination, as a heuristic for whether we need another wire. I
      // guess in reality we want to know if drawing another wire would be worse
      // than drawing a wire directly from the existing net. That doesn't
      // directly solve the problem of adjacent vias for two wires being too
      // close together, but it might just obviate the problem altogether.
      
      ConnectVertically(mem_Q->centre(),
                        next_D->centre(),
                        vertical_x,
                        layout,
                        net);
    } else {
      vertical_x = mem_Q->centre().IsStrictlyLeftOf(mem_D->centre()) &&
          row != num_ff_rows_bottom ? vertical_x_left : vertical_x_right;

      int64_t dest_y_offset = -3 * db.Rules("met1.drawing").min_pitch;

      geometry::Point source = mem_Q->centre();
      geometry::Point dest = next_D->centre();

      geometry::Point p1 = {vertical_x, source.y()};
      geometry::Point p2 = {vertical_x, dest.y() + dest_y_offset};
      geometry::Point p3 = {dest.x(), dest.y() + dest_y_offset};

      std::vector<geometry::Point> points = {source, p1, p2, p3, dest};
      layout->MakeVia("mcon.drawing", source);
      layout->MakeAlternatingWire(points, "met1.drawing", "met2.drawing", net);
      // We expect the final wire to be vertical, and thus on met2.
      layout->MakeVia("via1.drawing", dest);
      auto encap_info = db.TypicalViaEncap(
          "mcon.drawing", "met1.drawing", "via1.drawing");
      {
        ScopedLayer sl(layout, "met1.drawing");
        geometry::Rectangle *via_encap = layout->AddRectangle(
            geometry::Rectangle::CentredAt(
                dest, encap_info.length, encap_info.width));
        via_encap->set_net(net);
      }
      layout->MakeVia("mcon.drawing", dest);
      ++row;
    }


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
                  scan_order.back()->GetFirstPortNamed("Q")->centre(),
                  "li.pin");

  circuit::Wire scan_in = circuit->AddSignal("SCAN_IN");
  circuit::Wire scan_out = circuit->AddSignal("SCAN_OUT");

  scan_order.front()->circuit_instance()->Connect("D", scan_in);

  DCHECK(scan_order.back()->circuit_instance()->GetConnection("Q"));

  circuit->AddPort(scan_in);
  circuit->AddPort(scan_out);
}

bool Sky130InterconnectMux2::ConnectMemoryRowToStack(
    const std::vector<geometry::Instance*> &sorted_memories,
    const std::vector<GateAssignment> &gate_assignments,
    int64_t max_offset_from_first_poly_x,
    std::vector<GateContacts> *gates,
    geometry::Instance *stack,
    std::optional<int64_t> *left_most_vertical_x,
    std::optional<int64_t> *right_most_vertical_x,
    std::map<geometry::Instance*, std::string> *memory_output_nets,
    Layout *layout,
    Circuit *circuit) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;

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
        memory->name(), "_", memory_port, "_to_gate_", gate_number);
    // For the scan chain, later:
    if (!complement) {
      memory_output_nets->insert({memory, wire_name});
    }
    circuit::Wire control_wire = circuit->AddSignal(wire_name);
    stack->circuit_instance()->Connect(control_name, control_wire);
    memory->circuit_instance()->Connect(memory_port, control_wire);
  };

  auto update_bounds_fn = [&](int64_t x) {
    Utility::UpdateMin(x, left_most_vertical_x);
    Utility::UpdateMax(x, right_most_vertical_x);
  };

  // It's possible that the Q or QI line come too close to the CLK line, since
  // it's in the middle (vertically). since they are only one track apart, and
  // the CLK line has a couple of via encaps. So because I refuse to resort to
  // the RoutingGrid for this problem, we do a leetle tiny search for a jig of
  // appropriate length to avoid the known obstacles:
  geometry::ShapeCollection clk_net_shapes;
  for (geometry::Instance *memory : sorted_memories) {
    memory->CopyConnectableShapesOnNets(
        {absl::StrCat(memory->name(), ".CLK")}, &clk_net_shapes);
    LOG(INFO) << memory->name() <<  " at " << memory->lower_left().x();
  }


  for (size_t i = 0; i < sorted_memories.size(); ++i) {
    const GateAssignment &assignment = gate_assignments[i];
    geometry::Instance *memory = sorted_memories[i];
    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_QI = memory->GetFirstPortNamed("QI");

    std::string net_Q = absl::StrCat(memory->name(), ".Q");
    std::string net_QI = absl::StrCat(memory->name(), ".QI");

    const GateContacts &gate = assignment.gate;

    geometry::Point p_tab_centre = gate.p_contact;
    geometry::Point n_tab_centre = gate.n_contact;

    int64_t vertical_x = assignment.p_vertical_x;

    auto points = ConnectVertically(mem_Q->centre(),
                                    p_tab_centre,
                                    vertical_x - met2_pitch,
                                    layout,
                                    net_Q);

    update_bounds_fn(vertical_x - met2_pitch);

    // There should always be 4 points out of this function.
    geometry::Line p_tab_vertical_line(points[1], points[2]);

    auto encap = db.TypicalViaEncap(
        "mcon.drawing", "met1.drawing", "via1.drawing");

    geometry::Point target_centre = mem_QI->centre();

    auto via_with_margin_fn = [&](const geometry::Point &centre) {
      return geometry::Rectangle::CentredAt(
          centre, encap.length, encap.width).WithKeepout(
              db, "met1.drawing");
    };

    auto find_jig_fn = [&](
        const geometry::Line &test_line,
        bool port_is_left_of_x,
        int64_t start_x,
        int64_t y) {
      int64_t direction_coefficient = port_is_left_of_x ? -1 : 1;
      int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;
      int64_t jig = 0;

      geometry::Rectangle test_shape = via_with_margin_fn(
          {start_x + jig, y});
      size_t attempts = 0;
      while (clk_net_shapes.Overlaps(test_shape)) {
        if (attempts > 3) {
          LOG(ERROR) << "Could not jig enough to avoid obstacle";
          // Give up.
          break;
        }
        jig += direction_coefficient * met2_pitch;

        geometry::Line jig_line({start_x, y}, {start_x + jig, y});
        bool incident = false;
        geometry::Point intersection;
        if (jig_line.IntersectsInMutualBounds(
              test_line, &incident, &intersection)) {
          direction_coefficient *= -1;
          jig = 0;
          attempts = 0;
        }

        test_shape = via_with_margin_fn({start_x + jig, y});
        attempts++;
      }
      return jig;
    };

    int64_t jig_x = find_jig_fn(p_tab_vertical_line,
                                target_centre.x() < vertical_x,
                                vertical_x,
                                target_centre.y());

    if (jig_x != 0) {
      geometry::Point p0 = target_centre;
      geometry::Point p1 = {vertical_x + jig_x, target_centre.y()};
      geometry::Point p2 = {vertical_x, target_centre.y()};
      geometry::Point p3 = {vertical_x, n_tab_centre.y()};
      geometry::Point p4 = n_tab_centre;

      layout->MakeVia("mcon.drawing", p0);
      layout->MakeWire({p0, p1},
                       "met1.drawing",
                       "li.drawing",
                       "met2.drawing",
                       false,
                       false,
                       net_QI,
                       true);
      layout->MakeWire({p1, p2, p3},
                       "met2.drawing",
                       "met1.drawing",
                       "met1.drawing",
                       true,
                       false,
                       net_QI,
                       true);
      layout->MakeWire({p3, p4},
                       "met1.drawing",
                       "met2.drawing",
                       "li.drawing",
                       true,
                       false,
                       net_QI,
                       true);
      layout->MakeVia("mcon.drawing", p4);
    } else {
      ConnectVertically(target_centre,
                        n_tab_centre,
                        vertical_x,
                        layout,
                        net_QI);
    }

    update_bounds_fn(vertical_x);

    AddPolyconAndLi(p_tab_centre, true, layout);
    AddPolyconAndLi(n_tab_centre, false, layout);

    connect_memory_to_control_fn(memory, gate.number, true);
    connect_memory_to_control_fn(memory, gate.number, false);
  }
  return true;
}

std::vector<Sky130InterconnectMux2::GateAssignment>
Sky130InterconnectMux2::AssignRow(
    const std::vector<geometry::Instance*> &sorted_memories,
    int64_t max_offset_from_first_poly_x,
    std::vector<GateContacts> *gates) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch;

  // Assign gates to memories in the row and test for conflict.
  std::vector<GateAssignment> gate_assignments;

  for (size_t i = 0; i < sorted_memories.size(); ++i) {
    geometry::Instance *memory = sorted_memories[i];
    geometry::Port *mem_Q = memory->GetFirstPortNamed("Q");
    geometry::Port *mem_QI = memory->GetFirstPortNamed("QI");

    int64_t port_average_x = (mem_Q->centre().x() + mem_QI->centre().x()) / 2;

    const GateContacts &gate = *std::min_element(
        gates->begin(), gates->end(),
        [&](const GateContacts &lhs, const GateContacts &rhs) {
          int64_t lhs_distance = std::abs(lhs.p_contact.x() - port_average_x);
          int64_t rhs_distance = std::abs(rhs.p_contact.x() - port_average_x);
          return lhs_distance < rhs_distance;
        });

    LOG(INFO) << "gate " << gate.number << " is closest to memory ports x="
              << port_average_x;
    int64_t vertical_x = gate.p_contact.x() + max_offset_from_first_poly_x;

    GateAssignment assignment {
      .gate = gate,
      .p_vertical_x = vertical_x,
      .p_gate_x = mem_QI->centre().x(),
      .n_vertical_x = vertical_x - met2_pitch,
      .n_gate_x = mem_Q->centre().x()
    };
    gate_assignments.push_back(assignment);

    gates->erase(std::remove_if(gates->begin(), gates->end(),
                                    [&](const GateContacts &entry) {
                                      return entry.number == gate.number;
                                    }),
                     gates->end());
  }
  return gate_assignments;
}

std::optional<std::vector<std::vector<Sky130InterconnectMux2::GateAssignment>>>
Sky130InterconnectMux2::FindGateAssignment(
    const std::vector<geometry::Instance*> scan_order,
    size_t num_rows,
    size_t num_columns,
    int64_t max_offset_from_first_poly_x,
    std::vector<std::vector<geometry::Instance*>> *sorted_memories_per_row,
    std::vector<GateContacts> *gates) const {
  auto row_has_conflict_fn = [](
      const std::vector<GateAssignment> &assignments) -> bool {
    std::vector<geometry::Line> p_tests;
    std::vector<geometry::Line> n_tests;
    for (const auto &assignment : assignments) {
      p_tests.emplace_back(
          geometry::Point{assignment.p_gate_x, 0},
          geometry::Point{assignment.p_vertical_x, 0});
      n_tests.emplace_back(
          geometry::Point{assignment.n_gate_x, 0},
          geometry::Point{assignment.n_vertical_x, 0});
    }
    for (size_t i = 0; i < p_tests.size(); ++i) {
      for (size_t j = 0; j < p_tests.size(); ++j) {
        if (i <= j) {
          continue;
        }
        geometry::Line &a_p = p_tests[i];
        geometry::Line &b_p = p_tests[j];
        bool incident;
        geometry::Point point;
        if (a_p.IntersectsInMutualBounds(b_p, &incident, &point)) {
          return true;
        }

        geometry::Line &a_n = n_tests[i];
        geometry::Line &b_n = n_tests[j];
        if (a_n.IntersectsInMutualBounds(b_n, &incident, &point)) {
          return true;
        }
      }
    }
    return false;
  };

  // We need to permute the order in which we allocate the rows, because
  // sometimes there are conflicts!
  std::vector<int> row_order;
  for (int r = 0; r < num_rows; r++)
    row_order.push_back(r);

  std::vector<std::vector<geometry::Instance*>> memories_per_row;
  // The initial row order is the natural one: 0, 1, 2
  for (int r : row_order) {
    std::vector<geometry::Instance*> row_memories;
    for (int i = 0; i < num_columns; ++i) {
      size_t c = num_columns * r + i;
      if (c >= scan_order.size()) {
        break;
      }
      geometry::Instance *memory = scan_order[c];
      row_memories.push_back(memory);
    }
    // Sort memories in the row by increasing x position. Earlier entries are
    // now further to the left for sure.
    std::sort(row_memories.begin(),
              row_memories.end(),
              CompareInstancesByQPortX);
    memories_per_row.push_back(row_memories);
  }
  *sorted_memories_per_row = memories_per_row;

  auto check_permutation_fn = [&](
      const std::vector<int> &row_order,
      std::vector<std::vector<GateAssignment>> *assignments_out) -> bool {
    std::vector<GateContacts> gates_copy(gates->begin(), gates->end());
    std::vector<std::vector<GateAssignment>> all_assignments(
        row_order.size(), std::vector<GateAssignment>());
    for (int r : row_order) {
      std::vector<GateAssignment> assignments = AssignRow(
          memories_per_row[r], max_offset_from_first_poly_x, &gates_copy);
      if (row_has_conflict_fn(assignments)) {
        return false;
      }
      all_assignments[r] = assignments;
    }
    *gates = gates_copy;
    *assignments_out = all_assignments;
    return true;
  };

  do {
    std::vector<std::vector<GateAssignment>> assignments;
    bool success = check_permutation_fn(row_order, &assignments);
    if (!success) {
      LOG(INFO) << "Failed: Assignment in row order: "
                << absl::StrJoin(row_order, ", ");
      continue;
    }
    LOG(INFO) << "OK: Assignment in row order: "
              << absl::StrJoin(row_order, ", ");
    return assignments;
  } while (std::next_permutation(row_order.begin(), row_order.end()));
  return std::nullopt;
}


void Sky130InterconnectMux2::ConnectControlWiresWithEffort(
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
    Circuit *circuit) const {

  std::vector<std::vector<geometry::Instance*>> sorted_memories_per_row;
  auto assignment = FindGateAssignment(scan_order,
                                       num_rows,
                                       num_columns,
                                       max_offset_from_first_poly_x,
                                       &sorted_memories_per_row,
                                       gates);
  if (!assignment) {
    LOG(ERROR) << "Could not find gate assignment";
    return;
  }

  for (size_t r = 0; r < num_rows; ++r) {
    ConnectMemoryRowToStack(sorted_memories_per_row[r],
                            (*assignment)[r],
                            max_offset_from_first_poly_x,
                            gates,
                            stack,
                            left_most_vertical_x,
                            right_most_vertical_x,
                            memory_output_nets,
                            layout,
                            circuit);
  }
}

bool Sky130InterconnectMux2::VerticalWireWouldCollideWithOthers(
    const std::string &net,
    int64_t vertical_x,
    int64_t first_y,
    int64_t second_y,
    Layout *layout) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &met2_rules = db.Rules("met2.drawing");

  geometry::ShapeCollection same_net_shapes;
  layout->CopyConnectableShapesOnNets({net}, &same_net_shapes);
  same_net_shapes.KeepOnlyLayers({db.GetLayer("met2.drawing")});

  LOG(INFO) << same_net_shapes.Describe();

  auto encap_info = db.TypicalViaEncap("met2.drawing", "via1.drawing");
  geometry::Rectangle bottom_encap = geometry::Rectangle::CentredAt(
      {vertical_x, first_y},
      encap_info.width,
      encap_info.length);
  geometry::Rectangle top_encap = geometry::Rectangle::CentredAt(
      {vertical_x, second_y},
      encap_info.width,
      encap_info.length);

  geometry::Point first = {vertical_x, first_y};
  geometry::Point second = {vertical_x, second_y};

  geometry::PolyLine wire = geometry::PolyLine({first, second});
  wire.SetWidth(met2_rules.min_width);
  wire.set_min_separation(met2_rules.min_separation);
  wire.InsertBulge(first, encap_info.width, encap_info.length);
  wire.InsertBulge(second, encap_info.width, encap_info.length);

  PolyLineInflator inflator(db);
  std::optional<geometry::Polygon> polygon = inflator.InflatePolyLine(wire);
  polygon->Fatten(met2_rules.min_separation - 1);

  for (const auto &other : same_net_shapes.polygons()) {
    if (polygon->Overlaps(*other)) {
      LOG(INFO) << "Collision! " << *polygon << " intersects " << *other;
      return true;
    }
  }
  return false;
}

// IT SURE WOULD BE NICE to connect the transmission gate outputs to the output
// buffers on the bottom-most connection layer, li.drawing. This requires a
// slight shift to the contents of either the transmission gate stack cell, or
// the adjacent buffer cell, to squeeze another li.drawing wire in between.
// However, one of the output buffers will still have to be connected my
// met1/met2, so it doesn't seem like that much of a win. (It might make the
// single-output version nicer.)
void Sky130InterconnectMux2::DrawOutput(
    const std::vector<geometry::Instance*> &output_buffers,
    geometry::Instance *stack,
    int64_t *mux_pre_buffer_y,
    int64_t output_port_x,
    Layout *layout,
    Circuit *circuit) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  const auto &met2_rules = db.Rules("met2.drawing");

  LOG_IF(FATAL, NumOutputs() != output_buffers.size())
      << "We expect as many output buffers are there are output ports.";

  // Assign left-most mux output to left-most output buffer, and right-most
  // output to right-most output buffer.
  for (int i = 0; i < NumOutputs(); ++i) {
    // First connect the mux output to its respective buffer.
    geometry::Instance *buf = output_buffers[i];
    geometry::Port *buf_A = buf->GetFirstPortNamed("A");

    std::string stack_output_name = absl::StrCat(kStackOutputName, i);
    geometry::Port *ff_out = stack->GetNearestPortNamed(
        *buf_A, stack_output_name);

    std::string mid_net_name = absl::StrFormat(
        "%s_to_%s.%s", stack_output_name, buf->name(), "A");

    int64_t mid_y_level = ff_out->centre().y() + i * met2_rules.min_pitch;

    *mux_pre_buffer_y = mid_y_level;

    geometry::Point p0 = {ff_out->centre().x(), mid_y_level};
    geometry::Point p2 = buf_A->centre();
    geometry::Point p1 = {p2.x(), mid_y_level};

    layout->MakeAlternatingWire(
        {p0, p1, p2}, "met1.drawing", "met2.drawing", mid_net_name, true);

    layout->MakeVia("mcon.drawing", p0);

    LOG(INFO) << "Connecting " << ff_out << " to " << *buf_A;
    layout->MakeVia("mcon.drawing", p2, mid_net_name);
    const auto &encap = db.TypicalViaEncap(
        "via1.drawing", "met1.drawing", "mcon.drawing");
    layout->MakeVia("via1.drawing", p2, mid_net_name);
    layout->MakeViaEncap("met1.drawing", "mcon.drawing", "via1.drawing", p2);

    // Update circuit.
    circuit::Wire stack_to_buf = circuit->AddSignal(mid_net_name);
    stack->circuit_instance()->Connect(stack_output_name, stack_to_buf);
    buf->circuit_instance()->Connect("A", stack_to_buf);

    // Now connect buffer to output pin.
    geometry::Port *buf_X = buf->GetFirstPortNamed("X");

    std::string out_net_name = absl::StrCat("OUT", i);

    // ¯\_(ツ)_/¯
    int64_t out_y_level = mid_y_level +
        (i % 2 == 0 ? -1 : 1) * (i + 1) * met2_rules.min_pitch;

    p0 = buf_X->centre();
    p1 = {buf_X->centre().x(), out_y_level};
    p2 = {output_port_x, out_y_level};

    // TODO(another micro-optimisation to this layout would be to only use
    // met1: either by specialising the output path for each buffer, or by
    // alternating the buffer geometry.
    layout->MakeAlternatingWire({p0, p1, p2},
                                "met2.drawing",
                                "met1.drawing",
                                out_net_name,
                                true,     // Yes connectable.
                                true,     // Yes start encap.
                                false);   // No end encap.

    layout->MakePin(out_net_name, p2, "met1.pin");

    circuit::Wire output_signal = circuit->AddSignal(out_net_name);
    circuit->AddPort(output_signal);
    buf->circuit_instance()->Connect("X", output_signal);

    // To keep VLSIR happy, connect port P to a floating net (it is disconnected).
    // TODO(aryap): This should be automatically emitted by our circuit model for
    // explicitly disconnected ports!
    circuit::Wire disconnected_P = circuit->AddSignal(
        absl::StrCat("disconnected_P", i));
    buf->circuit_instance()->Connect("P", disconnected_P);
  }
}

void Sky130InterconnectMux2::DrawPowerAndGround(
    const MemoryBank &bank,
    int64_t start_column_x,
    Layout *layout,
    Circuit *circuit) const {
  // TODO: Implement dual-output specific power/ground routing
  Sky130InterconnectMux1::DrawPowerAndGround(
      bank, start_column_x, layout, circuit);
}

void Sky130InterconnectMux2::DrawClock(
    const MemoryBank &bank,
    const std::vector<geometry::Instance*> &top_memories,
    const std::vector<geometry::Instance*> &bottom_memories,
    const std::vector<geometry::Instance*> &clk_bufs,
    int64_t input_clk_x,
    int64_t clk_x,
    int64_t clk_i_x,
    Layout *layout,
    Circuit *circuit) const {
  // TODO: Implement dual-output specific clock routing
  Sky130InterconnectMux1::DrawClock(
      bank, top_memories, bottom_memories, clk_bufs,
      input_clk_x, clk_x, clk_i_x, layout, circuit);
}

}  // namespace atoms
}  // namespace bfg
