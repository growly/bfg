#include "sky130_split_buffer.h"

#include <memory>
#include <string>

#include "atom.h"
#include "../cell.h"
#include "../scoped_layer.h"
#include "../circuit/wire.h"
#include "../geometry/point.h"
#include "../geometry/rectangle.h"
#include "../layout.h"
#include "../utility.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Polygon;

void Sky130SplitBuffer::Parameters::ToProto(
    proto::parameters::Sky130SplitBuffer *pb) const {
  //pb->set_width_nm(width_nm);
  pb->set_height_nm(height_nm);

  pb->set_nfet_0_width_nm(nfet_0_width_nm);
  pb->set_nfet_1_width_nm(nfet_1_width_nm);
  pb->set_nfet_2_width_nm(nfet_2_width_nm);
  pb->set_pfet_0_width_nm(pfet_0_width_nm);
  pb->set_pfet_1_width_nm(pfet_1_width_nm);
  pb->set_pfet_2_width_nm(pfet_2_width_nm);

  pb->set_nfet_0_length_nm(nfet_0_length_nm);
  pb->set_nfet_1_length_nm(nfet_1_length_nm);
  pb->set_nfet_2_length_nm(nfet_2_length_nm);
  pb->set_pfet_0_length_nm(pfet_0_length_nm);
  pb->set_pfet_1_length_nm(pfet_1_length_nm);
  pb->set_pfet_2_length_nm(pfet_2_length_nm);

  pb->set_double_fet0(double_fet0);
  pb->set_double_fet2(double_fet2);
}

void Sky130SplitBuffer::Parameters::FromProto(
    const proto::parameters::Sky130SplitBuffer &pb) {
  //if (pb.has_width_nm()) width_nm = pb.width_nm();
  if (pb.has_height_nm()) height_nm = pb.height_nm();

  if (pb.has_nfet_0_width_nm()) nfet_0_width_nm = pb.nfet_0_width_nm();
  if (pb.has_nfet_1_width_nm()) nfet_1_width_nm = pb.nfet_1_width_nm();
  if (pb.has_nfet_2_width_nm()) nfet_2_width_nm = pb.nfet_2_width_nm();
  if (pb.has_pfet_0_width_nm()) pfet_0_width_nm = pb.pfet_0_width_nm();
  if (pb.has_pfet_1_width_nm()) pfet_1_width_nm = pb.pfet_1_width_nm();
  if (pb.has_pfet_2_width_nm()) pfet_2_width_nm = pb.pfet_2_width_nm();

  if (pb.has_nfet_0_length_nm()) nfet_0_length_nm = pb.nfet_0_length_nm();
  if (pb.has_nfet_1_length_nm()) nfet_1_length_nm = pb.nfet_1_length_nm();
  if (pb.has_nfet_2_length_nm()) nfet_2_length_nm = pb.nfet_2_length_nm();
  if (pb.has_pfet_0_length_nm()) pfet_0_length_nm = pb.pfet_0_length_nm();
  if (pb.has_pfet_1_length_nm()) pfet_1_length_nm = pb.pfet_1_length_nm();
  if (pb.has_pfet_2_length_nm()) pfet_2_length_nm = pb.pfet_2_length_nm();

  if (pb.has_double_fet0()) double_fet0 = pb.double_fet0();
  if (pb.has_double_fet2()) double_fet2 = pb.double_fet2();
}

bfg::Cell *Sky130SplitBuffer::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_split_buffer" : name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
}

