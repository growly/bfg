#include "design_database.h"

#include <memory>
#include <glog/logging.h>

#include "cell.h"
#include "vlsir/tech.pb.h"
#include "vlsir/circuit.pb.h"

namespace bfg {

void DesignDatabase::LoadPackage(const vlsir::circuit::Package &package) {
  size_t num_modules = 0;
  for (const auto &module_pb : package.modules()) {
    LoadModule(module_pb);
    ++num_modules;
  }
  size_t num_ext_modules = 0;
  for (const auto &module_pb : package.ext_modules()) {
    LoadExternalModule(module_pb);
    ++num_ext_modules;
  }
  LOG(INFO) << "Loaded " << num_modules << " modules and "
            << num_ext_modules << " external modules from package "
            << "(domain: \"" << package.domain() << "\")";
}

void DesignDatabase::LoadModule(const vlsir::circuit::Module &module_pb) {
  // Get any existing cell being referenced:
  auto cells_it = cells_.find(module_pb.name());
  bfg::Cell *cell = nullptr;
  if (cells_it != cells_.end()) {
    cell = cells_it->second.get();
  } else {
    cell = new bfg::Cell();
    cells_.insert({module_pb.name(), std::unique_ptr<bfg::Cell>(cell)});
  }

  cell->SetCircuit(Circuit::FromVLSIRModule(module_pb));
  VLOG(2) << "Loaded module \"" << module_pb.name() << "\"";
}

void DesignDatabase::LoadExternalModule(
    const vlsir::circuit::ExternalModule &module_pb) {
  LOG(WARNING) << "TODO: External module not loaded: "
               << module_pb.name().domain() << " "
               << module_pb.name().name();
}

void DesignDatabase::AddCell(bfg::Cell *cell) {
  auto it = cells_.find(cell->name());
  LOG_IF(FATAL, it != cells_.end())
      << "Cell " << cell->name() << " already exists in the design database.";
  cells_.insert({cell->name(), std::unique_ptr<bfg::Cell>(cell)});
}

}  // namespace bfg
