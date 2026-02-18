#include "carry1.h"

#include "../cell.h"
#include "proto/parameters/carry1.pb.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_tap.h"
#include "../atoms/sky130_xor2.h"
#include "../routing_track_direction.h"

namespace bfg {
namespace tiles {

void Carry1::Parameters::ToProto(proto::parameters::Carry1 *pb) const {
}

void Carry1::Parameters::FromProto(const proto::parameters::Carry1 &pb) {
}

namespace {

// Assumes the start and end are on li.drawing. Use a met2 vertical wire if the
// horizontal y-positions are more than 1 track apart.
void DrawElbowRoute(const PhysicalPropertiesDatabase &db,
                    const std::vector<geometry::Point> &points,
                    const std::string &net,
                    bfg::Layout *layout) {
  bool vert_first = points.begin()->x() == (points.begin() + 1)->x();
  int64_t y_diff = std::abs(vert_first ?
      points.begin()->y() - (points.begin() + 1)->y() :
      points.rbegin()->y() - (points.rbegin() + 1)->y());
  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch;

  const geometry::Point &start = *points.begin();
  const geometry::Point &end = *points.rbegin();

  if (y_diff > met1_pitch) {
    layout->MakeAlternatingWire(
        points,
        vert_first ? "met2.drawing" : "met1.drawing",
        vert_first ? "met1.drawing" : "met2.drawing",
        net);
    layout->MakeVia("via1.drawing", start, net);
    layout->MakeViaEncap("met1.drawing", "mcon.drawing", "via1.drawing", start);
    layout->MakeVia("mcon.drawing", start, net);
    layout->MakeVia("mcon.drawing", end, net);
    return;
  }

  layout->MakeWire(points, "met1.drawing", "li.drawing", "li.drawing",
                   false, false, net, false,
                   RoutingTrackDirection::kTrackHorizontal,
                   RoutingTrackDirection::kTrackHorizontal);
}

}   // namespace

Cell *Carry1::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));

  Layout *layout = new bfg::Layout(db);
  Circuit *circuit = new bfg::Circuit();

  cell->SetCircuit(circuit);
  cell->SetLayout(layout);

  // Place basic components.
  RowGuide row({0, 0}, layout, cell->circuit(), design_db_);

  // Set the tap cell:
  { 
    atoms::Sky130Tap::Parameters tap_params = {
      .height_nm = 2720,
      .width_nm = atoms::Sky130Parameters::kStandardCellUnitWidthNm
    };
    atoms::Sky130Tap tap_generator(tap_params, design_db_);
    Cell *tap_cell = tap_generator.GenerateIntoDatabase("tap");
    row.set_tap_cell(*tap_cell);
  };

  geometry::Instance *config_memory = AddConfigMemory(&row);
  geometry::Instance *generate_select = AddGenerateSelectMux(&row);
  geometry::Instance *sum_xor = AddSumXor(&row);
  geometry::Instance *carry_select = AddCarrySelectMux(&row);

  // Draw routes.

  int64_t height = row.GetTilingBounds()->Height();
  int64_t met1_pitch = db.Rules("met1.drawing").min_pitch; 
  int64_t met2_pitch = db.Rules("met2.drawing").min_pitch; 
  std::vector<int64_t> tracks_y;
  // Start 1.5 pitches in and end 1.5 pitches before the boundary to accommodate
  // the VPWR/VGND rails.
  //
  // TODO(aryap): We shouldn't be doing it this way in a Tile. This is a
  // violation of our intended separation of concerns. Tiles shouldn't know any
  // PDK or cell-specific detail, or should at least query. (We could query for
  // met1 - I mean, "arbitrary horizontal routing layer" - shapes and avoid
  // them.) Or we could use the RoutingGrid, which is the right level of
  // abstraction. Then again the whole reason I'm doing this is that the
  // RoutingGrid is too heavy. Maybe it's best to just call this a Sky130Carry1,
  // make it an Atom and not a Tile, and avoid the burden.
  for (int64_t y = 3 * met1_pitch / 2;
       y <= height - 3 * met1_pitch / 2;
       y += met1_pitch) {
    tracks_y.push_back(y);
  }

  static constexpr int kCarryInOutTrack = 5;

  geometry::Point carry_in_pin =
      {layout->GetTilingBounds().lower_left().x(), tracks_y[kCarryInOutTrack]};
  geometry::Point carry_out_pin =
      {layout->GetTilingBounds().upper_right().x(), tracks_y[kCarryInOutTrack]};
  layout->MakePin("C_I", carry_in_pin, "met1.pin");
  layout->MakePin("C_O", carry_out_pin, "met1.pin");

  // Since PortSets are sorted geometrically, the first port will always be the
  // same one.
  {
    // config_memory/Q -> generate_select/S -> CONFIG_OUT
    std::string net = "CONFIG_OUT";
    geometry::Point start = (
        *config_memory->GetInstancePortSet("Q").begin())->centre();
    geometry::Point end = (
        *generate_select->GetInstancePortSet("S").begin())->centre();
    geometry::Point p0 = {start.x(), end.y()};

    DrawElbowRoute(db, {start, p0, end}, net, layout);
  }

  {
    // generate_select/X -> carry_select/A1
    std::string net = "i_0";
    geometry::Point start = (
        *generate_select->GetInstancePortSet("X").begin())->centre();
    geometry::Point end = (
        *carry_select->GetInstancePortSet("A1").begin())->centre();

    // Bottom-right of A1:
    geometry::Point p0 =
        (*generate_select->GetInstancePortSet("A1").begin())->centre() +
        geometry::Point(2 * met2_pitch, -met1_pitch);

    geometry::Point p1 = {p0.x(), end.y()};

    DrawElbowRoute(db, {start, p0, p1, end}, net, layout);
  }

  geometry::Point c_i_internal;
  {
    // C_I -> sum_xor/B -> carry_select/A0
    std::string net = "C_I";
    geometry::Point start = (
        *sum_xor->GetInstancePortSet("B").begin())->centre();
    geometry::Point end = (
        *carry_select->GetInstancePortSet("A0").begin())->centre();
    geometry::Point p0 = {start.x(), end.y()};

    DrawElbowRoute(db, {start, p0, end}, net, layout);

    c_i_internal = start;
  }

  {
    // P -> sum_xor/A -> carry_select/S
    std::string net = "P";
    geometry::Point start = (
        *sum_xor->GetInstancePortSet("A").begin())->centre();
    geometry::Point end = (
        *carry_select->GetInstancePortSet("S").begin())->centre();
    geometry::Point p0 = {end.x(), start.y()};

    DrawElbowRoute(db, {start, p0, end}, net, layout);

    // Propagate input.
    layout->MakePin("P", start, "met1.pin");
  }

  {
    // carry_select/X -> C_0
    std::string net = "C_0";
    geometry::Point start = (
        *carry_select->GetInstancePortSet("X").rbegin())->centre();
    geometry::Point end = carry_out_pin;
    geometry::Point p0 = {start.x(), end.y()};

    layout->MakeWire({start, p0, end},
                     "met1.drawing", "li.drawing", std::nullopt,
                     false, false, net, false,
                     RoutingTrackDirection::kTrackHorizontal, std::nullopt);
  }

  {
    // C_I pin -> internal C_I net
    std::string net = "C_I";
    geometry::Point start = carry_in_pin;
    geometry::Point end = c_i_internal;

    geometry::Point p0 = start + geometry::Point(met2_pitch, 0);
    geometry::Point p1 = {p0.x(), tracks_y.back()};
    geometry::Point p2 = {end.x(), p1.y()};

    layout->MakeWire({start, p0, p1, p2},
                     "met1.drawing", std::nullopt, "met2.drawing",
                     false, false, net, false,
                     std::nullopt, RoutingTrackDirection::kTrackHorizontal);
    layout->MakeWire({p2, end},
                     "met2.drawing", "met1.drawing", std::nullopt,
                     false, false, net, false,
                     RoutingTrackDirection::kTrackVertical, std::nullopt);
  }

  // Sum output.
  {
    geometry::Point start = (
        *sum_xor->GetInstancePortSet("X").begin())->centre();
    layout->MakePin("P", start, "li.pin");
  }
 
  // Generate inputs.

  // TODO(aryap): These probably need to be broken out to more convenient pins
  // given where this cell is likely to go. Same with the P input and S output.
  layout->MakePin(
      "G_0",
      (*generate_select->GetInstancePortSet("A0").rbegin())->centre(),
      "li.pin");
  layout->MakePin(
      "G_1",
      (*generate_select->GetInstancePortSet("A1").rbegin())->centre(),
      "li.pin");

  // Config pins.
  layout->MakePin("CONFIG_CLK",
                  (*config_memory->GetInstancePortSet("CLK").begin())->centre(),
                  "li.pin");
  layout->MakePin("CONFIG_IN",
                  (*config_memory->GetInstancePortSet("D").begin())->centre(),
                  "li.pin");
  layout->MakePin("CONFIG_OUT",
                  (*config_memory->GetInstancePortSet("Q").begin())->centre(),
                  "li.pin");

  // Generate circuit.
  GenerateCircuit(row.generated_taps(),
                  config_memory,
                  generate_select,
                  carry_select,
                  sum_xor,
                  circuit);

  return cell.release();
}

