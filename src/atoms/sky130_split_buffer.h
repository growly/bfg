#ifndef ATOMS_SKY130_SPLIT_BUFFER_H_
#define ATOMS_SKY130_SPLIT_BUFFER_H_

#include <cstdint>

#include "atom.h"
#include "sky130_parameters.h"
#include "../circuit.h"
#include "../layout.h"

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
//                               /               /                /
//                               |               |                |
//                          g   _| s        g   _| s         g   _| s
//                          +-o|_ pfet_1    +-o|_  pfet_2a   +-o|_  pfet_2b
//                          |    | d        |    | d         |    | d
//                       +--+    +-----Xb---+----|-----------+    |
//                       |  |   _| d        |    +-----------|----+--- X
//                       |  +--|_ nfet_1    |    |           |    |
//                       |  g    | s        |   _| d         |   _| d
//                       |       |          +--|_  nfet_2a   +--|_  nfet_2b
//                       |       V          g    | s         g    | s
//                       |                       |                |
//                   A --+                       V                V
//                       |
//                       |         /                /
//                       |         |                |
//                       |    g   _| s         g   _| s
//                       |    +-o|_  pfet_0a   +-o|_  pfet_0b
//                       |    |    | d         |    | d
//                       +----+----|-----------+    |
//                            |    +-----------|----+--- P
//                            |    |           |    |
//                            |   _| d         |   _| d
//                            +--|_  nfet_0a   +--|_  nfet_0b
//                            g    | s         g    | s
//                                 |                |
//                                 V                V
//
// The layout should be straightforward. On the right hand side of the cell we
// put the top branch, with nfet1/2a and optionally nfet2b:
//
//                | VDD |                           |VDD|
// +--------------|     |--------------+------------|   |+
// |              +-----+              |            +---+|
// | pfet_1          | pfet_2a         | pfet_2b         |
// |      +---+      |      +---+      |      +---+      |
// +------|   |------+------|   |------+------|   |------+
//        |   |             |   +-------------+   |
//        |   |             |                     |
//        |   |             |   +-------------+   |
// +------|   |------+------|   |------+------|   |------+
// |      +---+      |      +---+      |      +---+      |
// | nfet_1          | nfet_2a         | nfet_2b         |
// |              +-----+              |            +---+|
// +--------------|     |--------------+------------|   |+
//                |VGND |                           |VGND
//
// On the left we put the bottom branch, which is optionally double-wide.
//
class Sky130SplitBuffer: public Atom {
 public:
  struct Parameters : public Sky130Parameters {
    uint64_t width_nm = 1380;
    uint64_t height_nm = 2720;

    uint64_t nfet_0_width_nm = 520;
    uint64_t nfet_1_width_nm = 520;
    uint64_t nfet_2_width_nm = 520;
    uint64_t pfet_0_width_nm = 790;
    uint64_t pfet_1_width_nm = 790;
    uint64_t pfet_2_width_nm = 790;

    uint64_t nfet_0_length_nm = 150;
    uint64_t nfet_1_length_nm = 150;
    uint64_t nfet_2_length_nm = 150;
    uint64_t pfet_0_length_nm = 150;
    uint64_t pfet_1_length_nm = 150;
    uint64_t pfet_2_length_nm = 150;

    bool double_fet0 = true;
    bool double_fet2 = true;

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
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  void SetUpTransistors();

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
