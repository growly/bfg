#ifndef TILES_LUT_B_H_
#define TILES_LUT_B_H_

#include <map>
#include <utility>
#include <string>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "tile.h"

#include "../geometry/compass.h"
#include "../geometry/group.h"
#include "../design_database.h"
#include "../row_guide.h"
#include "../routing_path.h"
#include "../memory_bank.h"
#include "../layout.h"

#include "proto/parameters/lut_b.pb.h"

namespace bfg {

class Cell;
class Layout;
class Circuit;
class RoutingGrid;

namespace geometry {

class Instance;

}  // namespace geometry

namespace tiles {

// Ideally this is what we have:
//                +-----------+
// LUT        k   |           |           |\        Combinational out,
// select   --/---|   k-LUT   |--+--------| |-------------- O
// S_k, S_k-1...  |           |  |   +----| |
//                +-----------+  |   |    |/
//                               |   |
//                               |   |
// Bypass                        |   |    |\   +-----+  Registered out,
// input, X  --------------------|---+----| |--| FF  |---- Q
//                               +--------| |  |>    |
//                                        |/   +-----+
//
// If add_s2_input_mux is set:       If add_s3_input_mux is set:
//                                   
//                 +-----------+                     +-----------+
// LUT        S0,1 |           |     LUT        S0,1 |           |
// select   ---/---|   k-LUT   |--   select   ---/---|   k-LUT   |--
//            +----|           |           S2 -------|           |
//       S3 --|----|           |                +----|           |
//            |    +-----------+                |    +-----------+
//       |\   |                            |\   |
// S2_A -| |--+                      S3_A -| |--+
// S2_B -| |                         S3_B -| |
//       |/                                |/ 
//        |                                 |
//       S2_S                              S3_S
//
// If add_third_input_to_output_muxes is set, conceptually:
//
//
//                                        +-----+
//                                        | Mem |-+
//                                        |>    | |
//                                        +-----+ |
//                +-----------+                   |
// LUT        k   |           |           |\     |\ Combinational out,
// select   --/---|   k-LUT   |--+--------| |----| |---------- O
// S_k, S_k-1...  |           |  |   +----| |  +-| |
//                +-----------+  |   |    |/   | |/    
//                               |   |         |
//                               |   | +-----+ |      
//                               |   | | Mem |----+
//                               |   | |>    | |  |
//                               |   | +-----+ |  |
//                               |   |         |  |
// Bypass                        |   |    |\   | |\    +-----+  Registered out,
// input, X  --------------------|---+----| |--|-| |---| FF  |---- Q
//                               +--------| |  +-| |   |>    |
//                                        |/   | |/    +-----+
//                                             |
//                                             |
//                                             X2
//
// But actually this:
//                                        +-----+
//                                        | Mem |-+
//                                        |>    | |
//                                        +-----+ |
//                +-----------+                   |
// LUT        k   |           |           |\     |\ Combinational out,
// select   --/---|   k-LUT   |--+--------| |----| |---------- O
// S_k, S_k-1...  |           |  |   +----| |  +-| |
//                +-----------+  |   |    |/   | |/    
//                               |   |         |
//                     +-----+   |   |         |
//                     | Mem |---|---|----+    |
//                     |>    |   |   |    |    |
//                     +-----+   |   |    |    |
//                               |   |    |    |  
//                               |   |    |    | |\    +-----+  Registered out,
//                               +---|-----------| |---| FF  |---- Q
//                                   |    |  +---| |   |>    |
//         Bypass                    |   |\  | | |/    +-----+
//         input, X  ----------------+---| |-+ |
//                                     +-| |   |
//                                     | |/    |
//                                     +-------+
//                                     |
//                                     X2
//
// Where X2 is typically used to connect the sum output from a carry-chain
// block.
//
// Note that with this additional logic, we have something closer to what Xilinx
// calls a "Configurable Logic Block".
class LutB : public Tile {
 public:
  // A k-LUT is made up of:
  //  - 2^k memories
  //  - k input buffers
  //  - 1 output buffer
  //  - (2^k / 8 = 2^k / 2^3 =) 2^(k - 3) 8:1 passive muxes
  //  - Enough passive or active muxes to reduce the outputs of the passive
  //  muxes
  //
  // We build the LUT by arranging memories and buffer cells in rows around a
  // central grid of active muxes, since they are large. An easy shorthand for
  // the arrangment is to consider the queue of each part and assign them to
  // rows by index, where rows are indexed from 0 (bottom). We do this once for
  // the bank of rows on the left side and once for the bank of rows on the
  // right.
  //
  // TODO(aryap): Imagine a shorthand that was like this:
  //  bank:
  //    row: memory2, memory3, buf1, buf2
  //    row: memory1, buf0
  //    row: memory0
  // Would that be good?
  struct BankArrangement {
    // constexpr comes to std::vector in C++20 but we don't have it yet.
    std::vector<size_t> memory_rows;
    std::vector<size_t> buffer_rows;
    std::vector<size_t> active_mux2_rows;
    std::vector<size_t> clk_buf_rows;
    geometry::Compass horizontal_alignment;
    geometry::Compass strap_alignment;
    bool alternate_rotation;
  };

  struct LayoutConfig {
    BankArrangement left;
    BankArrangement right;
    int64_t mux_area_horizontal_padding;
    int64_t mux_area_vertical_min_padding;
    size_t mux_area_rows;
    size_t mux_area_columns;
  };

  struct Parameters {
    // This is k.
    uint32_t lut_size = 4;
  