geometry::Instance * Carry1::AddConfigMemory(RowGuide *row) const {
  std::string template_name = "config_memory";
  std::string instance_name = absl::StrCat(template_name, "_i");
  atoms::Sky130Dfxtp::Parameters params = {
    .input_clock_buffer = true,
    .add_inverted_output_port = false
  };
  atoms::Sky130Dfxtp generator(params, design_db_);
  Cell *register_cell = generator.GenerateIntoDatabase(template_name);
  geometry::Instance *installed = row->InstantiateBack(
      instance_name, register_cell);
  return installed;
}

geometry::Instance* Carry1::AddGenerateSelectMux(RowGuide *row) const {
  std::string template_name = "generate_select";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130HdMux21 mux_generator({}, design_db_);
  Cell *mux_cell = mux_generator.GenerateIntoDatabase(template_name);
  mux_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, mux_cell);
  return installed;
}

geometry::Instance* Carry1::AddSumXor(RowGuide *row) const {
  std::string template_name = "sum_xor2";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130Xor2 xor_generator({}, design_db_);
  Cell *xor_cell = xor_generator.GenerateIntoDatabase(template_name);
  xor_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, xor_cell);
  return installed;
}

geometry::Instance* Carry1::AddCarrySelectMux(RowGuide *row) const {
  std::string template_name = "carry_select";
  std::string instance_name = absl::StrCat(template_name, "_i");
  // Default params.
  atoms::Sky130HdMux21 mux_generator({}, design_db_);
  Cell *mux_cell = mux_generator.GenerateIntoDatabase(template_name);
  mux_cell->layout()->ResetY();
  geometry::Instance *installed = row->InstantiateBack(instance_name, mux_cell);
  return installed;
}

