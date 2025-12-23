#include "sky130_transmission_gate_stack.h"

#include <cstdint>
#include <memory>
#include <map>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "../utility.h"
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

  if (poly_contact_vertical_pitch_nm) {
    pb->set_poly_contact_vertical_pitch_nm(*poly_contact_vertical_pitch_nm);
  } else {
    pb->clear_poly_contact_vertical_pitch_nm();
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
  if (pb.sequences_size() > 0) {
  sequences.clear();
  }
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
  }

  if (pb.has_min_height_nm()) {
    min_height_nm = pb.min_height_nm();
  }

  if (pb.has_poly_contact_vertical_pitch_nm()) {
    poly_contact_vertical_pitch_nm = pb.poly_contact_vertical_pitch_nm();
  }

  if (pb.has_horizontal_pitch_nm()) {
    horizontal_pitch_nm = pb.horizontal_pitch_nm();
  }

  if (pb.has_poly_pitch_nm()) {
    poly_pitch_nm = pb.poly_pitch_nm();
  }

  if (pb.has_min_p_tab_diff_separation_nm()) {
    min_p_tab_diff_separation_nm = pb.min_p_tab_diff_separation_nm();
  }

  if (pb.has_min_n_tab_diff_separation_nm()) {
    min_n_tab_diff_separation_nm = pb.min_n_tab_diff_separation_nm();
  }

  if (pb.has_min_poly_boundary_separation_nm()) {
    min_poly_boundary_separation_nm = pb.min_poly_boundary_separation_nm();
  }

  if (pb.has_insert_dummy_poly()) {
    insert_dummy_poly = pb.insert_dummy_poly();
  }
  if (pb.has_expand_wells_to_vertical_bounds()) {
    expand_wells_to_vertical_bounds = pb.expand_wells_to_vertical_bounds();
  }
}

std::string Sky130TransmissionGateStack::Parameters::DebugSequences() const {
  std::vector<std::string> strings;
  for (const auto &sequence : sequences) {
    strings.push_back(absl::StrFormat("{%s}", absl::StrJoin(sequence, ", ")));
  }
  return absl::StrJoin(strings, ", ");
}

// TODO(aryap): It would be nice to have the Sky130TransmissionGate tell us this
// based on its configuration. Or at least it would be nice to use parts of its
// static configuration to tell us this, like the nfet_generator()
// (Sky130SimpleTransistor) it has. Then we could update how we configure it
// based on that. Not sure if needed yet.
int64_t
Sky130TransmissionGateStack::GapInYFromNMOSDiffLowerLeftToMconViaCentre()
    const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &diff_dcon_rules = db.Rules("ncon.drawing", "ndiff.drawing");
  const auto &li_licon_rules = db.Rules("li.drawing", "licon.drawing");
  const auto &mcon_rules = db.Rules("mcon.drawing");
  const auto &li_mcon_rules = db.Rules("li.drawing", "mcon.drawing");
  int64_t gap_y =
      std::max(
          diff_dcon_rules.min_enclosure, diff_dcon_rules.min_enclosure_alt) -
      std::max(li_licon_rules.via_overhang, li_licon_rules.via_overhang_wide) +
      std::max(li_mcon_rules.via_overhang, li_mcon_rules.via_overhang_wide) +
      mcon_rules.via_height / 2;
  return gap_y;
}

