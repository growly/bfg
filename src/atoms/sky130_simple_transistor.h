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
      // There are also higher-voltage types in Sky130. For now assume 1.8v.
    };

    FetType fet_type = FetType::NMOS;
    uint64_t width_nm = 500;
    uint64_t length_nm = 150;
    bool stacks_left = false;
    bool stacks_right = false;

    // If given, this forces the diff extension on stacking sides of the
    // transistor such that another transistor with the same stacking pitch
    // would have its poly this many nm away.
    std::optional<uint64_t> stacking_pitch_nm;

    // TODO(aryap): Should have the option of forcing the diff width either side
    // (left and right) of the poly, since that isn't standard in at least
    // Sky130.
  };

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

  enum Landmark {
    POLY_TOP_CENTRE,
    POLY_BOTTOM_CENTRE
  };

  static const std::map<ViaPosition, std::string> kSavedPointNameByViaPosition;

  Sky130SimpleTransistor(
      const Parameters &parameters, DesignDatabase *design_db)
      : Atom(design_db),
        parameters_(parameters),
        origin_({0, 0}),
        poly_y_min_(0),
        poly_y_max_(0),
        diff_y_min_(0),
        diff_y_max_(0) {
    ComputeGeometries();
  }

  // This makes sense as a feature of this class and not of Atoms, or Layouts,
  // in general, because the alignment points are meaningful only in the
  // context of a transistor like this. An alternative would be to label points
  // in the layout with names and align those. That would be a general
  // solution, but we can do better I think.
  void AlignTransistorPartTo(
      const Landmark &alignment, const geometry::Point &point);

  geometry::Point Locate(const Landmark &alignment);

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
  geometry::Point ViaLocation(const ViaPosition &via_position) const;

  geometry::Point PolyTopCentre() const;
  geometry::Point PolyBottomCentre() const;
  geometry::Point PolyLowerLeft() const;
  geometry::Point PolyUpperRight() const;

  // Computes the lower left point without generating the layout.
  geometry::Point LowerLeft() const;

  std::string DiffLayer() const;
  std::string DiffConnectionLayer() const;

  std::string PolyLayer() const { return "poly.drawing"; }
  std::string PolyConnectionLayer() const { return "licon.drawing"; }

  int64_t TransistorWidth() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  }

  int64_t TransistorLength() const {
    return design_db_->physical_db().ToInternalUnits(parameters_.length_nm);
  }

  uint64_t PolyHeight() const;

  int64_t DiffWing(const geometry::Compass &direction) const;

  // This actually just returns the diffusion rectangle, but because the layout
  // object isn't generated until GenerateLayout is called this should be
  // treated as copy of the bounds.
  const geometry::Rectangle DiffBounds() const;

  std::string CircuitCellName() const;

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout() {
    return GenerateLayout(nullptr, nullptr);
  }
  // If given, **poly and **diff will be pointed to the poly and diff shapes in
  // the layout, respectively.
  bfg::Layout *GenerateLayout(
      geometry::Polygon **poly, geometry::Rectangle **diff);
  bfg::Circuit *GenerateCircuit();
  const geometry::Point &origin() const { return origin_; }

 private:
  void ComputeGeometries();

  Parameters parameters_;

  // Defaults to (0, 0).
  geometry::Point origin_;

  // These are all extensions relative to the origin, and are poorly named. e.g.
  // 'poly_y_min_' is actually the offset from the origin_.y() point to the
  // bottom of the poly. TODO(aryap): Refactor, obviously.
  int64_t poly_y_min_;
  int64_t poly_y_max_;
  int64_t diff_y_min_;
  int64_t diff_y_max_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_SIMPLE_TRANSISTOR_H_

