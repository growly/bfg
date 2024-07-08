#include "equivalent_nets.h"

#include <string>
#include <set>

namespace bfg {

bool EquivalentNets::Contains(const std::string &name) const {
  return nets_.find(name) != nets_.end();
}

bool EquivalentNets::Add(const std::string &name) {
  if (name == "") {
    return false;
  }
  bool added = nets_.insert(name).second;
  if (added && nets_.size() == 1) {
    primary_ = name;
  }
  return added;
}

bool EquivalentNets::Delete(const std::string &name) {
  return nets_.erase(name) > 0;
}

}  // namespace bfg
