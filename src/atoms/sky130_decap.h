#ifndef ATOMS_SKY130_DECAP_H_
#define ATOMS_SKY130_DECAP_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"

#include "proto/parameters/sky130_decap.pb.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// Generate a decap cell in the style of sky130.
class Sky130Decap: public Atom {
 public:
  struct Parameters {
    // This is the width of the hd-lib std-cell decap_12. We can probably do
    // more with more vias and stuff.
    static constexpr uint64_t kMaxWidthNm = 5520;
    // This is the width of the std_cell decap_3. Any smaller than this and we
    // violate DRC rules (look at the poly).
    static constexpr uint64_t kMinWidthNm = 460;

    uint64_t width_nm = 1380;
    uint64_t height_nm = 2720;

    std::optional<uint64_t> nfet_0_width_nm;
    std::optional<uint64_t> pfet_0_width_nm;
    std::optional<uint64_t> nfet_0_length_nm;
    std::optional<uint64_t> pfet_0_length_nm;

    bool label_pins = true;

    // Add the mcon vias on the VPWR/VGND met1 rails. This can be helpful if
    // adjacent cells do not conform to being a multiple of a std. cell site
    // width and so the vias do not line up with those set by cells in adjacent
    // rows.
    bool draw_overflowing_vias_and_pins = true;

    // NOTE(aryap): Why is the pitch 460? Because that's what it is in all the
    // other cells, and if the vias don't overlap between cells perfectly other
    // tools will complain. Not sure how to make this value portable, so it is
    // configurable.
    // TODO(aryap): Put this in the params proto.
    uint64_t mcon_via_pitch = 460;

    // TODO(aryap): Maybe we do need a "Transistor" class (which subclasses
    // Instance)?
    std::string fet_model_length_parameter = "l";
    std::string fet_model_width_parameter = "w";

    std::string power_net = "VPWR";
    std::string ground_net = "VGND";

    void ToProto(proto::parameters::Sky130Decap *pb) const;
    void FromProto(const proto::parameters::Sky130Decap &pb);
  };

  Sky130Decap(const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}

  // Caller takes ownership!
  bfg::Cell *Generate() override;

 private:
  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

  // TODO(aryap): We should be able to come up with numbers for these if none
  // are given. Just find the max size for each given their mobility ratios.
  int64_t NDiffHeight() const;
  int64_t PDiffHeight() const;

  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_DECAP_H_
