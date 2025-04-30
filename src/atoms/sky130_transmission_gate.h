#ifndef ATOMS_SKY130_TRANSMISSION_GATE_H_
#define ATOMS_SKY130_TRANSMISSION_GATE_H_

#include <cstdint>
#include <memory>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/compass.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../physical_properties_database.h"
#include "sky130_simple_transistor.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

class Sky130TransmissionGate : public Atom {
 public:
  struct Parameters {
    uint64_t p_width_nm = 1000;
    uint64_t p_length_nm = 150;
    uint64_t n_width_nm = 650;
    uint64_t n_length_nm = 150;

    bool stacks_left = false;
    bool stacks_right = false;

    std::optional<uint64_t> cell_height_nm = 2720; //std::nullopt;

    std::optional<uint64_t> height_divisor_nm;
    std::optional<uint64_t> stacking_pitch_nm;

    bool draw_nwell = false;

    std::optional<geometry::Compass> p_tab_position;
    std::optional<geometry::Compass> n_tab_position;
  };

  Sky130TransmissionGate(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {
    // There's a PMOS transistor and there's an NMOS transistor.
    Sky130SimpleTransistor::Parameters nfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
      .width_nm = parameters_.n_width_nm,
      .length_nm = parameters_.n_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
      .stacking_pitch_nm = parameters_.stacking_pitch_nm
    };

    nfet_generator_.reset(new Sky130SimpleTransistor(nfet_params, design_db_));

    Sky130SimpleTransistor::Parameters pfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
      .width_nm = parameters_.p_width_nm,
      .length_nm = parameters_.p_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
      .stacking_pitch_nm = parameters_.stacking_pitch_nm
    }; 
 
    pfet_generator_.reset(new Sky130SimpleTransistor(pfet_params, design_db_));
  }

  const geometry::Rectangle PMOSBounds() const {
    return pfet_generator_->DiffBounds();
  }
  const geometry::Rectangle NMOSBounds() const {
    return nfet_generator_->DiffBounds();
  }

  const Sky130SimpleTransistor &pfet_generator() const {
    return *pfet_generator_;
  }
  const Sky130SimpleTransistor &nfet_generator() const {
    return *nfet_generator_;
  }

  // Adds a tab to the poly for a via there:
  //
  //   +--------+
  //   |   A    |  < tab to the top left
  //   +--------+
  //      |     |
  // -----|     |-----
  //      |     |     
  //      |     |     
  //      |     |     
  //      |     |     
  //      |     |     
  // -----|     |-----
  //      |     |
  //      +--------+
  //      |    B   |  < tab to the bottom right
  //      +--------+
  //
  // Returns the point at centre of where the tab will be (connection point for
  // via).
  // FIXME(aryap): Making this a function of the transistor class itself adds
  // complexity to its contract and removes flexibility. If it's adding
  // convenience, it's not clear what that is yet. So move this out and maybe
  // readd it if necessary later.
  geometry::Rectangle *AddPolyTab(const Sky130SimpleTransistor &fet_generator,
                                  const geometry::Compass &position,
                                  Layout *layout);

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout();

  bfg::Circuit *GenerateCircuit();

 private:
  bool PMOSHasLowerTab() const;
  bool NMOSHasLowerTab() const;

  int64_t FigureCellHeight() const;
  int64_t PMOSPolyHeight() const;
  int64_t NMOSPolyHeight() const;
  // The tab will be a horizontal rectangle, whose height and width must
  // accommodate a via on the DiffConnectionLayer().
  int64_t PolyTabHeight(const Sky130SimpleTransistor &fet_generator) const;
  int64_t PolyTabWidth(const Sky130SimpleTransistor &fet_generator) const;
  int64_t NMOSPolyTabHeight() const {
    return PolyTabHeight(*nfet_generator_);
  }
  int64_t NMOSPolyTabWidth() const {
    return PolyTabWidth(*nfet_generator_);
  }
  int64_t PMOSPolyTabHeight() const {
    return PolyTabHeight(*pfet_generator_);
  }
  int64_t PMOSPolyTabWidth() const {
    return PolyTabWidth(*pfet_generator_);
  }

  Parameters parameters_;

  std::unique_ptr<Sky130SimpleTransistor> nfet_generator_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_generator_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_TRANSMISSION_GATE_H_

