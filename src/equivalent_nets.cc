#include "equivalent_nets.h"

#include <string>
#include <set>

namespace bfg {

bool EquivalentNets::Contains(const std::string &name) {
  return nets_.find(name) != nets_.end();
}

bool EquivalentNets::Add(const std::string &name) {
  return nets_.insert(name).second;
}

bool EquivalentNets::Delete(const std::string &name) {
  return nets_.erase(name) > 0;
}

}  // namespace bfg
