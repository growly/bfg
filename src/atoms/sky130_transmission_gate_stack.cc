#include "sky130_transmission_gate_stack.h"

#include <absl/strings/str_format.h>

#include "../modulo.h"
#include "../row_guide.h"
#include "../scoped_layer.h"
#include "../geometry/point.h"
#include "../geometry/poly_line.h"
#include "proto/parameters/sky130_transmission_gate_stack.pb.h"
#include "sky130_transmission_gate.h"

namespace bfg {
namespace atoms {

void Sky130TransmissionGateStack::Parameters::ToProto(
    proto::parameters::Sky130TransmissionGateStack *pb) const {
  pb->mutable_sequences()->Clear();
  for (const std::vector<std::string> &net_sequence : sequences) { 
    pb->add_sequences()->mutable_nets()->Add(
        net_sequence.begin(), net_sequence.end());
  }

  pb->set_p_width_nm(p_width_nm);
  pb->set_p_length_nm(p_length_nm);
  pb->set_n_width_nm(n_width_nm);
  pb->set_n_length_nm(n_length_nm);

  // TODO(aryap): This could be a macro. I hate that. Also the Google C++ style
  // guide thinks they have bad vibes.
  if (li_width_nm) {
    pb->set_li_width_nm(*li_width_nm);
  } else {
    pb->clear_li_width_nm();
  }

  if (min_height_nm) {
    pb->set_min_height_nm(*min_height_nm);
  } else {
    pb->clear_min_height_nm();
  }

  if (vertical_pitch_nm) {
    pb->set_vertical_pitch_nm(*vertical_pitch_nm);
  } else {
    pb->clear_vertical_pitch_nm();
  }

  if (horizontal_pitch_nm) {
    pb->set_horizontal_pitch_nm(*horizontal_pitch_nm);
  } else {
    pb->clear_horizontal_pitch_nm();
  }

  if (poly_pitch_nm) {
    pb->set_poly_pitch_nm(*poly_pitch_nm);
  } else {
    pb->clear_poly_pitch_nm();
  }

  if (min_p_tab_diff_separation_nm) {
    pb->set_min_p_tab_diff_separation_nm(*min_p_tab_diff_separation_nm);
  } else {
    pb->clear_min_p_tab_diff_separation_nm();
  }

  if (min_n_tab_diff_separation_nm) {
    pb->set_min_n_tab_diff_separation_nm(*min_n_tab_diff_separation_nm);
  } else {
    pb->clear_min_n_tab_diff_separation_nm();
  }

  if (min_n_tab_diff_separation_nm) {
    pb->set_min_n_tab_diff_separation_nm(*min_n_tab_diff_separation_nm);
  } else {
    pb->clear_min_n_tab_diff_separation_nm();
  }

  pb->set_insert_dummy_poly(insert_dummy_poly);
  pb->set_expand_wells_to_vertical_bounds(expand_wells_to_vertical_bounds);
}

void Sky130TransmissionGateStack::Parameters::FromProto(
    const proto::parameters::Sky130TransmissionGateStack &pb) {
  sequences.clear();
  for (const auto &sequence_pb : pb.sequences()) {
    std::vector<std::string> net_sequence = std::vector<std::string>(
        sequence_pb.nets().begin(), sequence_pb.nets().end());
    sequences.push_back(net_sequence);
  }

  if (pb.has_p_width_nm()) {
    p_width_nm = pb.p_width_nm();
  }

  if (pb.has_p_length_nm()) {
    p_length_nm = pb.p_length_nm();
  }

  if (pb.has_n_width_nm()) {
    n_width_nm = pb.n_width_nm();
  }

  if (pb.has_n_length_nm()) {
    n_length_nm = pb.n_length_nm();
  }

  if (pb.has_n_width_nm()) {
    n_width_nm = pb.n_width_nm();
  }

  if (pb.has_li_width_nm()) {
    li_width_nm = pb.li_width_nm();
  } else {
    li_width_nm.reset();
  }

  if (pb.has_min_height_nm()) {
    min_height_nm = pb.min_height_nm();
  } else {
    min_height_nm.reset();
  }

  if (pb.has_vertical_pitch_nm()) {
    vertical_pitch_nm = pb.vertical_pitch_nm();
  } else {
    vertical_pitch_nm.reset();
  }

  if (pb.has_horizontal_pitch_nm()) {
    horizontal_pitch_nm = pb.horizontal_pitch_nm();
  } else {
    horizontal_pitch_nm.reset();
  }

  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  } else {
    poly_pitch_nm.reset();
  }