void Sky130SplitBuffer::SetUpTransistors() {
  Sky130SimpleTransistor::Parameters base_nfet_params = {
    .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
    .stacks_left = false,
    .stacks_right = false,
    .stacking_pitch_nm = parameters_.poly_pitch_nm
  };
  Sky130SimpleTransistor::Parameters base_pfet_params = {
    .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
    .stacks_left = false,
    .stacks_right = false,
    .stacking_pitch_nm = parameters_.poly_pitch_nm
  };
 
  Sky130SimpleTransistor::Parameters nfet_0a_params = base_nfet_params;
  nfet_0a_params.length_nm = parameters_.nfet_0_length_nm;
  nfet_0a_params.width_nm = parameters_.nfet_0_width_nm;
  nfet_0a_params.stacks_right = true;

  Sky130SimpleTransistor::Parameters pfet_0a_params = base_pfet_params;
  pfet_0a_params.length_nm = parameters_.pfet_0_length_nm;
  pfet_0a_params.width_nm = parameters_.pfet_0_width_nm;
  pfet_0a_params.stacks_right = true;

  if (parameters_.double_fet0) {
    Sky130SimpleTransistor::Parameters nfet_0b_params = base_nfet_params;
    nfet_0b_params.length_nm = parameters_.nfet_0_length_nm;
    nfet_0b_params.width_nm = parameters_.nfet_0_width_nm;
    nfet_0b_params.stacks_right = true;
    nfet_0b_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;

    Sky130SimpleTransistor::Parameters pfet_0b_params = base_pfet_params;
    pfet_0b_params.length_nm = parameters_.pfet_0_length_nm;
    pfet_0b_params.width_nm = parameters_.pfet_0_width_nm;
    pfet_0b_params.stacks_right = true;
    pfet_0b_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;

    nfet_0a_params.stacks_left = true;
    nfet_0a_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;
    pfet_0a_params.stacks_left = true;
    pfet_0a_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;

    nfet_0b_gen_.reset(new Sky130SimpleTransistor(nfet_0b_params, design_db_));
    pfet_0b_gen_.reset(new Sky130SimpleTransistor(pfet_0b_params, design_db_));
  }
  nfet_0a_gen_.reset(new Sky130SimpleTransistor(nfet_0a_params, design_db_));
  pfet_0a_gen_.reset(new Sky130SimpleTransistor(pfet_0a_params, design_db_));

  Sky130SimpleTransistor::Parameters nfet_1_params = base_nfet_params;
  nfet_1_params.length_nm = parameters_.nfet_1_length_nm;
  nfet_1_params.width_nm = parameters_.nfet_1_width_nm;
  nfet_1_params.stacks_left = true;

  Sky130SimpleTransistor::Parameters pfet_1_params = base_pfet_params;
  pfet_1_params.length_nm = parameters_.pfet_1_length_nm;
  pfet_1_params.width_nm = parameters_.pfet_1_width_nm;
  pfet_1_params.stacks_left = true;

  nfet_1_gen_.reset(new Sky130SimpleTransistor(nfet_1_params, design_db_));
  pfet_1_gen_.reset(new Sky130SimpleTransistor(pfet_1_params, design_db_));

  Sky130SimpleTransistor::Parameters nfet_2a_params = base_nfet_params;
  nfet_2a_params.length_nm = parameters_.nfet_2_length_nm;
  nfet_2a_params.width_nm = parameters_.nfet_2_width_nm;

  Sky130SimpleTransistor::Parameters pfet_2a_params = base_pfet_params;
  pfet_2a_params.length_nm = parameters_.pfet_2_length_nm;
  pfet_2a_params.width_nm = parameters_.pfet_2_width_nm;

  if (parameters_.double_fet2) {
    Sky130SimpleTransistor::Parameters nfet_2b_params = base_nfet_params;
    nfet_2b_params.length_nm = parameters_.nfet_2_length_nm;
    nfet_2b_params.width_nm = parameters_.nfet_2_width_nm;
    nfet_2b_params.stacks_left = true;

    Sky130SimpleTransistor::Parameters pfet_2b_params = base_pfet_params;
    pfet_2b_params.length_nm = parameters_.pfet_2_length_nm;
    pfet_2b_params.width_nm = parameters_.pfet_2_width_nm;
    pfet_2b_params.stacks_left = true;

    nfet_2a_params.stacks_right = true;
    nfet_2a_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;
    pfet_2a_params.stacks_right = true;
    pfet_2a_params.stacking_pitch_nm = parameters_.split_poly_pitch_nm;

    nfet_2b_gen_.reset(new Sky130SimpleTransistor(nfet_2b_params, design_db_));
    pfet_2b_gen_.reset(new Sky130SimpleTransistor(pfet_2b_params, design_db_));
  }
  nfet_2a_gen_.reset(new Sky130SimpleTransistor(nfet_2a_params, design_db_));
  pfet_2a_gen_.reset(new Sky130SimpleTransistor(pfet_2a_params, design_db_));

  nfet_0a_gen_->set_name("nfet_0a");
  if (nfet_0b_gen_) nfet_0b_gen_->set_name("nfet_0b");
  nfet_1_gen_->set_name("nfet_1");
  nfet_2a_gen_->set_name("nfet_2a");
  if (nfet_2b_gen_) nfet_2b_gen_->set_name("nfet_2b");

  pfet_0a_gen_->set_name("pfet_0a");
  if (pfet_0b_gen_) pfet_0b_gen_->set_name("pfet_0b");
  pfet_1_gen_->set_name("pfet_1");
  pfet_2a_gen_->set_name("pfet_2a");
  if (pfet_2b_gen_) pfet_2b_gen_->set_name("pfet_2b");
}

bfg::Circuit *Sky130SplitBuffer::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  // Ports.
  circuit::Wire A    = circuit->AddSignal("A");
  circuit::Wire P    = circuit->AddSignal("P");
  circuit::Wire X    = circuit->AddSignal("X");
  circuit::Wire Xb   = circuit->AddSignal("Xb");
  circuit::Wire VPWR = circuit->AddSignal(parameters_.power_net);
  circuit::Wire VGND = circuit->AddSignal(parameters_.ground_net);
  circuit::Wire VPB  = circuit->AddSignal("VPB");
  circuit::Wire VNB  = circuit->AddSignal("VNB");

  circuit->AddPort(A);
  circuit->AddPort(P);
  circuit->AddPort(X);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  bfg::Circuit *nfet_01v8 =
      design_db_->FindCellOrDie("sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  bfg::Circuit *pfet_01v8_hvt =
      design_db_->FindCellOrDie(
          "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();

  // Top-branch inverter (pfet_1/nfet_1): A -> P = ~A.
  // P is also an output pin so the caller can use the complement directly.
  circuit::Instance *pfet_1 = circuit->AddInstance("pfet_1", pfet_01v8_hvt);
  circuit::Instance *nfet_1 = circuit->AddInstance("nfet_1", nfet_01v8);

  pfet_1->SetParameter(
      parameters_.fet_model_width_parameter,
      Parameter::FromInteger(
          parameters_.fet_model_width_parameter,
          static_cast<int64_t>(parameters_.pfet_1_width_nm),
          Parameter::SIUnitPrefix::NANO));
  pfet_1->SetParameter(
      parameters_.fet_model_length_parameter,
      Parameter::FromInteger(
          parameters_.fet_model_length_parameter,
          static_cast<int64_t>(parameters_.pfet_1_length_nm),
          Parameter::SIUnitPrefix::NANO));
  nfet_1->SetParameter(
      parameters_.fet_model_width_parameter,
      Parameter::FromInteger(
          parameters_.fet_model_width_parameter,
          static_cast<int64_t>(parameters_.nfet_1_width_nm),
          Parameter::SIUnitPrefix::NANO));
  nfet_1->SetParameter(
      parameters_.fet_model_length_parameter,
      Parameter::FromInteger(
          parameters_.fet_model_length_parameter,
          static_cast<int64_t>(parameters_.nfet_1_length_nm),
          Parameter::SIUnitPrefix::NANO));

  pfet_1->Connect({{"d", Xb}, {"g", A}, {"s", VPWR}, {"b", VPB}});
  nfet_1->Connect({{"d", Xb}, {"g", A}, {"s", VGND}, {"b", VNB}});

  // Top-branch second inverter (pfet_2/nfet_2): P -> X = ~~A.
  // When double_fet2 is true, two parallel instances are used.
  auto AddFet2 = [&](const std::string &suffix) {
    circuit::Instance *pfet = circuit->AddInstance(
        "pfet_2" + suffix, pfet_01v8_hvt);
    circuit::Instance *nfet = circuit->AddInstance(
        "nfet_2" + suffix, nfet_01v8);
    pfet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(parameters_.pfet_2_width_nm),
            Parameter::SIUnitPrefix::NANO));
    pfet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(parameters_.pfet_2_length_nm),
            Parameter::SIUnitPrefix::NANO));
    nfet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(parameters_.nfet_2_width_nm),
            Parameter::SIUnitPrefix::NANO));
    nfet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(parameters_.nfet_2_length_nm),
            Parameter::SIUnitPrefix::NANO));
    pfet->Connect({{"d", X}, {"g", Xb}, {"s", VPWR}, {"b", VPB}});
    nfet->Connect({{"d", X}, {"g", Xb}, {"s", VGND}, {"b", VNB}});
  };
  AddFet2("a");
  if (parameters_.double_fet2) {
    AddFet2("b");
  }

  // Bottom-branch first inverter (pfet_0/nfet_0): A -> P.
  // These transistors are placed on the left side of the cell and drive P in
  // parallel with pfet_1/nfet_1 to increase drive strength on the intermediate
  // node. When double_fet0 is true, two parallel instances are used.
  auto AddFet0 = [&](const std::string &suffix) {
    circuit::Instance *pfet = circuit->AddInstance(
        "pfet_0" + suffix, pfet_01v8_hvt);
    circuit::Instance *nfet = circuit->AddInstance(
        "nfet_0" + suffix, nfet_01v8);
    pfet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(parameters_.pfet_0_width_nm),
            Parameter::SIUnitPrefix::NANO));
    pfet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(parameters_.pfet_0_length_nm),
            Parameter::SIUnitPrefix::NANO));
    nfet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(parameters_.nfet_0_width_nm),
            Parameter::SIUnitPrefix::NANO));
    nfet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(parameters_.nfet_0_length_nm),
            Parameter::SIUnitPrefix::NANO));
    pfet->Connect({{"d", P}, {"g", A}, {"s", VPWR}, {"b", VPB}});
    nfet->Connect({{"d", P}, {"g", A}, {"s", VGND}, {"b", VNB}});
  };
  AddFet0("a");
  if (parameters_.double_fet0) {
    AddFet0("b");
  }

  return circuit.release();
}

