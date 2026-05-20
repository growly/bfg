#ifndef ATOMS_SKY130_SPLIT_BUFFER_H_
#define ATOMS_SKY130_SPLIT_BUFFER_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

#include "../geometry/rectangle.h"
#include "sky130_simple_transistor.h"
#include "proto/parameters/sky130_split_buffer.pb.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generates an inverter and a buffer in parallel from the output:
//
// A ---+--->o------ P = ~A
//      |
//      +--->o-->o-- X = ~~A
//
// The purpose of this cell is to try and produce a signal and its complement
// roughly in sync. This is useful for things like transmission gates.
//
// The circuit is:
//                                        
//                 /               /                /
//                 |               |                |
//            g   _| s        g   _| s         g   _| s
//            +-o|_ pfet_1    +-o|_  pfet_2a   +-o|_  pfet_2b
//            |    | d        |    | d         |    | d
//         +--+    +-----Xb---+----|-----------+    |
//         |  |   _| d        |    +-----------|----+--- X
//         |  +--|_ nfet_1    |    |           |    |
//         |  g    | s        |   _| d         |   _| d
//         |       |          +--|_  nfet_2a   +--|_  nfet_2b
//         |       V          g    | s         g    | s
//         |                       |                |
//     A --+                       V                V
//         |
//         |         /                /
//         |         |                |
//         |    g   _| s         g   _| s
//         |    +-o|_  pfet_0a   +-o|_  pfet_0b
//         |    |    | d         |    | d
//         +----+----|-----------+    |
//              |    +-----------|----+--- P
//              |    |           |    |
//              |   _| d         |   _| d
//              +--|_  nfet_0a   +--|_  nfet_0b
//              g    | s         g    | s
//                   |                |
//                   V                V
//
// The layout should be straightforward. On the right hand side of the cell we
// put the top branch, with nfet1/2a and optionally nfet2b:
//
//                | V     DD |                           |VDD|
// +--------------|          |--------------+------------|   |+
// |              +--     ---+              |            +---+|
// | pfet_1               | pfet_2a         | pfet_2b         |
// |      +---+           |      +---+      |      +---+      |
// +------|   |------     +------|   |------+------|   |------+
//        |   |                  |   +-------------+   |
//        |   |                  |                     |
//        |   |                  |   +-------------+   |
// +------|   |------     +------|   |------+------|   |------+
// |      +---+           |      +---+      |      +---+      |
// | nfet_1               | nfet_2a         | nfet_2b         |
// |              +--     ---+              |            +---+|
// +--------------|          |--------------+------------|   |+
//                |VGND |                                |VGND
//
// On the left we put the bottom branch, which is optionally double-wide.
//
//  |   |                         |VDD     |
// +|VDD|------------+------------|        |------------+
// |+---+            |            +--------+
// | pfet_0b         | pfet_0a         | pfet_1
// |      +---+      |      +---+      |
// +------|   |------+------|   |------+-----------------
//        |   +-------------+   |
//        |                     |
//        |   +-------------+   |
// +------|   |------+------|   |------+-----------------
// |      +---+      |      +---+      
// | nfet_0a         | nfet_0b         
// |+---+            |            +-------+
// +|VGND------------+------------|       |
//  |   |                         | VGND  |
//
//
class Sky130SplitBuffer: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t height_nm = 2720;
    std::optional<uint64_t> unit_width_nm;

    // FIXME(aryap): We currently have a problem when p/nfet 2a/b widths total
    // more than 1600 nm. The Xb connection to their gate poly needs to be moved
    // I think, so that the poly encap of licon is sufficient.
    uint64_t nfet_0_width_nm = 600;
    uint64_t nfet_1_width_nm = 600;
    uint64_t nfet_2_width_nm = 600;
    uint64_t pfet_0_width_nm = 1000;
    uint64_t pfet_1_width_nm = 1000;
    uint64_t pfet_2_width_nm = 1000;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;

    bool double_fet0 = true;
    bool double_fet2 = true;

    // This is db.Rules("poly.drawing").min_separation / 2.
    uint64_t min_poly_boundary_separation_nm = 105;

    // TODO(aryap): Diff extension from poly has a minimum value, so be careful
    // reducing the poly pitch. Sky130SimpleTransistor will only draw minimum
    // value and you will get an overlapping diff region. Either enable override
    // of the overlap or explicitly ignore it somehow.
    uint64_t split_poly_pitch_nm = 420;
    uint64_t poly_pitch_nm = 440;

    bool draw_overflowing_vias_and_pins = true;
    bool draw_vpwr_vias = true;
    bool draw_vgnd_vias = true;
    uint64_t mcon_via_pitch_nm = 460;
    bool expand_wells_to_horizontal_bounds = true;

    // Assumes an offset of half the given pitch from y = 0. A value of 0
    // disables.
    int64_t snap_ports_to_vertical_pitch_nm = 340;

    void ToProto(proto::parameters::Sky130SplitBuffer *pb) const;
    void FromProto(const proto::parameters::Sky130SplitBuffer &pb);
  };

  Sky130SplitBuffer(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {

    SetUpTransistors();
  }

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  struct TransistorAndViaLocation {
    Sky130SimpleTransistor *transistor;
    Sky130SimpleTransistor::ViaPosition location;
  };

  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  void SetUpTransistors();

  int64_t FirstPolyX() const;
  int64_t MapToTrackY(int64_t target_y, int64_t add_pitches) const;

  void DrawPolyA(geometry::Point *polycon_via, bfg::Layout *layout) const;
  void DrawPolyXb(geometry::Point *polycon_via, bfg::Layout *layout) const;
  std::vector<geometry::Point> AssemblePoly(
      std::vector<std::pair<geometry::Point, uint64_t>> tops,
      std::vector<std::pair<geometry::Point, uint64_t>> bottoms,
      int64_t add_left,
      geometry::Point *mid_left) const;

  std::optional<geometry::Point> AverageViaLocations(
      const std::vector<TransistorAndViaLocation> &lookups) const;

  std::unique_ptr<Sky130SimpleTransistor> nfet_0a_gen_;
  std::unique_ptr<Sky130SimpleTransistor> nfet_0b_gen_;
  std::unique_ptr<Sky130SimpleTransistor> nfet_1_gen_;
  std::unique_ptr<Sky130SimpleTransistor> nfet_2a_gen_;
  std::unique_ptr<Sky130SimpleTransistor> nfet_2b_gen_;

  std::unique_ptr<Sky130SimpleTransistor> pfet_0a_gen_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_0b_gen_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_1_gen_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_2a_gen_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_2b_gen_;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SPLIT_BUFFER_H_
