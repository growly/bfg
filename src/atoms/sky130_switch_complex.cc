#include "sky130_switch_complex.h"

#include <numeric>

#include "../circuit.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "../geometry/polygon.h"

#include "sky130_simple_transistor.h"

namespace bfg {
namespace atoms {

bfg::Cell *Sky130SwitchComplex::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_switch_complex": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

bfg::Layout *Sky130SwitchComplex::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  layout->SetActiveLayerByName("poly.drawing");

  // Poly pitches:
  //     0    1    2    3    4    5    6    7
  //
  //    NE_B EW_B      NS_B ES_B      SW_B NW_B
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //
  //
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //     |    |         |    |         |    |
  //    NE   EW        NS   ES        SW   NW
  //
  //     0    1    2    3    4    5    6    7

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &poly_rules = db.Rules("poly.drawing");
  const auto &poly_pdiff_rules = db.Rules("poly.drawing", "pdiff.drawing");
  const auto &poly_ndiff_rules = db.Rules("poly.drawing", "ndiff.drawing");
  int64_t poly_pdiff_overhang = poly_pdiff_rules.min_enclosure;
  int64_t poly_ndiff_overhang = poly_ndiff_rules.min_enclosure;
  int64_t poly_pitch = poly_rules.min_pitch;

  // "dcon" is shorthand for "pcon" or "ncon"
  const auto &pcon_rules = db.Rules("pcon.drawing");
  int64_t pcon_via_side = pcon_rules.via_width;
  const auto &ncon_rules = db.Rules("ncon.drawing");
  int64_t ncon_via_side = ncon_rules.via_width;

  //      poly    poly
  //      |   |   |
  // +----|   |---|
  // |    |   |   |
  // |    |   |   |
  // +----|   |---|
  //  <---|   |
  //   diff_wing
  //int64_t diff_wing = via_side + poly_dcon_rules.min_separation +
  //    diff_dcon_rules.min_enclosure;

  int64_t x_start = 0;
  int64_t y_min = 0;

