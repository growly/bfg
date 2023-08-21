#include "design_database.h"

#include <algorithm>
#include <absl/strings/str_join.h>
#include <deque>
#include <sstream>
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

namespace {

std::string JoinDomainAndName(const std::string &domain,
                              const std::string &name) {
  return absl::StrCat(domain, "/", name);
}

}  // namespace

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
  Circuit *circuit = Circuit::FromVLSIRModule(module_pb);
  const std::string &domain = circuit->domain();
  const std::string &name = circuit->name();

  // Get any existing cell being referenced:
  Cell *cell = FindCell(domain, name);
  if (!cell) {
    cell = new Cell();
    cell->set_domain(domain);
    cell->set_name(name);

    ConsumeCell(cell);
  }

  cell->SetCircuit(Circuit::FromVLSIRModule(module_pb));
  VLOG(3) << "Loaded module " << domain << "/" << name;
}

void DesignDatabase::LoadExternalModule(
    const vlsir::circuit::ExternalModule &module_pb) {
  // (We take ownership of the object.)
  Circuit *circuit = Circuit::FromVLSIRModule(module_pb);
  const std::string &domain = circuit->domain();
  const std::string &name = circuit->name();

  Cell *cell = FindCell(domain, name);
  if (!cell) {
    cell = new Cell();
    cell->set_domain(domain);
    cell->set_name(name);

    ConsumeCell(cell);
  }

  LOG_IF(WARNING, cell->circuit())
      << "Replacing circuit definition in cell domain: \""
      << domain << "\", name: \"" << name << "\"";

  cell->SetCircuit(circuit);

  VLOG(3) << "Loaded module \"" << domain << "/" << name << "\"";
}

bool DesignDatabase::ConsumeCell(Cell *cell) {
  const std::string &domain = cell->domain();
  const std::string &name = cell->name();
  auto outer_it = cells_.find(domain);
  if (outer_it == cells_.end()) {
    // NOTE(aryap): You need the std::move() to prevent triggering a copy of the
    // unique_ptr, which is a deleted function. Also be very very careful when
    // otherwise accidentally triggering copies of the inner structure, like for
    // instance if you write 'auto inner =' instead of 'auto &inner =' below.
    // This works:
    //  cells_[domain][name] = std::move(std::unique_ptr<Cell>(cell));
    // This is fewer lookups:
    auto insertion = cells_.insert(
        {domain, std::unordered_map<std::string, std::unique_ptr<Cell>>()});
    outer_it = insertion.first;
  }

  auto &inner = outer_it->second;
  auto inner_it = inner.find(name);

  if (inner_it != inner.end()) {
    VLOG(10)
        << "Could not consume cell, (domain, name) pair exists; domain: \""
        << domain << "\", name: \"" << name << "\"";
    return false;
  }

  inner.insert({name, std::move(std::unique_ptr<Cell>(cell))});
  return true;
}

Cell *DesignDatabase::FindCellOrDie(const std::string &name) const {
  return FindCellOrDie("", name);
}

Cell *DesignDatabase::FindCellOrDie(const std::string &domain,
                                    const std::string &name) const {
  auto domain_it = cells_.find(domain);
  if (domain_it == cells_.end()) {
    LOG(FATAL) << "Cell " << JoinDomainAndName(domain, name)
               << " not found; no entry for that domain";
    return nullptr;
  }

  const std::unordered_map<std::string, std::unique_ptr<Cell>> &inner =
      domain_it->second;

  auto it = inner.find(name);
  if (it == inner.end()) {
    LOG(FATAL) << "Cell " << JoinDomainAndName(domain, name)
               << " not found; no entry for that name in the domain";
    return nullptr;
  }
  Cell *cell = it->second.get();
  if (!cell) {
    LOG(FATAL) << "Cell " << JoinDomainAndName(domain, name)
               << " found but it is nullptr";
  }
  return cell;
}

Cell *DesignDatabase::FindCell(const std::string &domain,
                               const std::string &name) const {
  auto domain_it = cells_.find(domain);
  if (domain_it == cells_.end()) {
    return nullptr;
  }
  const std::unordered_map<std::string, std::unique_ptr<Cell>> &inner =
      domain_it->second;
  auto it = inner.find(name);
  if (it == inner.end()) {
    return nullptr;
  }
  Cell *cell = it->second.get();
  return cell;
}

// Place every cell in unordered_cells into *ordered_cells ensuring that every
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
    // Extract layout-only view of circuit.
    // TODO(aryap): This is broken. Need to be able to combine netlist and
    // layout hierarchies together.
    std::set<Cell*> direct_ancestors = cell->DirectAncestors(true);
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

std::string DesignDatabase::Describe() const {
  std::stringstream ss;
  for (const auto &domain_entry : cells_) {
    const std::string &domain = domain_entry.first;
    for (const auto &name_entry : domain_entry.second) {
      const std::string &name = name_entry.first;
      const Cell *const cell = name_entry.second.get();
      ss << cell << "\t" << domain << "\t" << name << std::endl;
    }
  }
  return ss.str();
}

}  // namespace bfg
