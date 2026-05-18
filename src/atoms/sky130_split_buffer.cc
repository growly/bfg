#include "sky130_split_buffer.h"

#include <memory>
#include <string>

#include "atom.h"
#include "../cell.h"
#include "../circuit/wire.h"
#include "../geometry/point.h"
#include "../geometry/rectangle.h"
#include "../layout.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Rectangle;

void Sky130SplitBuffer::Parameters::ToProto(
    proto::parameters::Sky130SplitBuffer *pb) const {
  pb->set_width_nm(width_nm);
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
  if (pb.has_width_nm()) width_nm = pb.width_nm();
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
    .stacks_left = false,
    .stacks_right = false
  };
  Sky130SimpleTransistor::Parameters base_pfet_params = {
  };
  //  .fet_type = Sky130SimpleTransistor::Parameters::FetType::NMOS,
  //  .length_nm = parameters_.n_length_nm,
  //  .stacks_left = parameters_.stacks_left,
  //  .stacks_right = parameters_.stacks_right,
  //  .poly_overhang_top_nm =
  //      (parameters_.n_tab_position &&
  //          geometry::CompassHasNorth(*parameters_.n_tab_position)) ?
  //          parameters_.min_n_tab_diff_separation_nm : std::nullopt,
  //  .poly_overhang_bottom_nm = 
  //      (parameters_.n_tab_position &&
  //          geometry::CompassHasSouth(*parameters_.n_tab_position)) ?
  //          parameters_.min_n_tab_diff_separation_nm : std::nullopt,
  //  .stacking_pitch_nm = parameters_.poly_pitch_nm
  //};

  //Sky130SimpleTransistor::Parameters pfet_params = {
  //  .fet_type = Sky130SimpleTransistor::Parameters::FetType::PMOS,
  //  .width_nm = parameters_.p_width_nm,
  //  .length_nm = parameters_.p_length_nm,
  //  .stacks_left = parameters_.stacks_left,
  //  .stacks_right = parameters_.stacks_right,
  //  .poly_overhang_top_nm =
  //      (parameters_.p_tab_position &&
  //          geometry::CompassHasNorth(*parameters_.p_tab_position)) ?
  //          parameters_.min_p_tab_diff_separation_nm : std::nullopt,
  //  .poly_overhang_bottom_nm = 
  //      (parameters_.p_tab_position &&
  //          geometry::CompassHasSouth(*parameters_.p_tab_position)) ?
  //          parameters_.min_p_tab_diff_separation_nm : std::nullopt,
  //  .stacking_pitch_nm = parameters_.poly_pitch_nm
  //}; 
 
  Sky130SimpleTransistor::Parameters nfet_0a_params = base_nfet_params;
  nfet_0a_params.length_nm = parameters_.nfet_0_length_nm;
  nfet_0a_params.width_nm = parameters_.nfet_0_width_nm;

  Sky130SimpleTransistor::Parameters pfet_0a_params = base_pfet_params;
  pfet_0a_params.length_nm = parameters_.pfet_0_length_nm;
  pfet_0a_params.width_nm = parameters_.pfet_0_width_nm;

  if (parameters_.double_fet0) {
    Sky130SimpleTransistor::Parameters nfet_0b_params = base_nfet_params;
    nfet_0b_params.length_nm = parameters_.nfet_0_length_nm;
    nfet_0b_params.width_nm = parameters_.nfet_0_width_nm;
    nfet_0b_params.stacks_right = true;
    nfet_0a_params.stacks_left = true;

    Sky130SimpleTransistor::Parameters pfet_0b_params = base_pfet_params;
    pfet_0b_params.length_nm = parameters_.pfet_0_length_nm;
    pfet_0b_params.width_nm = parameters_.pfet_0_width_nm;
    pfet_0b_params.stacks_right = true;
    pfet_0a_params.stacks_left = true;

    nfet_0b_gen_.reset(new Sky130SimpleTransistor(nfet_0b_params, design_db_));
    pfet_0b_gen_.reset(new Sky130SimpleTransistor(pfet_0b_params, design_db_));
  }
  nfet_0a_gen_.reset(new Sky130SimpleTransistor(nfet_0a_params, design_db_));
  pfet_0a_gen_.reset(new Sky130SimpleTransistor(pfet_0a_params, design_db_));

  Sky130SimpleTransistor::Parameters nfet_1_params = base_nfet_params;
  nfet_1_params.length_nm = parameters_.nfet_1_length_nm;
  nfet_1_params.width_nm = parameters_.nfet_1_width_nm;

  Sky130SimpleTransistor::Parameters pfet_1_params = base_pfet_params;
  pfet_1_params.length_nm = parameters_.pfet_1_length_nm;
  pfet_1_params.width_nm = parameters_.pfet_1_width_nm;

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
    nfet_2b_params.stacks_right = true;
    nfet_2a_params.stacks_left = true;

    Sky130SimpleTransistor::Parameters pfet_2b_params = base_pfet_params;
    pfet_2b_params.length_nm = parameters_.pfet_2_length_nm;
    pfet_2b_params.width_nm = parameters_.pfet_2_width_nm;
    pfet_2b_params.stacks_right = true;
    pfet_2a_params.stacks_left = true;

    nfet_2b_gen_.reset(new Sky130SimpleTransistor(nfet_2b_params, design_db_));
    pfet_2b_gen_.reset(new Sky130SimpleTransistor(pfet_2b_params, design_db_));
  }
  nfet_2a_gen_.reset(new Sky130SimpleTransistor(nfet_2a_params, design_db_));
  pfet_2a_gen_.reset(new Sky130SimpleTransistor(pfet_2a_params, design_db_));

  nfet_0a_gen_->set_name("nfet_0a");
  nfet_0b_gen_->set_name("nfet_0b");
  nfet_1_gen_->set_name("nfet_1");
  nfet_2a_gen_->set_name("nfet_2a");
  nfet_2b_gen_->set_name("nfet_2b");

  pfet_0a_gen_->set_name("pfet_0a");
  pfet_0b_gen_->set_name("pfet_0b");
  pfet_1_gen_->set_name("pfet_1");
  pfet_2a_gen_->set_name("pfet_2a");
  pfet_2b_gen_->set_name("pfet_2b");
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

bfg::Layout *Sky130SplitBuffer::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  int64_t width =
      design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  int64_t height =
      design_db_->physical_db().ToInternalUnits(parameters_.height_nm);

  nfet_0a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, 0});
  pfet_0a_gen_->AlignTransistorPartTo(
      Sky130SimpleTransistor::Landmark::POLY_BOTTOM_CENTRE,
      {0, 500});

  std::unique_ptr<bfg::Layout> nfet_0a_layout(nfet_0a_gen_->GenerateLayout());
  layout->AddLayout(*nfet_0a_layout, nfet_0a_gen_->name());
  std::unique_ptr<bfg::Layout> pfet_0a_layout(pfet_0a_gen_->GenerateLayout());
  layout->AddLayout(*pfet_0a_layout, pfet_0a_gen_->name());

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

  // TODO(aryap): Add diffusion, poly, li, licon, npc, nwell, psdm, nsdm,
  // hvtp shapes and pin annotations for A, P, X.

  return layout.release();
}

}   // namespace atoms
}   // namespace bfg
