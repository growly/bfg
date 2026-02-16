#include "carry1.h"

#include "../cell.h"
#include "proto/parameters/carry1.pb.h"

namespace bfg {
namespace tiles {

void Carry1::Parameters::ToProto(proto::parameters::Carry1 *pb) const {

}

void Carry1::Parameters::FromProto(const proto::parameters::Carry1 &pb) {
}

Cell *Carry1::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));
  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
