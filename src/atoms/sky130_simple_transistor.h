#ifndef ATOMS_SKY130_SIMPLE_TRANSISTOR_H_
#define ATOMS_SKY130_SIMPLE_TRANSISTOR_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/compass.h"
#include "../physical_properties_database.h"

namespace bfg {

class DesignDatabase;

namespace atoms {

// TODO(aryap): Is this a good idea? Or do we just a want a more general helper?
// Abstracting the transistors is clearly a good idea sometimes.
// TODO(aryap): There is basic functionality here that is broadly PDK agnostic.
// It will be worth factoring out eventually.
class Sky130SimpleTransistor : public Atom {
 public:
  struct Parameters {
    enum FetType {
      PMOS,
      PMOS_HVT,
      PMOS_LVT,
      NMOS,
      NMOS_HVT,
      NMOS_LVT
    };

    FetType fet_type = FetType::NMOS;
    uint64_t length_nm = 150;
    uint64_t width_nm = 500;
    bool stacks_left = false;
    bool stacks_right = false;
  };

  std::string DiffLayer() const;
  std::string DiffConnectionLayer() const;

  int64_t TransistorWidth() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  }

  int64_t TransistorLength() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.length_nm);
  }

  int64_t GetDiffWing(const geometry::Compass &direction) const;

  Sky130SimpleTransistor(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout();
  bfg::Circuit *GenerateCircuit();

 private:
  Parameters parameters_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SIMPLE_TRANSISTOR_H_

