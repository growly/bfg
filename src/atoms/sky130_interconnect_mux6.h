#ifndef ATOMS_SKY130_INTERCONNECT_MUX6_H_
#define ATOMS_SKY130_INTERCONNECT_MUX6_H_

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "proto/parameters/sky130_interconnect_mux6.pb.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// he mux6 we will build for interconnect (and the family of components like
// it) will look like:
//
//           +-+------------------------------+-+----+
//  Decap    |D|            FF                | | D  |
//  cells    +-+------------------------------+-+----+
//  creating |D|            FF                |T| <---- Tap
//  vertical +-+------------------------------+------+
//  routing  |D|            FF                | Buf  |  Clock Buffer
//  channel  +-+------------------------------+------+
//      -->  |D|  Transmission gate 6:1 Muxes | Buf  |  Output Buffer
//           | |                              |      |
//           +-+------------------------------+------+
//           |D|            FF                |T|    |
//           +-+------------------------------+-+----+
//           |D|            FF                |      |
//           +-+------------------------------+------+
//           |D|            FF                |  D   |
//           +-+------------------------------+------+

class Sky130InterconnectMux6 : public Atom {
 public:
  struct Parameters {
    std::optional<uint64_t> poly_pitch_nm = 600;

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
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_INTERCONNECT_MUX6_H_