int64_t Sky130SplitBuffer::FirstPolyX() const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  Sky130SimpleTransistor *first = nfet_0b_gen_ ?
      nfet_0b_gen_.get() : nfet_0a_gen_.get();

  return db.Rules("pdiff.drawing").min_separation / 2 +
      first->DiffWing(geometry::Compass::LEFT) +
      first->TransistorLength() / 2;
}

std::optional<Point> Sky130SplitBuffer::AverageViaLocations(
    const std::vector<TransistorAndViaLocation> &lookups) const {
  std::vector<Point> points;
  for (const auto &entry : lookups) {
    if (!entry.transistor)
      continue;
    points.push_back(entry.transistor->ViaLocation(entry.location));
  }

  if (points.empty())
    return std::nullopt;

  Point sum;
  for (const Point &point : points) {
    sum = sum + point;
  }
  return sum / points.size();
}

void Sky130SplitBuffer::DrawPolyA(
    Point *polycon_via, bfg::Layout *layout) const {
  geometry::Polygon poly_A;
  std::vector<std::pair<geometry::Point, uint64_t>> poly_A_tops = {
    {
      layout->GetPointOrDie("pfet_0a.poly_bottom_centre"),
      pfet_0a_gen_->TransistorLength()
    },
    {
      layout->GetPointOrDie("pfet_1.poly_bottom_centre"),
      pfet_1_gen_->TransistorLength()
    },
  };
  if (pfet_0b_gen_) {
    poly_A_tops.insert(poly_A_tops.begin(),
      {
        layout->GetPointOrDie("pfet_0b.poly_bottom_centre"),
        pfet_0b_gen_->TransistorLength()
      }
    );
  }

  Point nfet_0a_connect = layout->GetPointOrDie("nfet_0a.poly_top_centre");
  Point nfet_1_connect = layout->GetPointOrDie("nfet_1.poly_top_centre");
  std::vector<std::pair<geometry::Point, uint64_t>> poly_A_bottoms = {
    {
      nfet_0a_connect,
      nfet_0a_gen_->TransistorLength()
    },
    {
      nfet_1_connect,
      nfet_1_gen_->TransistorLength()
    },
  };
  if (nfet_0b_gen_) {
    poly_A_bottoms.insert(poly_A_bottoms.begin(),
      {
        layout->GetPointOrDie("nfet_0b.poly_top_centre"),
        nfet_0b_gen_->TransistorLength()
      }
    );
  }

  Point mid_left;
  int64_t polycon_via_x = (nfet_0a_connect.x() + nfet_1_connect.x()) / 2;
  auto vertices = AssemblePoly(poly_A_tops, poly_A_bottoms, 0, &mid_left);

  *polycon_via = {polycon_via_x, mid_left.y()};
  layout->MakeVia("polycon.drawing", *polycon_via);

  ScopedLayer sl(layout, "poly.drawing");
  layout->AddPolygon(Polygon(vertices));
}

void Sky130SplitBuffer::DrawPolyXb(
    Point *polycon_via, bfg::Layout *layout) const {
  geometry::Polygon poly_Xb;
  std::vector<std::pair<geometry::Point, uint64_t>> poly_Xb_tops = {
    {
      layout->GetPointOrDie("pfet_2a.poly_bottom_centre"),
      pfet_2a_gen_->TransistorLength()
    }
  };
  if (pfet_2b_gen_) {
    poly_Xb_tops.push_back(
      {
        layout->GetPointOrDie("pfet_2b.poly_bottom_centre"),
        pfet_2b_gen_->TransistorLength()
      }
    );
  }

  std::vector<std::pair<geometry::Point, uint64_t>> poly_Xb_bottoms = {
    {
      layout->GetPointOrDie("nfet_2a.poly_top_centre"),
      nfet_2b_gen_->TransistorLength()
    }
  };
  if (nfet_2b_gen_) {
    poly_Xb_bottoms.push_back(
      {
        layout->GetPointOrDie("nfet_2b.poly_top_centre"),
        nfet_2a_gen_->TransistorLength()
      }
    );
  }

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t poly_pitch = db.ToInternalUnits(parameters_.poly_pitch_nm);
  Point mid_left;
  auto vertices = AssemblePoly(
      poly_Xb_tops, poly_Xb_bottoms, poly_pitch, &mid_left);

  const auto &polycon_rules = db.Rules("polycon.drawing");
  const auto &poly_polycon_rules = db.Rules("poly.drawing", "polycon.drawing");
  int64_t via_centre_to_poly_edge = polycon_rules.via_width / 2 + std::max(
      poly_polycon_rules.via_overhang, poly_polycon_rules.via_overhang_wide);
  *polycon_via = {mid_left.x() + via_centre_to_poly_edge, mid_left.y()};
  layout->MakeVia("polycon.drawing", *polycon_via);

  ScopedLayer sl(layout, "poly.drawing");
  layout->AddPolygon(Polygon(vertices));
}

