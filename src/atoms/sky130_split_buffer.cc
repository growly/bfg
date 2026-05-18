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

bfg::Cell *Sky130SplitBuffer::Generate() {
  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_split_buffer" : name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());
  return cell.release();
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
  // When double_nfet2 is true, two parallel instances are used.
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
  if (parameters_.double_nfet2) {
    AddFet2("b");
  }

  // Bottom-branch first inverter (pfet_0/nfet_0): A -> P.
  // These transistors are placed on the left side of the cell and drive P in
  // parallel with pfet_1/nfet_1 to increase drive strength on the intermediate
  // node. When double_nfet0 is true, two parallel instances are used.
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
  if (parameters_.double_nfet0) {
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
