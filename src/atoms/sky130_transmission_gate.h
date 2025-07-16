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
#include "proto/parameters/sky130_transmission_gate.pb.h"
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

    std::optional<uint64_t> min_cell_height_nm;

    std::optional<uint64_t> vertical_tab_pitch_nm;
    std::optional<uint64_t> vertical_tab_offset_nm;
    
    std::optional<uint64_t> poly_pitch_nm;

    std::optional<uint64_t> min_p_tab_diff_separation_nm;
    std::optional<uint64_t> min_n_tab_diff_separation_nm;

    std::optional<uint64_t> min_poly_boundary_separation_nm;

    bool tabs_should_avoid_nearest_vias = true;

    bool draw_nwell = false;

    std::optional<geometry::Compass> p_tab_position;
    std::optional<geometry::Compass> n_tab_position;

    void ToProto(proto::parameters::Sky130TransmissionGate *pb) const;
    void FromProto(const proto::parameters::Sky130TransmissionGate &pb);
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
      .poly_overhang_top_nm =
          (parameters_.n_tab_position &&
              geometry::CompassHasNorth(*parameters_.n_tab_position)) ?
              parameters_.min_n_tab_diff_separation_nm : std::nullopt,
      .poly_overhang_bottom_nm = 
          (parameters_.n_tab_position &&
              geometry::CompassHasSouth(*parameters_.n_tab_position)) ?
              parameters_.min_n_tab_diff_separation_nm : std::nullopt,
      .stacking_pitch_nm = parameters_.poly_pitch_nm
    };

    nfet_generator_.reset(new Sky130SimpleTransistor(nfet_params, design_db_));
    nfet_generator_->set_name("nmos");

    Sky130SimpleTransistor::Parameters pfet_params = {
      .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
      .width_nm = parameters_.p_width_nm,
      .length_nm = parameters_.p_length_nm,
      .stacks_left = parameters_.stacks_left,
      .stacks_right = parameters_.stacks_right,
      .poly_overhang_top_nm =
          (parameters_.p_tab_position &&
              geometry::CompassHasNorth(*parameters_.p_tab_position)) ?
              parameters_.min_p_tab_diff_separation_nm : std::nullopt,
      .poly_overhang_bottom_nm = 
          (parameters_.p_tab_position &&
              geometry::CompassHasSouth(*parameters_.p_tab_position)) ?
              parameters_.min_p_tab_diff_separation_nm : std::nullopt,
      .stacking_pitch_nm = parameters_.poly_pitch_nm
    }; 
 
    pfet_generator_.reset(new Sky130SimpleTransistor(pfet_params, design_db_));
    pfet_generator_->set_name("pmos");
  }

  const geometry::Rectangle PMOSBounds() const {
    return pfet_generator_->DiffBounds();
  }
  const geometry::Rectangle NMOSBounds() const {
    return nfet_generator_->DiffBounds();
  }

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

  const Sky130SimpleTransistor &pfet_generator() const {
    return *pfet_generator_;
  }
  const Sky130SimpleTransistor &nfet_generator() const {
    return *nfet_generator_;
  }

  // This will return the transistor as a single Cell, which is usually
  // annoying. Prefer calling GenerateLayout and GenerateCircuit to flatly merge
  // outputs directly into parent cell.
  bfg::Cell *Generate() override;

  bfg::Layout *GenerateLayout();

  bfg::Circuit *GenerateCircuit();

 private:
  // The layout's vertical axis has these components, schematically:
  //
  // ----------- top boundary
  //           ^
  //           | top padding: space to top boundary
  //           v
  //    +-+    ^
  //    | |    | pmos tab (complex) height (fixed)
  //    +-+    v 
  //    | |    ^  
  //    | |    v pmos tab connector (variable)
  //    +-+    ^ 
  //    | |    |
  //    | |    | pmos poly height (fixed)
  //    +-+    v
  //           ^
  //           | cmos gap (variable)
  //           v
  //    +-+    ^
  //    | |    | nmos poly height (fixed)
  //    | |    v
  //    +-+    ^
  //    | |    | nmos tab connector (variable)
  //    | |    v
  //    +-+    ^
  //    | |    | nmos tab (complex) height (fixed)
  //    +-+    v
  //           ^
  //           | bottom padding: space to bottom boundary
  //           v
  // ----------- bottom boundary (y = 0)
  //
  // For convenience we can force the tabs to line up with an overlying grid
  // with two parameters: pitch (spacing between lines) and offset (distance to
  // first line from y = 0).
  //
  // The algorithm for placement will be something like:
  //  - If no grid is given, place NMOS, PMOS and their tabs as compactly as
  //  possible.
  //  - If a grid is given, starting at y = 0 and going up, place tabs, then
  //  their corresponding transistors (or vice versa depending on where the tabs
  //  are needed), so that tabs line up with the nearest grid position.
  //  Placement can be expanded up with increasing y, but not down.
  struct VerticalSpacings {
    int64_t bottom_padding;
    //int64_t nmos_tab_height;
    int64_t nmos_tab_extension;
    int64_t nmos_poly_bottom_y;
    //int64_t nmos_poly_height;
    //int64_t cmos_gap;
    //int64_t pmos_tab_height;
    int64_t pmos_tab_extension;
    //int64_t pmos_poly_height;
    int64_t pmos_poly_bottom_y;
    int64_t top_padding;
    int64_t cell_height;
  };

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
  geometry::Polygon *AddPolyTab(const Sky130SimpleTransistor &fet_generator,
                                const geometry::Compass &position,
                                int64_t connector_height,
                                Layout *layout);

  const VerticalSpacings FigureSpacings() const;

  bool PMOSHasUpperTab() const;
  bool PMOSHasLowerTab() const;
  bool NMOSHasUpperTab() const;
  bool NMOSHasLowerTab() const;

  bool PMOSHasAnyTab() const {
    return PMOSHasUpperTab() || PMOSHasLowerTab();
  }

  int64_t NextYOnGrid(int64_t current_y) const;

  int64_t FigureBottomPadding() const;
  int64_t FigureNMOSLowerTabConnectorHeight() const;
  int64_t FigureNMOSUpperTabConnectorHeight(int64_t nmos_poly_top_y) const;
  int64_t FigurePMOSLowerTabConnectorHeight() const;
  int64_t FigurePMOSUpperTabConnectorHeight(int64_t pmos_poly_top_y) const;
  int64_t FigureCMOSGap(int64_t current_y) const;

  int64_t FigureTopPadding(int64_t pmos_poly_top_y) const;

  int64_t PMOSPolyHeight() const {
    return pfet_generator_->PolyHeight();
  }
  int64_t PMOSPolyOverhangTop() const {
    return pfet_generator_->PolyOverhangTop();
  }
  int64_t PMOSPolyOverhangBottom() const {
    return pfet_generator_->PolyOverhangBottom();
  }
  int64_t NMOSPolyHeight() const {
    return nfet_generator_->PolyHeight();
  }
  int64_t NMOSPolyOverhangTop() const {
    return nfet_generator_->PolyOverhangTop();
  }
  int64_t NMOSPolyOverhangBottom() const {
    return nfet_generator_->PolyOverhangBottom();
  }

  // The tab will be a horizontal rectangle, whose height and width must
  // accommodate a via on the DiffConnectionLayer().
  int64_t PolyTabHeight(const Sky130SimpleTransistor &fet_generator) const;
  int64_t PolyTabWidth(const Sky130SimpleTransistor &fet_generator) const;

  Parameters parameters_;

  std::unique_ptr<Sky130SimpleTransistor> nfet_generator_;
  std::unique_ptr<Sky130SimpleTransistor> pfet_generator_;
};

}  // namespace atoms
}  // namespace bfg

#endif  // ATOMS_SKY130_TRANSMISSION_GATE_H_