void Carry1::GenerateCircuit(const std::vector<geometry::Instance*> &taps,
                             geometry::Instance *config_memory,
                             geometry::Instance *generate_select,
                             geometry::Instance *carry_select,
                             geometry::Instance *sum_xor,
                             bfg::Circuit *circuit) const {
  circuit::Wire VPWR = circuit->AddSignal("VPWR");
  circuit::Wire VGND = circuit->AddSignal("VGND");

  circuit::Wire C_I = circuit->AddSignal("C_I");
  circuit::Wire P = circuit->AddSignal("P");
  circuit::Wire G_0 = circuit->AddSignal("G_0");
  circuit::Wire G_1 = circuit->AddSignal("G_1");
  circuit::Wire CONFIG_IN = circuit->AddSignal("CONFIG_IN");
  circuit::Wire CONFIG_CLK = circuit->AddSignal("CONFIG_CLK");

  circuit::Wire S = circuit->AddSignal("S");
  circuit::Wire C_O = circuit->AddSignal("C_O");
  circuit::Wire CONFIG_OUT = circuit->AddSignal("CONFIG_OUT");

  circuit->AddPort(C_I);
  circuit->AddPort(P);
  circuit->AddPort(G_0);
  circuit->AddPort(G_1);
  circuit->AddPort(CONFIG_IN);
  circuit->AddPort(CONFIG_CLK);
  circuit->AddPort(S);
  circuit->AddPort(C_O);
  circuit->AddPort(CONFIG_OUT);

  circuit::Wire i_0 = circuit->AddSignal("i_0");

  config_memory->circuit_instance()->Connect({
      {"D", CONFIG_IN},
      {"Q", CONFIG_OUT},
      {"CLK", CONFIG_CLK}});

  generate_select->circuit_instance()->Connect({
      {"A0", G_0},
      {"A1", G_1},
      {"S", CONFIG_OUT},
      {"X", i_0}});

  carry_select->circuit_instance()->Connect({
      {"A0", C_I},
      {"A1", i_0},
      {"S", P},
      {"X", C_O}});

  sum_xor->circuit_instance()->Connect({
      {"A", P},
      {"B", C_I},
      {"X", S}});

  std::array<circuit::Instance*, 4> all_powered = {
      config_memory->circuit_instance(),
      generate_select->circuit_instance(),
      carry_select->circuit_instance(),
      sum_xor->circuit_instance()};
  for (circuit::Instance *sub_circuit : all_powered) {
    sub_circuit->Connect({{"VPWR", VPWR},
                          {"VPB", VPWR},
                          {"VGND", VGND},
                          {"VNB", VGND}});
  }

  for (geometry::Instance *tap : taps) {
    tap->circuit_instance()->Connect("VPWR", VPWR);
    tap->circuit_instance()->Connect("VGND", VGND);
  }
}

}   // namespace tiles
}   // namespace bfg