    bool add_s2_input_mux = false;

    // Pending a thorough redesign, you can only add an s3 input mux if you also
    // opt to add_third_input_to_output_muxes, below.
    bool add_s3_input_mux = true;
  
    bool add_third_input_to_output_muxes = false;

    bool split_scan_order = false;

    std::optional<uint64_t> tiling_width_unit_nm = 460;

    uint64_t default_row_height_nm = 2720;

    void ToProto(proto::parameters::LutB *pb) const;
    void FromProto(const proto::parameters::LutB &pb); 
  };

  LutB(const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters),
        s2_select_mux_(nullptr),
        s3_select_mux_(nullptr),
        aux_comb_output_mux_(nullptr),
        aux_comb_output_mux_config_(nullptr),
        aux_reg_output_mux_(nullptr),
        aux_reg_output_mux_config_(nullptr),
        comb_output_mux_(nullptr),
        comb_output_mux_config_(nullptr),
        reg_output_flop_(nullptr),
        reg_output_mux_(nullptr),
        reg_output_mux_config_(nullptr) {
    LOG_IF(WARNING,
        parameters_.add_s3_input_mux &&
        !parameters.add_third_input_to_output_muxes)
        << "add_s3_input_mux set but add_third_input_to_output_muxes is not; "
        << "no S3 mux will be added";
  }

  Cell *Generate() override;

 protected:
  struct PortKey {
    geometry::Instance *instance;
    std::string port_name;
  };
  struct PortKeyCollection {
    std::vector<PortKey> port_keys;
    std::optional<EquivalentNets> as_nets;
  };
  struct PortKeyAlias {
    PortKey key;
    std::string alias;
  };
  // TODO(aryap): This can be replaced with the more generic "PortKey" structs
  // above.
  struct AutoMemoryMuxConnection {
    geometry::Instance *source_memory;
    geometry::Instance *target_mux;
    std::string mux_port_name;
  };

  void AddClockAndPowerStraps(
      RoutingGrid *routing_grid, Circuit *circuit, Layout *layout) const;

  void ConfigureRoutingGrid(RoutingGrid *grid, Layout *layout) const;

  // Sets the scan_order_ based on memories_ and other participating flops.
  void SetScanOrder();

  std::optional<std::string> GetMemoryOutputNet(
      geometry::Instance *memory) const;

  void Route(Circuit *circuit, Layout *layout);
  void RouteClockBuffers(
      RoutingGrid *routing_grid, Circuit *circuit, Layout *layout);
  void RouteRemainder(
      RoutingGrid *routing_grid, Circuit *circuit, Layout *layout);
  void RouteMuxInputs(
      RoutingGrid *routing_grid,
      Circuit *circuit,
      Layout *layout);
  void RouteScanChain(
      RoutingGrid *routing_grid,
      Circuit *circuit,
      Layout *layout);
  void RouteInputs(
      RoutingGrid *routing_grid,
      Circuit *circuit,
      Layout *layout);
  void RouteOutputs(
      RoutingGrid *routing_grid,
      Circuit *circuit,
      Layout *layout);

  std::vector<std::set<geometry::Port*>> ResolvePortKeyCollection(
      const PortKeyCollection &collection) const;

  absl::StatusOr<std::vector<RoutingPath*>> AddMultiPointRoute(
      const PortKeyCollection &collection,
      RoutingGrid *routing_grid,
      Circuit *circuit,
      Layout *layout) const;

  // TODO(aryap): This feels like a nice general feature of the Layout class.
  // Ok I did that. It's called MakeVerticalSpineWithFingers. Use it.
  geometry::Group AddVerticalSpineWithFingers(
      const std::string &spine_layer_name,
      const std::string &via_layer_name,
      const std::string &finger_layer_name,
      const std::string &net,
      const std::vector<geometry::Point> &connections,
      int64_t spine_x,
      int64_t spine_width,
      Layout *layout) const;

  void FillDecapsRight(int64_t span, RowGuide *row);

  // TODO(aryap): Candidate for inclusion in base class.
  void AccumulateAnyErrors(const absl::Status &status);

  Parameters parameters_;

  static const LayoutConfig *GetLayoutConfiguration(size_t lut_size);

  static const std::pair<size_t, LayoutConfig> kLayoutConfigurations[];

  std::vector<MemoryBank> banks_;
  std::vector<geometry::Instance*> buf_order_;
  std::vector<geometry::Instance*> mux_order_;
  std::vector<geometry::Instance*> active_mux2s_;
  std::vector<geometry::Instance*> clk_buf_order_;
  std::vector<geometry::Instance*> memories_;

  // Contains all memories involved in the scan chain in the right order.
  std::vector<geometry::Instance*> scan_order_;

  // Sometimes features:
  geometry::Instance *s2_select_mux_;
  geometry::Instance *s3_select_mux_;

  geometry::Instance *aux_comb_output_mux_;
  geometry::Instance *aux_comb_output_mux_config_;
  geometry::Instance *aux_reg_output_mux_;
  geometry::Instance *aux_reg_output_mux_config_;

  // Features of all LutBs:
  geometry::Instance *comb_output_mux_;
  geometry::Instance *comb_output_mux_config_;
  geometry::Instance *reg_output_flop_;
  geometry::Instance *reg_output_mux_;
  geometry::Instance *reg_output_mux_config_;

  std::vector<absl::Status> errors_;

  std::map<geometry::Instance*, std::string> memory_output_net_names_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // TILES_LUT_B_H_
