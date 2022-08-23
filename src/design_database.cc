#include "design_database.h"

#include <algorithm>
#include <absl/strings/str_join.h>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <memory>
#include <glog/logging.h>
#include <google/protobuf/text_format.h>
#include <vector>
#include <set>

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
  Cell *cell = nullptr;
  if (cells_it != cells_.end()) {
    cell = cells_it->second.get();
  } else {
    cell = new Cell();
    cell->set_name(module_pb.name());
    cells_.insert({module_pb.name(), std::unique_ptr<Cell>(cell)});
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

void DesignDatabase::ConsumeCell(Cell *cell) {
  auto it = cells_.find(cell->name());
  LOG_IF(FATAL, it != cells_.end())
      << "Cell " << cell->name() << " already exists in the design database.";
  cells_.insert({cell->name(), std::unique_ptr<Cell>(cell)});
}

Cell *DesignDatabase::FindCellOrDie(const std::string &name) const {
  auto it = cells_.find(name);
  if (it == cells_.end()) {
    LOG(FATAL) << "Cell not found: \"" << name << "\"";
    return nullptr;
  }
  Cell *cell = it->second.get();
  LOG_IF(FATAL, !cell)
      << "Cell found but it's nullptr: \"" << name << "\"";
  return cell;
}

// Place every cell in cells_ into *ordered_cells ensuring that every
// dependency of a later cell is placed earlier in the list. This does no
// pruning and no partitioning. We create a list of ancestor Cells for each
// Cell by looking at the cells instantiated in the layouts and circuits. We
// then use this as a partial ordering to sort the output list. The output list
// doesn't have any useful structure beyond the dependency ordering constraint.
void DesignDatabase::OrderCells(
    const std::vector<Cell*> &unordered_cells,
    std::vector<Cell*> *ordered_cells) {
  // Simpler than std::transform.
  ordered_cells->clear();
  ordered_cells->insert(
      ordered_cells->begin(), unordered_cells.begin(), unordered_cells.end());

  std::sort(ordered_cells->begin(), ordered_cells->end(),
      [&](Cell *left, Cell *right) {
    std::set<Cell*> ancestors = right->DirectAncestors();
    return ancestors.find(left) != ancestors.end();
  });
}

void DesignDatabase::WriteCellsToVLSIRLibrary(
    const std::vector<Cell*> ordered_cells,
    const std::string &file_name,
    bool include_text_format) {
  ::vlsir::raw::Library library;
  library.set_units(::vlsir::raw::Units::NANO);

  for (Cell *cell : ordered_cells) {
    *library.add_cells() = cell->ToVLSIRCell();
  }

  if (include_text_format) {
    std::string text_format;
    google::protobuf::TextFormat::PrintToString(library, &text_format);

    std::fstream text_format_output(
        file_name + ".txt",
        std::ios::out | std::ios::trunc | std::ios::binary);
    text_format_output << text_format;
    text_format_output.close();
  }

  std::fstream output_file(
      file_name, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!library.SerializeToOstream(&output_file)) {
    LOG(ERROR) << "Failed to write library";
  } else {
    LOG(INFO) << "Wrote library to " << file_name;
  }
}

void DesignDatabase::WriteTop(
    const std::string &top_name,
    const std::string &file_name,
    bool include_text_format) const {
  Cell *top = FindCellOrDie(top_name);
  WriteTop(*top, file_name, include_text_format);
}

void DesignDatabase::WriteTop(
    const Cell &top,
    const std::string &file_name,
    bool include_text_format) const {
  Cell *top_cell = const_cast<Cell*>(&top);
  std::set<Cell*> ancestors;
  std::vector<Cell*> reverse_ordered_cells;
  ancestors.insert(top_cell);
  reverse_ordered_cells.push_back(top_cell);

  // Graph-traversal to find the transitive closure of all ancestor Cells.
  std::deque<Cell*> to_visit;
  to_visit.push_back(top_cell);

  while (!to_visit.empty()) {
    Cell *cell = to_visit.front();
    to_visit.pop_front();
    std::set<Cell*> direct_ancestors = cell->DirectAncestors();
    for (Cell *ancestor : direct_ancestors) {
      if (ancestors.find(ancestor) != ancestors.end())
        continue;
      ancestors.insert(ancestor);
      reverse_ordered_cells.push_back(ancestor);
      to_visit.push_back(ancestor);
    }
  }

  std::vector<Cell*> cells(ancestors.begin(), ancestors.end());
  std::vector<Cell*> ordered_cells(
      reverse_ordered_cells.rbegin(), reverse_ordered_cells.rend());
  WriteCellsToVLSIRLibrary(ordered_cells, file_name, include_text_format);
}

}  // namespace bfg