  std::map<std::string, TransistorSpecs> transistor_specs = {
    {"NE", TransistorSpecs{
      .x = x_start,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ne_nfet_width_nm,
        .length_nm = parameters_.ne_nfet_length_nm,
        .stacks_right = true
      }}},
    {"EW", TransistorSpecs{
      .x = x_start + poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ew_nfet_width_nm,
        .length_nm = parameters_.ew_nfet_length_nm,
        .stacks_left = true
      }}},
    {"NS", TransistorSpecs{
      .x = x_start + 3 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.ns_nfet_width_nm,
        .length_nm = parameters_.ns_nfet_length_nm,
        .stacks_right = true
      }}},
    {"ES", TransistorSpecs{
      .x = x_start + 4 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.es_nfet_width_nm,
        .length_nm = parameters_.es_nfet_length_nm,
        .stacks_left = true
      }}},
    {"SW", TransistorSpecs{
      .x = x_start + 6 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.nw_nfet_width_nm,
        .length_nm = parameters_.nw_nfet_length_nm,
        .stacks_right = true
      }}},
    {"NW", TransistorSpecs{
      .x = x_start + 7 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
        .width_nm = parameters_.sw_nfet_width_nm,
        .length_nm = parameters_.sw_nfet_length_nm,
        .stacks_left = true
      }}},
    {"NE_B", TransistorSpecs{
      .x = x_start,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ne_pfet_width_nm,
        .length_nm = parameters_.ne_pfet_length_nm,
        .stacks_right = true
      }}},
    {"EW_B", TransistorSpecs{
      .x = x_start + poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ew_pfet_width_nm,
        .length_nm = parameters_.ew_pfet_length_nm,
        .stacks_left = true
      }}},
    {"NS_B", TransistorSpecs{
      .x = x_start + 3 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.ns_pfet_width_nm,
        .length_nm = parameters_.ns_pfet_length_nm,
        .stacks_right = true
      }}},
    {"ES_B", TransistorSpecs{
      .x = x_start + 4 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.es_pfet_width_nm,
        .length_nm = parameters_.es_pfet_length_nm,
        .stacks_left = true
      }}},
    {"SW_B", TransistorSpecs{
      .x = x_start + 6 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.nw_pfet_width_nm,
        .length_nm = parameters_.nw_pfet_length_nm,
        .stacks_right = true
      }}},
    {"NW_B", TransistorSpecs{
      .x = x_start + 7 * poly_pitch,
      .fet_params = Sky130SimpleTransistor::Parameters{
        .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
        .width_nm = parameters_.sw_pfet_width_nm,
        .length_nm = parameters_.sw_pfet_length_nm,
        .stacks_left = true
      }}},
  };


  // We explicitly record separately which are the Ns and which are the Ps so we
  // can conveniently decide which row to put them on. In principal this could
  // be done with a loop of the transistor_specs map too.
  static const std::vector<std::string> kNfetKeys = {
      "NE", "EW", "NS", "ES", "SW", "NW"};
  static const std::vector<std::string> kPfetKeys = {
      "NE_B", "EW_B", "NS_B", "ES_B", "SW_B", "NW_B"};

  for (auto &entry : transistor_specs) {
    TransistorSpecs &specs = entry.second;
    Sky130SimpleTransistor *fet_generator = new Sky130SimpleTransistor(
        specs.fet_params, design_db_);
    specs.fet_generator = fet_generator;
  }

  // Get the max length of all the bottom- and top-row TransistorSpecs.
  //
  // NOTE: This is more lines of code than a simple for loop to do the same
  // thing.
  uint64_t bottom_row_length_max = std::accumulate(
      kNfetKeys.begin(), kNfetKeys.end(),
      0, [&](uint64_t existing, const std::string &key) {
        return std::max(
            existing, transistor_specs[key].fet_generator->PolyHeight());
      });

  uint64_t top_row_length_max = std::accumulate(
      kPfetKeys.begin(), kPfetKeys.end(),
      0, [&](uint64_t existing, const std::string &key) {
        return std::max(
            existing, transistor_specs[key].fet_generator->PolyHeight());
      });

  for (const std::string &key : kNfetKeys) {
    const TransistorSpecs &specs = transistor_specs[key];
    Sky130SimpleTransistor *fet_generator = specs.fet_generator;
    fet_generator->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
        {specs.x, y_min});
  }

  y_min = y_min + bottom_row_length_max + poly_rules.min_separation +
          top_row_length_max;

  for (const std::string &key : kPfetKeys) {
    const TransistorSpecs &specs = transistor_specs[key];
    Sky130SimpleTransistor *fet_generator = specs.fet_generator;
    fet_generator->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
        {specs.x, y_min});
  }

  for (auto &entry : transistor_specs) {
    TransistorSpecs &specs = entry.second;
    std::unique_ptr<bfg::Layout> transistor_layout(
        specs.fet_generator->GenerateLayout());
    layout->AddLayout(*transistor_layout);
  }

  struct KeyAndViaPosition {
    std::string key;
    Sky130SimpleTransistor::ViaPosition via_position;
  };
  struct JogSpec {
    std::vector<KeyAndViaPosition> pmos;
    std::vector<KeyAndViaPosition> nmos;
  };

  static const std::map<geometry::Compass, std::vector<JogSpec>>
      connections_for_compass_direction = {
    {
      geometry::Compass::NORTH, {
        {
          .pmos = {
            { .key = "NE_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "NE",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        },
        {
          .pmos = {
            { .key = "NS_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "NS",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        },
        {
          .pmos = {
            { .key = "NW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "NW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          }
        }
      }
    }, {
      geometry::Compass::EAST, {
        {
          .pmos = {
            // Use the midpoint of two stacked transistor diff regions:
            { .key = "NE_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "EW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "NE",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "EW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        },
        {
          .pmos = {
            { .key = "ES_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "ES",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          }
        }
      }
    }, {
      geometry::Compass::SOUTH, {
        {
          .pmos = {
            { .key = "NS_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "ES_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "NS",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "ES",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        },
        {
          .pmos = {
            { .key = "SW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "SW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        }
      }
    }, {
      geometry::Compass::WEST, {
        {
          .pmos = {
            { .key = "SW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "NW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "SW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE },
            { .key = "NW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_MIDDLE }
          }
        },
        {
          .pmos = {
            { .key = "EW_B",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          },
          .nmos = {
            { .key = "EW",
              .via_position =
                  Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_MIDDLE }
          }
        }
      }
    }
  };

  // TODO(aryap): Most of these specs are static, and these can be class methods
  // (thay are maybe static?):
  auto locate_via = [&](
      const KeyAndViaPosition &key_and_position) -> geometry::Point {
    return transistor_specs[key_and_position.key].fet_generator->ViaLocation(
        key_and_position.via_position);
  };

  auto resolve_positions = [&](
      const std::vector<KeyAndViaPosition> &keys,
      bool pick_top) -> geometry::Point {
    if (keys.size() != 2) {
      return locate_via(keys.front());
    }

    geometry::Point first = locate_via(keys.front());
    geometry::Point second = locate_via(keys.back());

    if (pick_top) {
      return geometry::Point::PickMaxY(first, second);
    } else {
      return geometry::Point::PickMinY(first, second);
    }
  };

  layout->SetActiveLayerByName("li.drawing");
  // This would be nice:
  // for (const auto &[direction, keys_and_via_positions] : outer_connections) {
  for (const auto &outer : connections_for_compass_direction) {
    const geometry::Compass &direction = outer.first;
    // Each entry in the inner vector is a pair of KeyAndViaPositions defining
    // the P-n and N-FET via connections for a li.drawing jog:
    for (const JogSpec &jog_spec : outer.second) {
      geometry::Point p_via_centre = resolve_positions(jog_spec.pmos, true);
      geometry::Point n_via_centre = resolve_positions(jog_spec.nmos, false);

      geometry::PolyLine line = geometry::PolyLine(
          {p_via_centre, n_via_centre});
      layout->AddPolyLine(line);
    }
  }

  for (auto &entry : transistor_specs) {
    delete entry.second.fet_generator;
    entry.second.fet_generator = nullptr;
  }

  return layout.release();
}

bfg::Circuit *Sky130SwitchComplex::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());
  // TODO(aryap): This.
  return circuit.release();
}

}  // namespace atoms
}  // namespace bfg
