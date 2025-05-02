#include "sky130_transmission_gate_stack.h"

#include <absl/strings/str_format.h>

#include "../row_guide.h"
#include "../scoped_layer.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "sky130_transmission_gate.h"

namespace bfg {
namespace atoms {

bfg::Cell *Sky130TransmissionGateStack::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_transmission_gate_stack": name_));
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  cell->SetLayout(new bfg::Layout(db));

  RowGuide row(geometry::Point(0, 0),
               cell->layout(),
               cell->circuit(),
               design_db_);

  const size_t num_gates = parameters_.net_sequence.empty() ?
      0 : (parameters_.net_sequence.size() - 1) / 2;

  std::optional<geometry::Rectangle> pdiff_cover;
  std::optional<geometry::Rectangle> ndiff_cover;

  std::optional<uint64_t> height;

  for (size_t i = 0; i < num_gates; ++i) {
    Sky130TransmissionGate::Parameters gate_params = {
      .stacks_left = i > 0,
      .stacks_right = i < num_gates - 1,
      .cell_height_nm = parameters_.min_height_nm,
      .vertical_tab_pitch_nm = parameters_.vertical_pitch_nm,
      .poly_pitch_nm = parameters_.horizontal_pitch_nm,
      .p_tab_position = geometry::Compass::LOWER,
      .n_tab_position = geometry::Compass::LOWER
    };

    Sky130TransmissionGate generator =
        Sky130TransmissionGate(gate_params, design_db_);

    std::string instance_name =
        absl::StrFormat("stack_%d_gate_%d_template", 0, i);
    Cell *transmission_gate = generator.GenerateIntoDatabase(instance_name);

    // TODO(aryap): I spent a lot of effort in the Sky130TransmissionGate
    // decoupling constructed properties (i.e. positions of elements, widths of
    // diffs, etc) from the actual layout generation, whereas it would have been
    // simpler to do it all at the same time, when building the layout. The
    // point of that was to have access to where e.g. Vias might go without
    // having to generate the layout. But most of that work is based on the
    // origin_ set in the Generator, whereas we usually want to manipulate the
    // origin_ of an Instance, i.e. what the RowGuid does. So most of the
    // precomputed properties available through the Generator are useless here.
    geometry::Instance *instance = row.InstantiateBack(
        instance_name, transmission_gate->layout());

    // Connecting P- and NMOS sources and drains:
    const std::string &left_net = parameters_.net_sequence[2 * i];
    const std::string &gate_net = parameters_.net_sequence[2 * i + 1];
    const std::string &right_net = parameters_.net_sequence[2 * i + 2];

    LOG(INFO) << left_net << ", " << gate_net << ", " << right_net;

    geometry::Point pmos_ll = instance->GetPointOrDie("pmos.diff_lower_left");
    geometry::Point pmos_ur = instance->GetPointOrDie("pmos.diff_upper_right");
    if (!pdiff_cover) {
      pdiff_cover = geometry::Rectangle(pmos_ll, pmos_ur);
    } else {
      pdiff_cover->ExpandToCover(geometry::Rectangle(pmos_ll, pmos_ur));
    }
    geometry::Point nmos_ll = instance->GetPointOrDie("nmos.diff_lower_left");
    geometry::Point nmos_ur = instance->GetPointOrDie("nmos.diff_upper_right");
    if (!ndiff_cover) {
      ndiff_cover = geometry::Rectangle(nmos_ll, nmos_ur);
    } else {
      ndiff_cover->ExpandToCover(geometry::Rectangle(nmos_ll, nmos_ur));
    }

    geometry::Point top = instance->GetPointOrDie("pmos.via_left_diff_upper");
    geometry::Point bottom =
        instance->GetPointOrDie("nmos.via_left_diff_lower");

    ConnectDiffs(generator, top, bottom, left_net, cell->layout());

    if (i == num_gates - 1) {
      top = instance->GetPointOrDie("pmos.via_right_diff_upper");
      bottom = instance->GetPointOrDie("nmos.via_right_diff_lower");
      ConnectDiffs(generator, top, bottom, right_net, cell->layout());
    }
  }

  // Add nwell.
  if (pdiff_cover) {
    int64_t psdm_margin = db.Rules(
        "psdm.drawing", "pdiff.drawing").min_enclosure;
    {
      ScopedLayer layer(cell->layout(), "psdm.drawing");
      cell->layout()->AddRectangle(pdiff_cover->WithPadding(psdm_margin));
    }
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    {
      ScopedLayer layer(cell->layout(), "nwell.drawing");
      cell->layout()->AddRectangle(
          pdiff_cover->WithPadding(nwell_margin));
    }
  }
  if (ndiff_cover) {
    ScopedLayer layer(cell->layout(), "nsdm.drawing");
    int64_t nsdm_margin = db.Rules(
        "nsdm.drawing", "ndiff.drawing").min_enclosure;
    cell->layout()->AddRectangle(ndiff_cover->WithPadding(nsdm_margin));
  }

//  cell->SetLayout(GenerateLayout());
//  cell->SetCircuit(GenerateCircuit());
//
  return cell.release();
}

void Sky130TransmissionGateStack::ConnectDiffs(
    const Sky130TransmissionGate &generator,
    const geometry::Point &top,
    const geometry::Point &bottom,
    const std::string &net,
    Layout *layout) {
  static constexpr char kMetalLayer[] = "li.drawing";

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  const auto &metal_rules = db.Rules(kMetalLayer);

  const auto &pcon_rules = db.Rules(
      generator.pfet_generator().DiffConnectionLayer());
  const auto &ncon_rules = db.Rules(
      generator.nfet_generator().DiffConnectionLayer());
  const auto &metal_pcon_rules = db.Rules(
      kMetalLayer, generator.pfet_generator().DiffConnectionLayer());
  const auto &metal_ncon_rules = db.Rules(
      kMetalLayer, generator.nfet_generator().DiffConnectionLayer());

  LOG(INFO) << "line between " << top << " and " << bottom;
  layout->MakeVia(generator.pfet_generator().DiffConnectionLayer(), top);
  layout->MakeVia(generator.nfet_generator().DiffConnectionLayer(), bottom);

  ScopedLayer scoped_layer(layout, kMetalLayer);
  geometry::PolyLine line = geometry::PolyLine({top, bottom});

  if (parameters_.li_width_nm) {
    line.SetWidth(db.ToInternalUnits(*parameters_.li_width_nm));
  } else {
    line.SetWidth(metal_rules.min_width);
  }

  line.InsertBulge(
      top,
      pcon_rules.via_width + 2 * metal_pcon_rules.via_overhang_wide,
      pcon_rules.via_height + 2 * metal_pcon_rules.via_overhang);
  line.InsertBulge(
      bottom,
      ncon_rules.via_width + 2 * metal_ncon_rules.via_overhang_wide,
      ncon_rules.via_height + 2 * metal_ncon_rules.via_overhang);

  line.set_net(net);

  layout->AddPolyLine(line);
}

}  // namespace atoms
}  // namespace bfg