void Sky130TransmissionGateStack::BuildSequence(
    const std::vector<std::string> &net_sequence,
    size_t *gates_so_far,
    std::map<std::string, size_t> *net_counts,
    bfg::Cell *cell,
    RowGuide *row,
    std::optional<geometry::Rectangle> *pdiff_cover,
    std::optional<geometry::Rectangle> *ndiff_cover,
    std::optional<geometry::Rectangle> *p_poly_via_cover,
    std::optional<geometry::Rectangle> *n_poly_via_cover) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // Get a handle to the circuit so we can output the circuit simultaneously.
  bfg::Circuit *circuit = cell->circuit();

  const size_t num_gates = net_sequence.empty() ?
      0 : (net_sequence.size() - 1) / 2;

  std::optional<uint64_t> min_via_distance_nm;
  if (parameters_.num_horizontal_channels) {
    min_via_distance_nm = *parameters_.num_horizontal_channels *
                          parameters_.li_via_pitch.value_or(340);
  }

  // In order to line up the connections to the li wires between the P- and NMOS
  // diffs (the inputs to the gates) with a given vertical pitch, we have to
  // find the distance between the bottom-most one of those and the lower-left
  // corner of the NMOS diff, in y. This is so that we can specify to the
  // Sky130TransmissionGate to move the diffs in such a way that the
  // pitch-alignmed contacts all fit.
  int64_t diff_ll_to_bottom_via_centre_y =
      GapInYFromNMOSDiffLowerLeftToMconViaCentre();

  for (size_t i = 0; i < num_gates; ++i) {
    Sky130TransmissionGate::Parameters gate_params = {
      .p_width_nm = parameters_.p_width_nm,
      .p_length_nm = parameters_.p_length_nm,
      .n_width_nm = parameters_.n_width_nm,
      .n_length_nm = parameters_.n_length_nm,
      .stacks_left = i > 0,
      .stacks_right = i < num_gates - 1,
      .min_cell_height_nm = parameters_.min_height_nm,
      .vertical_tab_pitch_nm = parameters_.poly_contact_vertical_pitch_nm,
      .vertical_tab_offset_nm =
          parameters_.poly_contact_vertical_offset_nm.value_or(0),
      .poly_pitch_nm = parameters_.poly_pitch_nm,
      .nmos_ll_vertical_pitch_nm = parameters_.input_vertical_pitch_nm,
      .nmos_ll_vertical_offset_nm = -db.ToExternalUnits(
          diff_ll_to_bottom_via_centre_y) +
          parameters_.input_vertical_offset_nm.value_or(0),
      .min_p_tab_diff_separation_nm = parameters_.min_p_tab_diff_separation_nm,
      .min_n_tab_diff_separation_nm = parameters_.min_n_tab_diff_separation_nm,
      .allow_metal_channel_top = parameters_.top_metal_channel_net.has_value(),
      .allow_metal_channel_bottom = parameters_.bottom_metal_channel_net.has_value(),
      .min_poly_boundary_separation_nm =
          parameters_.min_poly_boundary_separation_nm,
      .min_furthest_via_distance_nm = min_via_distance_nm,
      .pitch_match_to_boundary = false,
      .tabs_should_avoid_nearest_vias = true,
      .draw_nwell = false,
      .p_tab_position = geometry::Compass::UPPER,
      .n_tab_position = geometry::Compass::LOWER
    };

    Sky130TransmissionGate generator =
        Sky130TransmissionGate(gate_params, design_db_);

    std::string instance_name =
        absl::StrFormat("stack_%d_gate_%u", *gates_so_far, i);
    Cell *transmission_gate = generator.GenerateIntoDatabase(
        absl::StrCat(instance_name, "_template"));

    // LOG(INFO) << transmission_gate->layout()->GetBoundingBox().Width();

    // TODO(aryap): I spent a lot of effort in the Sky130TransmissionGate
    // decoupling constructed properties (i.e. positions of elements, widths
    // of diffs, etc) from the actual layout generation, whereas it would have
    // been simpler to do it all at the same time, when building the layout.
    // The point of that was to have access to where e.g. Vias might go
    // without having to generate the layout. But most of that work is based
    // on the origin_ set in the Generator, whereas we usually want to
    // manipulate the origin_ of an Instance, i.e. what the RowGuid does. So
    // most of the precomputed properties available through the Generator are
    // useless here. So, great. Nice work. Whatever.
    geometry::Instance *layout_instance = row->InstantiateBack(
        instance_name, transmission_gate->layout());

    circuit::Instance *circuit_instance = circuit->AddInstance(
        instance_name, transmission_gate->circuit());
    bfg::Cell::TieInstances(circuit_instance, layout_instance);

    // Connecting P- and NMOS sources and drains:
    const std::string &left_net = net_sequence[2 * i];
    const std::string &gate_net = net_sequence[2 * i + 1];
    const std::string &right_net = net_sequence[2 * i + 2];

    circuit_instance->Connect("IN", *circuit->GetOrAddSignal(left_net, 1));
    circuit_instance->Connect("OUT", *circuit->GetOrAddSignal(right_net, 1));
    circuit_instance->Connect("S", *circuit->GetOrAddSignal(gate_net, 1));
    circuit_instance->Connect(
        "S_B", *circuit->GetOrAddSignal(absl::StrCat(gate_net, "_B"), 1));
    circuit_instance->Connect(
        "VPB", *circuit->GetOrAddSignal(parameters_.power_net, 1));
    circuit_instance->Connect(
        "VNB", *circuit->GetOrAddSignal(parameters_.ground_net, 1));

    geometry::Point pmos_ll = layout_instance->GetPointOrDie(
        "pmos.diff_lower_left");
    geometry::Point pmos_ur = layout_instance->GetPointOrDie(
        "pmos.diff_upper_right");
    geometry::Rectangle::ExpandAccumulate(
        geometry::Rectangle(pmos_ll, pmos_ur), pdiff_cover);

    std::optional<geometry::Point> p_via_ll =
        layout_instance->GetPoint("pmos.poly_tab_ll");
    std::optional<geometry::Point> p_via_ur =
        layout_instance->GetPoint("pmos.poly_tab_ur");
    if (p_via_ll && p_via_ur) {
      geometry::Rectangle::ExpandAccumulate(
          geometry::Rectangle(*p_via_ll, *p_via_ur), p_poly_via_cover);
    }

    geometry::Point nmos_ll = layout_instance->GetPointOrDie(
        "nmos.diff_lower_left");
    geometry::Point nmos_ur = layout_instance->GetPointOrDie(
        "nmos.diff_upper_right");
    geometry::Rectangle::ExpandAccumulate(
        geometry::Rectangle(nmos_ll, nmos_ur), ndiff_cover);

    std::optional<geometry::Point> n_via_ll =
        layout_instance->GetPoint("nmos.poly_tab_ll");
    std::optional<geometry::Point> n_via_ur =
        layout_instance->GetPoint("nmos.poly_tab_ur");
    if (n_via_ll && n_via_ur) {
      geometry::Rectangle::ExpandAccumulate(
          geometry::Rectangle(*n_via_ll, *n_via_ur), n_poly_via_cover);
    }

    geometry::Point top = layout_instance->GetPointOrDie(
        "pmos.via_left_diff_upper");
    geometry::Point bottom = layout_instance->GetPointOrDie(
        "nmos.via_left_diff_lower");

    ConnectDiffs(generator, top, bottom, left_net, net_counts, cell->layout());

    if (i == num_gates - 1) {
      top = layout_instance->GetPointOrDie("pmos.via_right_diff_upper");
      bottom = layout_instance->GetPointOrDie("nmos.via_right_diff_lower");
      ConnectDiffs(
          generator, top, bottom, right_net, net_counts, cell->layout());
    }

    size_t k = i + *gates_so_far;
    cell->layout()->SavePoint(
        absl::StrFormat("gate_%u_p_tab_centre", k),
        (*p_via_ll + *p_via_ur) / 2);
    cell->layout()->SavePoint(
        absl::StrFormat("gate_%u_n_tab_centre", k),
        (*n_via_ll + *n_via_ur) / 2);
  }

  *gates_so_far += num_gates;
}

