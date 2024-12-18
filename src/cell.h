#ifndef CELL_H_
#define CELL_H_

#include <string>
#include <vector>

#include "circuit.h"
#include "layout.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

class Cell {
 public:
  static void TieInstances(
      circuit::Instance *circuit, geometry::Instance *geometry);

  Cell() = default;
  Cell(const std::string &name)
    : is_external_(false),
      name_(name) {}

  void set_domain(const std::string &domain) { domain_ = domain; }
  const std::string &domain() const { return domain_; }

  void set_name(const std::string &name) { name_ = name; }
  const std::string &name() const { return name_; }

  void set_description(const std::string &description) { description_ = description; }
  const std::string &description() const { return description_; }

  std::set<Cell*> DirectAncestors(bool layout_only = false) const;

  void SetLayout(Layout *layout) {
    layout_.reset(layout);
    layout->set_parent_cell(this);
  }
  Layout *layout() { return layout_.get(); }
  Layout *const layout() const { return layout_.get(); }

  void SetCircuit(Circuit *circuit) {
    circuit_.reset(circuit);
    circuit->set_parent_cell(this);
  }
  Circuit *circuit() { return circuit_.get(); }
  Circuit *const circuit() const { return circuit_.get(); }

  void set_is_external(bool is_external) { is_external_ = is_external; };
  bool is_external() const { return is_external_; }

  ::vlsir::raw::Cell ToVLSIRCell() const;

 private:
  bool is_external_;

  std::string domain_;
  std::string name_;
  std::string description_;

  std::unique_ptr<Layout> layout_;
  std::unique_ptr<Circuit> circuit_;
};

}  // namespace bfg

#endif  // CELL_H_