int64_t Sky130SplitBuffer::MapToTrackY(
    int64_t target_y, int64_t add_pitches) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t pitch = db.ToInternalUnits(
      parameters_.snap_ports_to_vertical_pitch_nm);
  if (pitch == 0) {
    return target_y;
  }
  int64_t offset_target_y = target_y - pitch / 2;
  int64_t low_error = (offset_target_y) -
      Utility::LastMultiple(offset_target_y, pitch);
  int64_t high_error =
      Utility::NextMultiple(offset_target_y, pitch) - (offset_target_y);
  int64_t mapped_y = 0;
  if (low_error <= high_error) {
    mapped_y = Utility::LastMultiple(offset_target_y, pitch) + pitch / 2;
  } else {
    mapped_y = Utility::NextMultiple(offset_target_y, pitch) + pitch / 2;
  }
  mapped_y += add_pitches * pitch;
  return mapped_y;
}

std::vector<geometry::Point> Sky130SplitBuffer::AssemblePoly(
    std::vector<std::pair<Point, uint64_t>> tops,
    std::vector<std::pair<Point, uint64_t>> bottoms,
    int64_t add_left,
    Point *mid_left) const {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  // Let's start by assuming both tops and bottoms and sorted in x. If that
  // doesn't hold we can just sort them ourselves.
  auto via_encap_info = db.TypicalViaEncap("poly.drawing", "polycon.drawing");
  int64_t poly_mid_height = std::min(
      via_encap_info.length, via_encap_info.width);

  int64_t min_top_y = tops.front().first.y();
  for (auto it = tops.begin() + 1; it < tops.end(); ++it) {
    min_top_y = std::min(min_top_y, it->first.y());
  }
  int64_t max_bottom_y = bottoms.front().first.y();
  for (auto it = bottoms.begin() + 1; it < bottoms.end(); ++it) {
    max_bottom_y = std::max(max_bottom_y, it->first.y());
  }
  int64_t mid_y = (min_top_y + max_bottom_y) / 2;

  int64_t poly_mid_top_y = min_top_y - mid_y > poly_mid_height / 2 ?
      mid_y + poly_mid_height / 2 : min_top_y;
  int64_t poly_mid_bottom_y = mid_y - max_bottom_y > poly_mid_height / 2 ?
      mid_y - poly_mid_height / 2 : max_bottom_y;

  std::vector<Point> vertices;

  int64_t left_x = tops[0].first.x();
  if (add_left > 0) {
    vertices.push_back({left_x - add_left, poly_mid_bottom_y});
    vertices.push_back({left_x - add_left, poly_mid_top_y});
    left_x -= add_left;
  }
  if (mid_left) *mid_left = Point(left_x, mid_y);

  for (size_t i = 0; i < tops.size(); ++i) {
    const Point &centre = tops[i].first;
    const uint64_t width = tops[i].second;

    int64_t left_x = centre.x() - width / 2; 
    int64_t right_x = centre.x() + width / 2; 

    if (i > 0 || add_left > 0) {
      vertices.push_back({left_x, poly_mid_top_y});
    }

    vertices.push_back({left_x, centre.y()});
    vertices.push_back({right_x, centre.y()});

    if (i < tops.size() - 1) {
      vertices.push_back({right_x, poly_mid_top_y});
    }
  }

  for (int i = bottoms.size() - 1; i >= 0; --i) {
    const Point &centre = bottoms[i].first;
    const uint64_t width = bottoms[i].second;

    int64_t left_x = centre.x() - width / 2; 
    int64_t right_x = centre.x() + width / 2; 

    if (i < bottoms.size() - 1) {
      vertices.push_back({right_x, poly_mid_bottom_y});
    }

    vertices.push_back({right_x, centre.y()});
    vertices.push_back({left_x, centre.y()});

    if (i > 0 || add_left > 0) {
      vertices.push_back({left_x, poly_mid_bottom_y});
    }
  }

  return vertices;
}

