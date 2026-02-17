#include "sky130_xor2.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../circuit/wire.h"
#include "../cell.h"
#include "../geometry/layer.h"
#include "../geometry/point.h"
#include "../geometry/polygon.h"
#include "../geometry/rectangle.h"
#include "../layout.h"

namespace bfg {
namespace atoms {

using ::bfg::geometry::Point;
using ::bfg::geometry::Polygon;
using ::bfg::geometry::Rectangle;
using ::bfg::geometry::Layer;

bfg::Cell *Sky130Xor2::Generate() {
  // A 2-input XOR gate implements the function:
  //               _   _
  //   X = A⊕B = A·B + A·B

  std::unique_ptr<bfg::Cell> cell(
      new bfg::Cell(name_.empty() ? "sky130_xor2": name_));
  cell->SetLayout(GenerateLayout());
  cell->SetCircuit(GenerateCircuit());

  return cell.release();
}

bfg::Circuit *Sky130Xor2::GenerateCircuit() {
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  circuit::Wire X = circuit->AddSignal("X");
  circuit::Wire A = circuit->AddSignal("A");
  circuit::Wire B = circuit->AddSignal("B");
  circuit::Wire VPWR = circuit->AddSignal(parameters_.power_net);
  circuit::Wire VGND = circuit->AddSignal(parameters_.ground_net);
  circuit::Wire VPB = circuit->AddSignal("VPB");
  circuit::Wire VNB = circuit->AddSignal("VNB");

  circuit->AddPort(X);
  circuit->AddPort(A);
  circuit->AddPort(B);
  circuit->AddPort(VPWR);
  circuit->AddPort(VGND);
  circuit->AddPort(VPB);
  circuit->AddPort(VNB);

  bfg::Circuit *nfet_01v8 =
      design_db_->FindCellOrDie("sky130", "sky130_fd_pr__nfet_01v8")->circuit();
  bfg::Circuit *pfet_01v8 =
      design_db_->FindCellOrDie(
          "sky130", "sky130_fd_pr__pfet_01v8_hvt")->circuit();

  // TODO(aryap): Implement XOR logic using transistors.
  // A typical XOR2 implementation would require multiple transistors
  // configured to implement X = A⊕B.
  //
  // For now, this is a placeholder structure.

  // nfet_0
  circuit::Instance *nfet_0 = circuit->AddInstance("nfet_0", nfet_01v8);
  // nfet_1
  circuit::Instance *nfet_1 = circuit->AddInstance("nfet_1", nfet_01v8);
  // nfet_2
  circuit::Instance *nfet_2 = circuit->AddInstance("nfet_2", nfet_01v8);
  // nfet_3
  circuit::Instance *nfet_3 = circuit->AddInstance("nfet_3", nfet_01v8);
  // nfet_4
  circuit::Instance *nfet_4 = circuit->AddInstance("nfet_4", nfet_01v8);

  // pfet_0
  circuit::Instance *pfet_0 = circuit->AddInstance("pfet_0", pfet_01v8);
  // pfet_1
  circuit::Instance *pfet_1 = circuit->AddInstance("pfet_1", pfet_01v8);
  // pfet_2
  circuit::Instance *pfet_2 = circuit->AddInstance("pfet_2", pfet_01v8);
  // pfet_3
  circuit::Instance *pfet_3 = circuit->AddInstance("pfet_3", pfet_01v8);
  // pfet_4
  circuit::Instance *pfet_4 = circuit->AddInstance("pfet_4", pfet_01v8);

  struct FetParameters {
    circuit::Instance *instance;
    uint64_t width_nm;
    uint64_t length_nm;
  };
  std::array<FetParameters, 10> fet_parameters = {
    FetParameters {
      nfet_0, parameters_.nfet_0_width_nm, parameters_.nfet_0_length_nm
    },
    FetParameters {
      nfet_1, parameters_.nfet_1_width_nm, parameters_.nfet_1_length_nm
    },
    FetParameters {
      nfet_2, parameters_.nfet_2_width_nm, parameters_.nfet_2_length_nm
    },
    FetParameters {
      nfet_3, parameters_.nfet_3_width_nm, parameters_.nfet_3_length_nm
    },
    FetParameters {
      nfet_4, parameters_.nfet_4_width_nm, parameters_.nfet_4_length_nm
    },
    FetParameters {
      pfet_0, parameters_.pfet_0_width_nm, parameters_.pfet_0_length_nm
    },
    FetParameters {
      pfet_1, parameters_.pfet_1_width_nm, parameters_.pfet_1_length_nm
    },
    FetParameters {
      pfet_2, parameters_.pfet_2_width_nm, parameters_.pfet_2_length_nm
    },
    FetParameters {
      pfet_3, parameters_.pfet_3_width_nm, parameters_.pfet_3_length_nm
    },
    FetParameters {
      pfet_4, parameters_.pfet_4_width_nm, parameters_.pfet_4_length_nm
    }
  };
  for (size_t i = 0; i < fet_parameters.size(); ++i) {
    circuit::Instance *fet = fet_parameters[i].instance;
    fet->SetParameter(
        parameters_.fet_model_width_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_width_parameter,
            static_cast<int64_t>(fet_parameters[i].width_nm),
            Parameter::SIUnitPrefix::NANO));
    fet->SetParameter(
        parameters_.fet_model_length_parameter,
        Parameter::FromInteger(
            parameters_.fet_model_length_parameter,
            static_cast<int64_t>(fet_parameters[i].length_nm),
            Parameter::SIUnitPrefix::NANO));
  }

