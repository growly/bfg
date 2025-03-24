#ifndef ATOMS_SKY130_SIMPLE_TRANSISTOR_H_
#define ATOMS_SKY130_SIMPLE_TRANSISTOR_H_

#include <cstdint>

#include "atom.h"
#include "../circuit.h"
#include "../layout.h"
#include "../geometry/compass.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
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
    uint64_t width_nm = 500;
    uint64_t length_nm = 150;
    bool stacks_left = false;
    bool stacks_right = false;
  };

  enum Alignment {
    POLY_TOP_CENTRE,
    POLY_BOTTOM_CENTRE
  };

  // This makes sense as a feature of this class and not of Atoms, or Layouts,
  // in general, because the alignment points are meaningful only in the
  // context of a transistor like this. An alternative would be to label points
  // in the layout with names and align those. That would be a general
  // solution, but we can do better I think.
  void AlignTransistorPartTo(
      const Alignment &alignment, const geometry::Point &point);

  // Calculates locations of key via positions for convenience:
  //
  //             +-----+
  //             |  D  |
  // +-----------|     |------+
  // |     A     |     |      G
  // |           |     |      |
  // |     B     |  E  |      H
  // |           |     |      |
  // |     C     |     |      I
  // +-----------|     |------+
  //             |  F  |
  //             +-----+
  // A: Left diff, uppermost (shown not stacking)
  // B: Left diff, middle (shown not stacking)
  // C: Left diff, lowermost (shown not stacking)
  // D: Poly, uppermost
  // E: Poly, middle
  // F: Poly, lowermost
  // G: Right diff, uppermost (shown stacking)
  // H: Right diff, middle (shown stacking)
  // I: Right diff, lowermost (shown stacking)
  enum ViaPosition {
    LEFT_DIFF_UPPER,
    LEFT_DIFF_MIDDLE,
    LEFT_DIFF_LOWER,
    POLY_UPPER,
    POLY_MIDDLE,
    POLY_LOWER,
    RIGHT_DIFF_UPPER,
    RIGHT_DIFF_MIDDLE,
    RIGHT_DIFF_LOWER
  };
  geometry::Point ViaLocation(const ViaPosition &via_position) const;

  // Computes the lower left point without generating the layout.
  geometry::Point LowerLeft() const;

  std::string DiffLayer() const;
  std::string DiffConnectionLayer() const;

  int64_t TransistorWidth() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  }

  int64_t TransistorLength() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.length_nm);
  }

  uint64_t PolyHeight() const;

  int64_t DiffWing(const geometry::Compass &direction) const;

  Sky130SimpleTransistor(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters) {}

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout() {
    return GenerateLayout(nullptr, nullptr);
  }
  bfg::Layout *GenerateLayout(
      geometry::Polygon **poly, geometry::Rectangle **diff);
  bfg::Circuit *GenerateCircuit();

 private:
  Parameters parameters_;

  std::optional<Alignment> alignment_;
  std::optional<geometry::Point> alignment_point_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SIMPLE_TRANSISTOR_H_