bfg::Cell *Sky130TransmissionGateStack::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_transmission_gate_stack": name_));
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  cell->SetLayout(new bfg::Layout(db));
  cell->SetCircuit(new bfg::Circuit());

  LOG(INFO) << "Building net sequences: " << parameters_.DebugSequences();

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

  std::map<std::string, size_t> net_counts;

  row.AddBlankSpaceBack(min_spacing / 2);
  for (size_t i = 0; i < parameters_.sequences.size(); ++i) {
    BuildSequence(parameters_.sequences[i],
                  &num_gates,
                  &net_counts,
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
  cell->layout()->Flatten(true);
  cell->layout()->EraseLayerByName("areaid.standardc");
  {
    ScopedLayer layer(cell->layout(), "areaid.standardc");
    cell->layout()->AddRectangle(new_tiling_bounds);
  }
  cell->layout()->SetTilingBounds(new_tiling_bounds);

  // Add the top and/or bottom li channel, if present.
  if (parameters_.top_metal_channel_net) {
    AddHorizontalMetalChannel(
        net_counts,
        geometry::Compass::UPPER,
        *parameters_.top_metal_channel_net,
        cell->layout());
  }
  if (parameters_.bottom_metal_channel_net) {
    AddHorizontalMetalChannel(
        net_counts,
        geometry::Compass::LOWER,
        *parameters_.bottom_metal_channel_net,
        cell->layout());
  }

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
        psdm_rectangle.ExpandUpToCover(new_tiling_bounds);
      }
      if (parameters_.expand_wells_to_horizontal_bounds) {
        psdm_rectangle.ExpandHorizontallyToCover(new_tiling_bounds);
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
        nwell_rectangle.ExpandUpToCover(new_tiling_bounds);
      }
      if (parameters_.expand_wells_to_horizontal_bounds) {
        nwell_rectangle.ExpandHorizontallyToCover(new_tiling_bounds);
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
        hvtp_rectangle.ExpandUpToCover(new_tiling_bounds);
      }
      if (parameters_.expand_wells_to_horizontal_bounds) {
        hvtp_rectangle.ExpandHorizontallyToCover(new_tiling_bounds);
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

  // Not strictly necessary to do this, but way more readable. And we do it for
  // the layout so it keeps the hierarchies 1:1.
  cell->circuit()->Flatten();

  return cell.release();
}

void Sky130TransmissionGateStack::ConnectDiffs(
    const Sky130TransmissionGate &generator,
    const geometry::Point &top,
    const geometry::Point &bottom,
    const std::string &net,
    std::map<std::string, size_t> *net_counts,
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


  // In C++23 this is just:
  //line.SetWidth(parameters_.li_width_nm
  //                  .and_then(db.ToInternalUnits)
  //                  .value_or(metal_rules.min_width));
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

  geometry::Polygon *wire = layout->AddPolyLine(line);

  // If configured, centre_y has to be a multiple of the pitch from the
  // bottom-most via position.
  int64_t centre_y = (top.y() + bottom.y()) / 2;
  if (parameters_.input_vertical_pitch_nm) {
    int64_t pitch = db.ToInternalUnits(*parameters_.input_vertical_pitch_nm);
    int64_t offset = db.ToInternalUnits(
        parameters_.input_vertical_offset_nm.value_or(0));
    centre_y = Utility::LastMultiple(centre_y - offset, pitch) + offset;
  }

  if (parameters_.add_ports) {
    const auto &via_rules = db.Rules(kMetalViaLayer);
    const auto &metal_via_rules = db.Rules(kMetalLayer, kMetalViaLayer);

    geometry::Rectangle pin = geometry::Rectangle::CentredAt(
        {top.x(), centre_y}, via_rules.via_width, via_rules.via_height);
    ScopedLayer sl(layout, kMetalPinLayer);
    layout->AddRectangleAsPort(pin, net);
  }

  size_t count = (*net_counts)[net];
  layout->SavePoint(absl::StrFormat("net_%s_via_top_%u", net, count), top);
  layout->SavePoint(
      absl::StrFormat("net_%s_via_bottom_%u", net, count),
      bottom);

  geometry::Point wire_centre = wire->GetBoundingBox().centre();
  layout->SavePoint(
      absl::StrFormat("net_%s_edge_top_%u", net, count),
      {wire_centre.x(), wire->GetBoundingBox().upper_right().y()});
  layout->SavePoint(
      absl::StrFormat("net_%s_edge_bottom_%u", net, count),
      {wire_centre.x(), wire->GetBoundingBox().lower_left().y()});
  (*net_counts)[net] = count + 1;
}

void Sky130TransmissionGateStack::AddHorizontalMetalChannel(
    const std::map<std::string, size_t> &net_counts,
    const geometry::Compass &side,
    const std::string &net,
    Layout *layout) const {
  // Assemble the connection points for the bars of the given net.
  std::vector<geometry::Point> edge_points;

  // TODO(aryap): We don't have to use the saved point mechanism for this,
  // since we're using the data in this generator. We could just  emit the
  // points directly. But that adds significant complexity to the function
  // signature for very little real gain.
  size_t count = net_counts.find(net)->second;
  std::string top_or_bottom =
      geometry::CompassHasNorth(side) ? "top" : "bottom";
  for (int i = 0; i < count; ++i) {
    geometry::Point point = layout->GetPointOrDie(
        absl::StrFormat("net_%s_edge_%s_%u", net, top_or_bottom, i));
    edge_points.push_back(point);
  }

  if (edge_points.size() < 2) {
    LOG(WARNING) << "Will not draw horizontal metal channel for net with 1 "
                 << "connection point";
    // Nope.
    return;
  }

  std::sort(
      edge_points.begin(), edge_points.end(), geometry::Point::CompareXThenY);

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  const auto &metal_rules = db.Rules(kMetalLayer);

  int64_t metal_width = parameters_.li_width_nm ?
    metal_width = db.ToInternalUnits(*parameters_.li_width_nm) :
    metal_rules.min_width;

  const geometry::Point &first = edge_points.front();
  const geometry::Point &last = edge_points.back();

  int64_t y_level = edge_points.front().y() + (
      geometry::CompassHasNorth(side) ? 1 : -1) * (
      metal_width / 2 + metal_rules.min_separation);
  geometry::Point p1 = {first.x(), y_level};
  geometry::Point p2 = {last.x(), y_level};

  ScopedLayer sl(layout, kMetalLayer);
  geometry::PolyLine line({first, p1, p2, last});
  line.SetWidth(metal_width);
  line.set_net(net);

  // p2 becomes a good place to put a port, by the way.
  geometry::Polygon *wire = layout->AddPolyLine(line);

  layout->SavePoint(absl::StrFormat("net_%s_rightmost_wire_centre", net), p2);

  // Lastly, connect all the other wires.
  //
  // TODO(aryap): I'm trying to just touch the metal wire, but under some
  // conditions (odd wire width), truncation on division by 2 will mean the
  // wire doesn't touch. That is bad. Easy remedy is to just overlap the wire,
  // but it's inelegant.
  for (auto it = edge_points.begin() + 1; it < edge_points.end() - 1; ++it) {
    const geometry::Point &source = *it;
    geometry::Point dest = {
        source.x(),
        y_level + (geometry::CompassHasNorth(side) ? -1 : 1) * metal_width / 2};
    geometry::PolyLine jog({source, dest});
    jog.SetWidth(metal_width);
    jog.set_net(net);
    layout->AddPolyLine(jog);
  }
}

}  // namespace atoms
}  // namespace bfg