  if (pb.has_min_p_tab_diff_separation_nm()) {
    min_p_tab_diff_separation_nm = pb.min_p_tab_diff_separation_nm();
  } else {
    min_p_tab_diff_separation_nm.reset();
  }

  if (pb.has_min_n_tab_diff_separation_nm()) {
    min_n_tab_diff_separation_nm = pb.min_n_tab_diff_separation_nm();
  } else {
    min_n_tab_diff_separation_nm.reset();
  }

  if (pb.has_min_poly_boundary_separation_nm()) {
    min_poly_boundary_separation_nm = pb.min_poly_boundary_separation_nm();
  } else {
    min_poly_boundary_separation_nm.reset();
  }

  if (pb.has_insert_dummy_poly()) {
    insert_dummy_poly = pb.insert_dummy_poly();
  }
  if (pb.has_expand_wells_to_vertical_bounds()) {
    expand_wells_to_vertical_bounds = pb.expand_wells_to_vertical_bounds();
  }
}

void Sky130TransmissionGateStack::BuildSequence(
    const std::vector<std::string> &net_sequence,
    size_t *gates_so_far,
    bfg::Cell *cell,
    RowGuide *row,
    std::optional<geometry::Rectangle> *pdiff_cover,
    std::optional<geometry::Rectangle> *ndiff_cover,
    std::optional<geometry::Rectangle> *p_poly_via_cover,
    std::optional<geometry::Rectangle> *n_poly_via_cover) {
  const size_t num_gates = net_sequence.empty() ?
      0 : (net_sequence.size() - 1) / 2;

  for (size_t i = 0; i < num_gates; ++i) {
    Sky130TransmissionGate::Parameters gate_params = {
      .p_width_nm = parameters_.p_width_nm,
      .p_length_nm = parameters_.p_length_nm,
      .n_width_nm = parameters_.n_width_nm,
      .n_length_nm = parameters_.n_length_nm,
      .stacks_left = i > 0,
      .stacks_right = i < num_gates - 1,
      .min_cell_height_nm = parameters_.min_height_nm,
      .vertical_tab_pitch_nm = parameters_.vertical_pitch_nm,
      .vertical_tab_offset_nm = parameters_.vertical_pitch_nm.value_or(0) / 2,
      .poly_pitch_nm = parameters_.poly_pitch_nm,
      .min_poly_boundary_separation_nm =
          parameters_.min_poly_boundary_separation_nm,
      .pitch_match_to_boundary = false,
      .tabs_should_avoid_nearest_vias = true,
      .draw_nwell = false,
      .p_tab_position = geometry::Compass::UPPER,
      .n_tab_position = geometry::Compass::LOWER
    };

    Sky130TransmissionGate generator =
        Sky130TransmissionGate(gate_params, design_db_);

    std::string instance_name =
        absl::StrFormat("stack_%d_gate_%d_template", 0, i);
    Cell *transmission_gate = generator.GenerateIntoDatabase(instance_name);

    LOG(INFO) << transmission_gate->layout()->GetBoundingBox().Width();

    // TODO(aryap): I spent a lot of effort in the Sky130TransmissionGate
    // decoupling constructed properties (i.e. positions of elements, widths
    // of diffs, etc) from the actual layout generation, whereas it would have
    // been simpler to do it all at the same time, when building the layout.
    // The point of that was to have access to where e.g. Vias might go
    // without having to generate the layout. But most of that work is based
    // on the origin_ set in the Generator, whereas we usually want to
    // manipulate the origin_ of an Instance, i.e. what the RowGuid does. So
    // most of the precomputed properties available through the Generator are
    // useless here.
    geometry::Instance *instance = row->InstantiateBack(
        instance_name, transmission_gate->layout());

    // Connecting P- and NMOS sources and drains:
    const std::string &left_net = net_sequence[2 * i];
    const std::string &gate_net = net_sequence[2 * i + 1];
    const std::string &right_net = net_sequence[2 * i + 2];

    LOG(INFO) << left_net << ", " << gate_net << ", " << right_net;

    geometry::Point pmos_ll = instance->GetPointOrDie("pmos.diff_lower_left");
    geometry::Point pmos_ur = instance->GetPointOrDie(
        "pmos.diff_upper_right");
    geometry::Rectangle::ExpandAccumulate(
        geometry::Rectangle(pmos_ll, pmos_ur), pdiff_cover);

    std::optional<geometry::Point> p_via_ll =
        instance->GetPoint("pmos.poly_tab_ll");
    std::optional<geometry::Point> p_via_ur =
        instance->GetPoint("pmos.poly_tab_ur");
    if (p_via_ll && p_via_ur) {
      geometry::Rectangle::ExpandAccumulate(
          geometry::Rectangle(*p_via_ll, *p_via_ur), p_poly_via_cover);
    }

    geometry::Point nmos_ll = instance->GetPointOrDie("nmos.diff_lower_left");
    geometry::Point nmos_ur = instance->GetPointOrDie("nmos.diff_upper_right");
    geometry::Rectangle::ExpandAccumulate(
        geometry::Rectangle(nmos_ll, nmos_ur), ndiff_cover);

    std::optional<geometry::Point> n_via_ll =
        instance->GetPoint("nmos.poly_tab_ll");
    std::optional<geometry::Point> n_via_ur =
        instance->GetPoint("nmos.poly_tab_ur");
    if (n_via_ll && n_via_ur) {
      geometry::Rectangle::ExpandAccumulate(
          geometry::Rectangle(*n_via_ll, *n_via_ur), n_poly_via_cover);
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

    cell->layout()->SavePoint(
        absl::StrFormat("gate_%u_p_tab_centre", i + *gates_so_far),
        (*p_via_ll + *p_via_ur) / 2);
    cell->layout()->SavePoint(
        absl::StrFormat("gate_%u_n_tab_centre", i + *gates_so_far),
        (*n_via_ll + *n_via_ur) / 2);
  }

  *gates_so_far += num_gates;
}


bfg::Cell *Sky130TransmissionGateStack::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_transmission_gate_stack": name_));
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  cell->SetLayout(new bfg::Layout(db));

  RowGuide row(geometry::Point(0, 0),
               cell->layout(),
               cell->circuit(),
               design_db_);

  std::optional<geometry::Rectangle> pdiff_cover;
  std::optional<geometry::Rectangle> ndiff_cover;
  std::optional<geometry::Rectangle> p_poly_via_cover;
  std::optional<geometry::Rectangle> n_poly_via_cover;

  std::optional<uint64_t> height;

  size_t num_gates = 0;

  int64_t min_spacing = db.Rules("diff.drawing").min_separation;

  row.AddBlankSpaceBack(min_spacing / 2);
  for (size_t i = 0; i < parameters_.sequences.size(); ++i) {
    BuildSequence(parameters_.sequences[i],
                  &num_gates,
                  cell.get(),
                  &row,
                  &pdiff_cover,
                  &ndiff_cover,
                  &p_poly_via_cover,
                  &n_poly_via_cover);

    if (i < parameters_.sequences.size() - 1) {
      if (parameters_.poly_pitch_nm) {
        geometry::Instance *last = row.instances().back();
        if (!last)
          continue;
        int64_t pitch = db.ToInternalUnits(*parameters_.poly_pitch_nm);
        int64_t poly_centre_x = last->GetPointOrDie("pmos.poly_centre").x();
        int64_t right_edge_x = last->GetPointOrDie("pmos.diff_upper_right").x();
        int64_t centre_to_edge = right_edge_x - poly_centre_x;
        int64_t remainder = modulo(pitch - (2 * centre_to_edge), pitch);
        row.AddBlankSpaceBack(remainder);
      } else {
        row.AddBlankSpaceBack(min_spacing);
      }

      if (parameters_.insert_dummy_poly) {

      }
    }
  }
  // Force at least enough space to a nearby cell.
  row.AddBlankSpaceBack(min_spacing / 2);

  geometry::Rectangle tiling_bounds = cell->layout()->GetTilingBounds();

  if (parameters_.horizontal_pitch_nm) {
    int64_t pitch = db.ToInternalUnits(*parameters_.horizontal_pitch_nm);
    // We want std::ceil behaviour:
    int64_t min_width =
        (((tiling_bounds.Width() + min_spacing) / pitch) + 1) * pitch;
    min_spacing = (min_width - tiling_bounds.Width());
  }

  geometry::Rectangle new_tiling_bounds =
      tiling_bounds.WithPadding(min_spacing / 2, 0, min_spacing / 2, 0);

  // Turn the transmission gates into a single flat layout so that the nsdm/psdm
  // layers can cover their diffusion regions without causing DRC violations.
  cell->layout()->Flatten();
  cell->layout()->EraseLayerByName("areaid.standardc");
  {
    ScopedLayer layer(cell->layout(), "areaid.standardc");
    cell->layout()->AddRectangle(new_tiling_bounds);
  }
  cell->layout()->SetTilingBounds(new_tiling_bounds);

  // Add nwell.
  if (pdiff_cover) {
    // FIXME(aryap): These need an nwell.pin and the nwell.drawing has to cover
    // it. Same with pwell.pin.
    int64_t psdm_margin = db.Rules(
        "psdm.drawing", "pdiff.drawing").min_enclosure;
    {
      ScopedLayer layer(cell->layout(), "psdm.drawing");
      geometry::Rectangle psdm_rectangle =
          pdiff_cover->WithPadding(psdm_margin);
      if (parameters_.expand_wells_to_vertical_bounds) {
        psdm_rectangle.upper_right().set_y(row.UpperRight().y());
      }
      cell->layout()->AddRectangle(psdm_rectangle);
    }
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    {
      ScopedLayer layer(cell->layout(), "nwell.drawing");
      geometry::Rectangle nwell_rectangle =
          pdiff_cover->WithPadding(nwell_margin);
      if (parameters_.expand_wells_to_vertical_bounds) {
        nwell_rectangle.upper_right().set_y(row.UpperRight().y());
      }
      cell->layout()->AddRectangle(nwell_rectangle);
    }
    int64_t hvtp_margin = db.Rules(
        "hvtp.drawing", "pdiff.drawing").min_enclosure;
    {
      ScopedLayer layer(cell->layout(), "hvtp.drawing");
      geometry::Rectangle hvtp_rectangle =
          pdiff_cover->WithPadding(hvtp_margin);
      if (parameters_.expand_wells_to_vertical_bounds) {
        hvtp_rectangle.upper_right().set_y(row.UpperRight().y());
      }
      cell->layout()->AddRectangle(hvtp_rectangle);
    }
  }
  if (ndiff_cover) {
    ScopedLayer layer(cell->layout(), "nsdm.drawing");
    int64_t nsdm_margin = db.Rules(
        "nsdm.drawing", "ndiff.drawing").min_enclosure;
    geometry::Rectangle nsdm_rectangle =
        ndiff_cover->WithPadding(nsdm_margin);
    if (parameters_.expand_wells_to_vertical_bounds) {
      nsdm_rectangle.lower_left().set_y(row.LowerLeft().y());
    }
    cell->layout()->AddRectangle(nsdm_rectangle);
  }

  // Draw npc.drawing box around poly contacts.
  if (p_poly_via_cover) {
    ScopedLayer layer(cell->layout(), "npc.drawing");
    int64_t npc_margin = db.Rules(
        "npc.drawing", "polycon.drawing").min_enclosure;
    cell->layout()->AddRectangle(p_poly_via_cover->WithPadding(npc_margin));
  }
  if (n_poly_via_cover) {
    ScopedLayer layer(cell->layout(), "npc.drawing");
    int64_t npc_margin = db.Rules(
        "npc.drawing", "polycon.drawing").min_enclosure;
    cell->layout()->AddRectangle(n_poly_via_cover->WithPadding(npc_margin));
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
