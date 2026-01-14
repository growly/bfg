#include "interconnect_wire_block.h"

#include <string>

#include <absl/strings/str_format.h>

#include "proto/parameters/interconnect_wire_block.pb.h"
#include "../routing_track_direction.h"

namespace bfg {
namespace tiles {

void InterconnectWireBlock::Parameters::ToProto(
    proto::parameters::InterconnectWireBlock *pb) const {
  // TODO(aryap): Complete.
}

void InterconnectWireBlock::Parameters::FromProto(
    const proto::parameters::InterconnectWireBlock &pb) {
  // TODO(aryap): Complete.
}

void InterconnectWireBlock::DetermineLayersAndRules(
    const RoutingTrackDirection &main_direction) {

}

Cell *InterconnectWireBlock::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  // Laying out a bundle is just drawing N wires in the right direction.
  for (const auto &channel : parameters_.channels) {
    for (size_t i = 0; i < channel.num_bundles; ++i) {
      geometry::PolyLine line = ;
    }
  }

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
