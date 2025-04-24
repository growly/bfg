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

    bool draw_nwell = false;
  };

  enum Alignment {
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
    };

    nfet_generator_.reset(new Sky130SimpleTransistor(nfet_params, design_db_));

    Sky130SimpleTransistor::Parameters pfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
      .width_nm = parameters_.p_width_nm,
      .length_nm = parameters_.p_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
    }; 
 
    pfet_generator_.reset(new Sky130SimpleTransistor(pfet_params, design_db_));
  }

  const geometry::Rectangle PMOSBounds() const {
    return pfet_generator_->DiffBounds();
  }
  const geometry::Rectangle NMOSBounds() const {
    return nfet_generator_->DiffBounds();
  }

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout();

  bfg::Circuit *GenerateCircuit();

 private:
  Parameters parameters_;

  std::unique_ptr<Sky130SimpleTransistor> nfet_generator_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_generator_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_TRANSMISSION_GATE_H_