bfg::Layout *Sky130SplitBuffer::GenerateLayout() {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();

  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));

  int64_t height = db.ToInternalUnits(parameters_.height_nm);

  int64_t poly_low_y = db.ToInternalUnits(
      parameters_.min_poly_boundary_separation_nm);
  int64_t poly_high_y = height - poly_low_y;

  // TODO(aryap): Parameterise.
  int64_t poly_pitch = db.ToInternalUnits(parameters_.poly_pitch_nm);
  int64_t split_poly_pitch = db.ToInternalUnits(
      parameters_.split_poly_pitch_nm);

  // Align transistors.
  int64_t last_poly_x = FirstPolyX();

  if (nfet_0b_gen_ && pfet_0b_gen_) {
    nfet_0b_gen_->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
        {last_poly_x, poly_low_y});
    pfet_0b_gen_->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
        {last_poly_x, poly_high_y});
    last_poly_x += split_poly_pitch;
  }
  nfet_0a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {last_poly_x, poly_low_y});
  pfet_0a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
      {last_poly_x, poly_high_y});
  last_poly_x += poly_pitch;

  nfet_1_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {last_poly_x, poly_low_y});
  pfet_1_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
      {last_poly_x, poly_high_y});

  // Next transistor must be placed to accomodate minimum distance between diff
  // wings.
  int64_t min_diff_x = nfet_1_gen_->TransistorLength() / 2 +
      nfet_1_gen_->DiffWing(geometry::Compass::RIGHT) + 
      db.Rules("ndiff.drawing").min_separation +
      nfet_2a_gen_->DiffWing(geometry::Compass::LEFT) +
      nfet_2a_gen_->TransistorLength() / 2;

  //last_poly_x += Utility::NextMultiple(min_diff_x, poly_pitch);
  last_poly_x += min_diff_x;

  int64_t min_width = 0;

  nfet_2a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {last_poly_x, poly_low_y});
  pfet_2a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
      {last_poly_x, poly_high_y});
  if (nfet_2b_gen_ && pfet_2b_gen_) {
    last_poly_x += split_poly_pitch;
    nfet_2b_gen_->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
        {last_poly_x, poly_low_y});
    pfet_2b_gen_->AlignTransistorPartTo(
        Sky130SimpleTransistor::Landmark::POLY_TOP_CENTRE,
        {last_poly_x, poly_high_y});
    min_width = last_poly_x +
                nfet_2b_gen_->TransistorLength() / 2 +
                nfet_2b_gen_->DiffWing(geometry::Compass::RIGHT);
  } else {
    min_width = last_poly_x +
                nfet_2a_gen_->TransistorLength() / 2 +
                nfet_2a_gen_->DiffWing(geometry::Compass::RIGHT);
  }

  min_width += db.Rules("ndiff.drawing").min_separation / 2;

  std::optional<Rectangle> ndiff;
  std::array<Sky130SimpleTransistor*, 5> nmos_generators = {
    nfet_0a_gen_.get(),
    nfet_0b_gen_.get(),
    nfet_1_gen_.get(),
    nfet_2a_gen_.get(),
    nfet_2b_gen_.get()
  };
  for (Sky130SimpleTransistor *generator : nmos_generators) {
    if (!generator)
      continue;
    std::unique_ptr<bfg::Layout> transistor_layout(generator->GenerateLayout());
    layout->AddLayout(*transistor_layout, generator->name());

    if (!ndiff) {
      ndiff = generator->DiffBounds();
    } else {
      ndiff->ExpandToCover(generator->DiffBounds());
    }
  }

  std::optional<Rectangle> pdiff;
  std::array<Sky130SimpleTransistor*, 5> pmos_generators = {
    pfet_0a_gen_.get(),
    pfet_0b_gen_.get(),
    pfet_1_gen_.get(),
    pfet_2a_gen_.get(),
    pfet_2b_gen_.get()
  };
  for (Sky130SimpleTransistor *generator : pmos_generators) {
    if (!generator)
      continue;
    std::unique_ptr<bfg::Layout> transistor_layout(generator->GenerateLayout());
    layout->AddLayout(*transistor_layout, generator->name());

    if (!pdiff) {
      pdiff = generator->DiffBounds();
    } else {
      pdiff->ExpandToCover(generator->DiffBounds());
    }
  }

  // Draw poly connections for net A:
  Point polycon_via_A;
  DrawPolyA(&polycon_via_A, layout.get());

  // Complete the poly connections and the link for Xb:
  Point polycon_via_Xb;
  DrawPolyXb(&polycon_via_Xb, layout.get());

  int64_t li_Xb_left_x = 0;
  int64_t li_Xb_right_x = 0;
  int64_t li_Xb_bulge_y_max = 0;
  int64_t li_Xb_bulge_y_min = 0;
  // Connect output of p/nfet1 to input of p/nfet2a/b:
  {
    Point top_drain = pfet_1_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER);
    Point top_drain_low = pfet_1_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER);
    auto via_encap = db.TypicalViaEncap("li.drawing", "pcon.drawing");
    int64_t via_side = db.Rules("pcon.drawing").via_width;

    Point p0 = {top_drain.x() - via_side / 2,
                top_drain.y() + via_encap.width / 2};
    Point p1 = {top_drain.x() + via_encap.length / 2,
                top_drain.y() + via_encap.width / 2};

    via_encap = db.TypicalViaEncap("li.drawing", "polycon.drawing");
    Point p2 = {p1.x(), polycon_via_Xb.y() + via_encap.length / 2};
    Point p3 = {polycon_via_Xb.x() + via_encap.width / 2,
                polycon_via_Xb.y() + via_encap.length / 2};
    Point p4 = {polycon_via_Xb.x() + via_encap.width / 2,
                polycon_via_Xb.y() - via_encap.length / 2};
    Point p5 = {p1.x(), polycon_via_Xb.y() - via_encap.length / 2};

    Point bottom_drain = nfet_1_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER);
    Point bottom_drain_high = nfet_1_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER);
    via_encap = db.TypicalViaEncap("li.drawing", "ncon.drawing");
    Point p6 = {bottom_drain.x() + via_encap.length / 2,
                bottom_drain.y() - via_encap.width / 2};
    Point p7 = {bottom_drain.x() - via_side / 2,
                bottom_drain.y() - via_encap.width / 2};

    layout->DistributeVias(
        "pcon.drawing", top_drain, top_drain_low, std::nullopt, true);
    layout->DistributeVias(
        "ncon.drawing", bottom_drain, bottom_drain_high, std::nullopt, true);

    std::vector<Point> vertices = {p0, p1, p2, p3, p4, p5, p6, p7};
    ScopedLayer sl(layout.get(), "li.drawing");
    layout->AddPolygon(Polygon(vertices));

    li_Xb_left_x = p0.x();
    li_Xb_right_x = p1.x();
    li_Xb_bulge_y_max = p2.y();
    li_Xb_bulge_y_min = p5.y();
  }

  const auto polycon_encap_info =
      db.TypicalViaEncap("li.drawing", "polycon.drawing");
  const auto mcon_encap_info =
      db.TypicalViaEncap("li.drawing", "mcon.drawing");
  const auto pcon_encap_info = 
      db.TypicalViaEncap("li.drawing", "pcon.drawing");
  const auto ncon_encap_info = 
      db.TypicalViaEncap("li.drawing", "ncon.drawing");
  const auto &li_rules = db.Rules("li.drawing");

  int64_t li_P_bulge_y_max = 0;
  int64_t li_P_bulge_y_min = 0;
  int64_t li_P_left_x = 0;
  int64_t li_P_right_x = 0;
  Point port_P_centre;
  // Connect output of {p,n}fet0{a,b}:
  {
    auto via_encap = db.TypicalViaEncap("li.drawing", "pcon.drawing");
    int64_t via_side = db.Rules("pcon.drawing").via_width;

    int64_t top_x = pfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).x();
    if (pfet_0b_gen_) {
      top_x = (top_x + pfet_0b_gen_->ViaLocation(
          Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).x()) / 2;
    }
    int64_t top_y = pfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).y();
    int64_t top_y_low = pfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER).y();

    Point p0 = {top_x - via_encap.length / 2,
                top_y + via_encap.width / 2};
    Point p1 = {top_x + via_side / 2,
                top_y + via_encap.width / 2};

    via_encap = db.TypicalViaEncap("li.drawing", "polycon.drawing");

    int64_t bottom_x = nfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).x();
    if (nfet_0b_gen_) {
      bottom_x = (bottom_x + nfet_0b_gen_->ViaLocation(
          Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).x()) / 2;
    }
    int64_t bottom_y = nfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER).y();
    int64_t bottom_y_high = nfet_0a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).y();

    via_encap = db.TypicalViaEncap("li.drawing", "ncon.drawing");
    Point p2 = {bottom_x + via_side / 2,
                bottom_y - via_encap.width / 2};
    Point p3 = {bottom_x - via_encap.length / 2,
                bottom_y - via_encap.width / 2};

    // Find port P's centre y.
    int64_t port_P_y = MapToTrackY((p0.y() + p3.y()) / 2, 0);
    int64_t left_bounds_x = p3.x() - mcon_encap_info.MaxSide();

    // TODO(aryap): This is the minimum cover... but we want to fatten this up
    // as much as possible.
    int64_t li_bulge_max_y = pfet_0a_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER).y() -
        li_rules.min_separation - pcon_encap_info.MinSide() / 2;
    int64_t li_bulge_min_y = nfet_0a_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).y() +
        li_rules.min_separation + ncon_encap_info.MinSide() / 2;
 
    Point p4 = {p3.x(), std::min(
        li_bulge_min_y, port_P_y - mcon_encap_info.MinSide()/ 2)};
    Point p5 = {left_bounds_x, p4.y()};
    Point p6 = {p5.x(), std::max(
        li_bulge_max_y, p4.y() + mcon_encap_info.MinSide())};
    Point p7 = {p4.x(), p6.y()};

    Point bottom_drain = {bottom_x, bottom_y};
    Point bottom_drain_high = {bottom_x, bottom_y_high};
    Point top_drain = {top_x, top_y};
    Point top_drain_low = {top_x, top_y_low};

    layout->DistributeVias(
        "pcon.drawing", top_drain, top_drain_low, std::nullopt, true);
    layout->DistributeVias(
        "ncon.drawing", bottom_drain, bottom_drain_high, std::nullopt, true);

    std::vector<Point> vertices = {p0, p1, p2, p3, p4, p5, p6, p7};
    ScopedLayer sl(layout.get(), "li.drawing");
    layout->AddPolygon(Polygon(vertices));

    li_P_right_x = p1.x();
    li_P_left_x = p0.x();
    li_P_bulge_y_max = p7.y();
    li_P_bulge_y_min = p4.y();
    port_P_centre = {p5.x() + mcon_encap_info.MaxSide() / 2, port_P_y};
  }

  // Add port P.
  layout->MakePin("P", port_P_centre, "li.pin");

  Point port_X_centre;
  int64_t li_X_left_x = 0;
  int64_t li_X_right_x = 0;
  int64_t li_X_bulge_y_max = 0;
  int64_t li_X_bulge_y_min = 0;
  // Connect output of {p,n}fet2{a,b}:
  {
    auto via_encap = db.TypicalViaEncap("li.drawing", "pcon.drawing");
    int64_t via_side = db.Rules("pcon.drawing").via_width;

    int64_t top_x = pfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).x();
    if (pfet_2b_gen_) {
      top_x = (top_x + pfet_2b_gen_->ViaLocation(
          Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).x()) / 2;
    }
    int64_t top_y = pfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).y();
    int64_t top_y_low = pfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER).y();

    Point p0 = {top_x - via_encap.length / 2,
                top_y + via_encap.width / 2};
    Point p1 = {top_x + via_side / 2,
                top_y + via_encap.width / 2};

    int64_t bottom_x = nfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).x();
    if (nfet_2b_gen_) {
      bottom_x = (bottom_x + nfet_2b_gen_->ViaLocation(
          Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).x()) / 2;
    }
    int64_t bottom_y = nfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER).y();
    int64_t bottom_y_high = nfet_2a_gen_->ViaLocation(
        Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER).y();

    via_encap = db.TypicalViaEncap("li.drawing", "polycon.drawing");

    via_encap = db.TypicalViaEncap("li.drawing", "ncon.drawing");
    Point p6 = {bottom_x + via_side / 2,
                bottom_y - via_encap.width / 2};
    Point p7 = {bottom_x - via_encap.length / 2,
                bottom_y - via_encap.width / 2};

    Point bottom_drain = {bottom_x, bottom_y};
    Point bottom_drain_high = {bottom_x, bottom_y_high};
    Point top_drain = {top_x, top_y};
    Point top_drain_low = {top_x, top_y_low};

    layout->DistributeVias(
        "pcon.drawing", top_drain, top_drain_low, std::nullopt, true);
    layout->DistributeVias(
        "ncon.drawing", bottom_drain, bottom_drain_high, std::nullopt, true);

    int64_t port_X_y = MapToTrackY((p0.y() + p7.y()) / 2, 0);
    int64_t right_bounds_x = p1.x() + mcon_encap_info.MaxSide();

    int64_t li_bulge_max_y = pfet_2a_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER).y() -
        li_rules.min_separation - pcon_encap_info.MinSide() / 2;
    int64_t li_bulge_min_y = nfet_2a_gen_->ViaLocation(
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER).y() +
        li_rules.min_separation + ncon_encap_info.MinSide() / 2;

    Point p2 = {p1.x(), std::max(
        li_bulge_max_y, port_X_y + mcon_encap_info.MinSide() / 2)};
    Point p3 = {right_bounds_x, p2.y()};
    Point p4 = {right_bounds_x, std::min(
        li_bulge_min_y, p2.y() - mcon_encap_info.MinSide())};
    Point p5 = {p2.x(), p4.y()};

    std::vector<Point> vertices = {p0, p1, p2, p3, p4, p5, p6, p7};
    ScopedLayer sl(layout.get(), "li.drawing");
    layout->AddPolygon(Polygon(vertices));

    li_X_left_x = p0.x();
    li_X_right_x = p1.x();
    li_X_bulge_y_max = p2.y();
    li_X_bulge_y_min = p5.y();
    port_X_centre = {p1.x() + mcon_encap_info.MaxSide() / 2, port_X_y};
  }

  layout->MakePin("X", port_X_centre, "li.pin");

  // Draw the li.drawing encap and port A:
  // Now we want an li pour with an li pin on it for the A port.
  int64_t li_A_top_y = 0;
  int64_t li_A_bottom_y = 0;
  {
    int64_t port_y = MapToTrackY(polycon_via_A.y(), 1);
    Point port_A_centre = {polycon_via_A.x(), port_y};
    layout->MakePin("A", port_A_centre, "li.pin");
    
    Rectangle encap = Rectangle::CentredAt(
        polycon_via_A,
        polycon_encap_info.MinSide(),   // Width.
        polycon_encap_info.MaxSide());  // Height.
    encap.ExpandToCover(Rectangle::CentredAt(
        port_A_centre,
        mcon_encap_info.MinSide(),    // Width.
        mcon_encap_info.MaxSide()));  // Height.

    // "Manually" inflate the encap to the bounds set by the flanking li.drawing
    // pours.
    encap.lower_left().set_x(
        std::min(li_P_right_x + li_rules.min_separation,
                 encap.lower_left().x()));
    encap.upper_right().set_x(
        std::max(li_Xb_left_x - li_rules.min_separation,
                 encap.upper_right().x()));

    int64_t required_area = li_rules.min_area;
    LOG_IF(WARNING, encap.Area() < required_area)
        << "Encap area too small, "
        << "you should change this to do something about it: "
        << encap.Area() << " vs " << required_area;

    li_A_top_y = encap.upper_right().y();
    li_A_bottom_y = encap.lower_left().y();

    ScopedLayer sl(layout.get(), "li.drawing");
    layout->AddRectangle(encap);
  }

  int64_t width = Utility::NextMultiple(
      min_width,
      db.ToInternalUnits(Sky130Parameters::kStandardCellUnitWidthNm));

  // met1.drawing power rails.
  layout->SetActiveLayerByName("met1.drawing");
  Rectangle *vgnd_bar = layout->AddRectangle(
      Rectangle(Point(0, -240), width, 480));
  vgnd_bar->set_net(parameters_.ground_net);

  Rectangle *vpwr_bar = layout->AddRectangle(
      Rectangle(Point(0, height - 240), width, 480));
  vpwr_bar->set_net(parameters_.power_net);

  // areaid.standardc 81/4 - tiling boundary.
  layout->SetActiveLayerByName("areaid.standardc");
  Rectangle *tiling_bounds = layout->AddRectangle(
      Rectangle(Point(0, 0), width, height));
  layout->SetTilingBounds(*tiling_bounds);

  // li bars over power/ground.
  // li.drawing
  layout->SetActiveLayerByName("li.drawing");
  int64_t li_width = db.Rules("li.drawing").min_width;
  Rectangle *vpwr_li = layout->AddRectangle(
      Rectangle({0, height - li_width / 2}, width, li_width));
  Rectangle *vgnd_li = layout->AddRectangle(
      Rectangle({0, - li_width / 2}, width, li_width));

  // Connect VDD.
  struct RailBoundingInfo {
    int64_t top_y;
    int64_t bottom_y;
    int64_t left_x;
    int64_t right_x;
    std::vector<TransistorAndViaLocation> top_vias;
    std::vector<TransistorAndViaLocation> bottom_vias;
    std::string diff_connection_layer;
    ViaEncapInfo encap_info;
  };
  std::array<RailBoundingInfo, 8> li_rail_connections = {
    RailBoundingInfo {
      vpwr_li->lower_left().y(),
      li_P_bulge_y_max + li_rules.min_separation,
      0 + li_rules.min_separation,
      li_P_left_x - li_rules.min_separation,
      {
        {pfet_0b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {pfet_0b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      vpwr_li->lower_left().y(),
      li_A_top_y + li_rules.min_separation,
      li_P_right_x + li_rules.min_separation,
      li_Xb_left_x - li_rules.min_separation,
      {
        {pfet_0a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER},
        {pfet_1_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {pfet_0a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER},
        {pfet_1_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      vpwr_li->lower_left().y(),
      li_Xb_bulge_y_max + li_rules.min_separation,
      li_Xb_right_x + li_rules.min_separation,
      li_X_left_x - li_rules.min_separation,
      {
        {pfet_2a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {pfet_2a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      vpwr_li->lower_left().y(),
      li_X_bulge_y_max + li_rules.min_separation,
      li_X_right_x + li_rules.min_separation,
      width - li_rules.min_separation,
      {
        {pfet_2b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER}
      },
      {
        {pfet_2b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      li_P_bulge_y_min - li_rules.min_separation,
      vgnd_li->upper_right().y(),
      0 + li_rules.min_separation,
      li_P_left_x - li_rules.min_separation,
      {
        {nfet_0b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {nfet_0b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      li_A_bottom_y - li_rules.min_separation,
      vgnd_li->upper_right().y(),
      li_P_right_x + li_rules.min_separation,
      li_Xb_left_x - li_rules.min_separation,
      {
        {nfet_0a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER},
        {nfet_1_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {nfet_0a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER},
        {nfet_1_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      li_Xb_bulge_y_min - li_rules.min_separation,
      vgnd_li->upper_right().y(),
      li_Xb_right_x + li_rules.min_separation,
      li_X_left_x - li_rules.min_separation,
      {
        {nfet_2a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_UPPER}
      },
      {
        {nfet_2a_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::LEFT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
    RailBoundingInfo {
      li_X_bulge_y_min - li_rules.min_separation,
      vgnd_li->upper_right().y(),
      li_X_right_x + li_rules.min_separation,
      width - li_rules.min_separation,
      {
        {nfet_2b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_UPPER},
      },
      {
        {nfet_2b_gen_.get(),
            Sky130SimpleTransistor::ViaPosition::RIGHT_DIFF_LOWER}
      },
      "pcon.drawing",
      pcon_encap_info
    },
  };

  for (auto &info : li_rail_connections) {
    auto maybe_top_via = AverageViaLocations(info.top_vias);
    auto maybe_bottom_via = AverageViaLocations(info.bottom_vias);

    if (!maybe_top_via || !maybe_bottom_via)
      continue;

    Point top_via = {
      maybe_top_via->x(), std::min(info.top_y, maybe_top_via->y())};
    Point bottom_via = {
      maybe_bottom_via->x(), std::max(info.bottom_y, maybe_bottom_via->y())};

    Rectangle li_pour = Rectangle(
      {std::max(info.left_x, bottom_via.x() - info.encap_info.MaxSide() / 2),
          info.bottom_y},
      {std::min(info.right_x, bottom_via.x() + info.encap_info.MaxSide() / 2),
          info.top_y});

    layout->DistributeVias(
        info.diff_connection_layer, top_via, bottom_via, std::nullopt, true);

    ScopedLayer sl(layout.get(), "li.drawing");
    layout->AddRectangle(li_pour);
  }
  //
  // Connect VSS.

  // TODO(aryap): Add npc.

  int64_t mcon_via_pitch = db.ToInternalUnits(parameters_.mcon_via_pitch_nm);

  Rectangle *nwell_pin = nullptr;
  Rectangle *pwell_pin = nullptr;
  if (parameters_.draw_overflowing_vias_and_pins) {
    if (parameters_.draw_vpwr_vias) {
      layout->StampVias(
          "mcon.drawing",
          {vpwr_bar->lower_left().x(), vpwr_bar->centre().y()},
          {vpwr_bar->upper_right().x(), vpwr_bar->centre().y()},
          mcon_via_pitch);
    }
    if (parameters_.draw_vgnd_vias) {
      layout->StampVias(
          "mcon.drawing",
          {vgnd_bar->lower_left().x(), vgnd_bar->centre().y()},
          {vgnd_bar->upper_right().x(), vgnd_bar->centre().y()},
          mcon_via_pitch);
    }

    int64_t mcon_side = std::max(db.Rules("mcon.drawing").via_width,
                                 db.Rules("mcon.drawing").via_height);
    int64_t pin_x = mcon_via_pitch / 2;

    // met1.pin 68/16
    layout->SetActiveLayerByName("met1.pin");
    // Apply VPWR or VGND label with pin:
    layout->MakePin(
        parameters_.power_net, {230, static_cast<int64_t>(height)}, "met1.pin");
    layout->MakePin(
        parameters_.ground_net, {230, 0}, "met1.pin");

    // nwell.pin 64/16
    layout->SetActiveLayerByName("nwell.pin");
    nwell_pin =
        layout->AddSquare({pin_x, vpwr_bar->centre().y()}, mcon_side);
    nwell_pin->set_net("VPB");

    // pwell.pin 122/16
    layout->SetActiveLayerByName("pwell.pin");
    Rectangle *pwell_pin =
        layout->AddSquare({pin_x, vgnd_bar->centre().y()}, mcon_side);
    // FIXME(aryap): This breaks proto2gds?
    //pwell_pin->set_net("VNB");
  }

  // Add npc.drawing around polycon contacts.
  {
    int64_t via_side = db.Rules("polycon.drawing").via_width;
    Rectangle cover = Rectangle::CentredAt(
        polycon_via_A, via_side, via_side);
    cover.ExpandToCover(
        Rectangle::CentredAt(polycon_via_Xb, via_side, via_side));

    int64_t padding = db.Rules("polycon.drawing", "npc.drawing").min_enclosure;

    ScopedLayer sl(layout.get(), "npc.drawing");
    layout->AddRectangle(cover.WithPadding(padding));
  }

  int64_t nwell_y_max = nwell_pin ? nwell_pin->upper_right().y() : height;
  {
    ScopedLayer scoped_layer(layout.get(), "nwell.drawing");
    int64_t nwell_margin = db.Rules(
        "nwell.drawing", "pdiff.drawing").min_enclosure;
    Rectangle nwell_rectangle = pdiff->WithPadding(nwell_margin);
    // Extend the nwell to the top of the cell.
    nwell_rectangle.upper_right().set_y(nwell_y_max);
    if (parameters_.expand_wells_to_horizontal_bounds) {
      nwell_rectangle.ExpandHorizontallyToCover(*tiling_bounds);
    }
    layout->AddRectangle(nwell_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "psdm.drawing");
    int64_t psdm_margin = db.Rules(
        "psdm.drawing", "pdiff.drawing").min_enclosure;
    Rectangle psdm_rectangle = pdiff->WithPadding(psdm_margin);
    psdm_rectangle.upper_right().set_y(nwell_y_max);
    if (parameters_.expand_wells_to_horizontal_bounds) {
      psdm_rectangle.ExpandHorizontallyToCover(*tiling_bounds);
    }
    layout->AddRectangle(psdm_rectangle);
  }
  {
    ScopedLayer layer(layout.get(), "hvtp.drawing");
    int64_t hvtp_margin = db.Rules(
        "hvtp.drawing", "pdiff.drawing").min_enclosure;
    Rectangle hvtp_rectangle = pdiff->WithPadding(hvtp_margin);
    hvtp_rectangle.upper_right().set_y(nwell_y_max);
    layout->AddRectangle(hvtp_rectangle);
  }

  int64_t psdm_y_min = pwell_pin ? pwell_pin->lower_left().y() : 0;
  {
    ScopedLayer layer(layout.get(), "nsdm.drawing");
    int64_t nsdm_margin = db.Rules(
        "nsdm.drawing", "ndiff.drawing").min_enclosure;
    Rectangle nsdm_rectangle = ndiff->WithPadding(nsdm_margin);
    nsdm_rectangle.lower_left().set_y(psdm_y_min);
    if (parameters_.expand_wells_to_horizontal_bounds) {
      nsdm_rectangle.ExpandHorizontallyToCover(*tiling_bounds);
    }
    layout->AddRectangle(nsdm_rectangle);
  }

  return layout.release();
}

}   // namespace atoms
}   // namespace bfg