  // TODO(aryap): Wire up the transistors to implement XOR logic.

  return circuit.release();
}

bfg::Layout *Sky130Xor2::GenerateLayout() {
  std::unique_ptr<bfg::Layout> layout(
      new bfg::Layout(design_db_->physical_db()));

  // TODO(arya): Might be useful to set this to a multiple of some unit value
  // (per usual std. cells).
  int64_t width =
      design_db_->physical_db().ToInternalUnits(parameters_.width_nm);
  int64_t height =
      design_db_->physical_db().ToInternalUnits(parameters_.height_nm);

  // met1.drawing 68/20
  // The second "metal" layer.
  layout->SetActiveLayerByName("met1.drawing");
  Rectangle *vgnd_bar = layout->AddRectangle(
      Rectangle(Point(0, -240), width, 480));
  vgnd_bar->set_net(parameters_.ground_net);

  Rectangle *vpwr_bar =
      layout->AddRectangle(Rectangle(Point(0, height - 240), width, 480));
  vpwr_bar->set_net(parameters_.power_net);

  // TODO(aryap): Implement full layout with diffusion, poly, contacts, etc.

  if (parameters_.draw_overflowing_vias_and_pins) {
    if (parameters_.draw_vpwr_vias) {
      // Metal to li1.drawing contacts (VPWR side).
      layout->MakeVia("mcon.drawing", {230,  static_cast<int64_t>(height)});
      layout->MakeVia("mcon.drawing", {690,  static_cast<int64_t>(height)});
      layout->MakeVia("mcon.drawing", {1150, static_cast<int64_t>(height)});
    }

    // met1.pin
    layout->SetActiveLayerByName("met1.pin");
    layout->MakePin("VPWR", {230, static_cast<int64_t>(height)}, "met1.pin");
    layout->MakePin("VGND", {230, 0}, "met1.pin");

    if (parameters_.draw_vgnd_vias) {
      // Metal to li1.drawing contacts (VGND side).
      layout->MakeVia("mcon.drawing", {230, 0});
      layout->MakeVia("mcon.drawing", {690, 0});
      layout->MakeVia("mcon.drawing", {1150, 0});
    }

    // nwell.pin 64/16
    layout->SetActiveLayerByName("nwell.pin");
    layout->AddSquare({230, static_cast<int64_t>(height)}, 170);

    // pwell.pin 122/16
    layout->SetActiveLayerByName("pwell.pin");
    layout->AddSquare({240, 0}, 170);
  }

  // areaid.standardc 81/4
  layout->SetActiveLayerByName("areaid.standardc");
  // Boundary for tiling; when abutting to others, this cannot be overlapped.
  Rectangle *tiling_bounds = layout->AddRectangle(
      Rectangle(Point(0, 0), width, height));
  layout->SetTilingBounds(*tiling_bounds);

  return layout.release();
}

}  // namespace atoms
}  // namespace bfg
