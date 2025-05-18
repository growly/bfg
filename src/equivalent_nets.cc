#include "equivalent_nets.h"

#include <string>
#include <set>

namespace bfg {

bool EquivalentNets::ContainsAny(const EquivalentNets &other) const {
  // std::vector<std::string> intersection;
  // std::set_intersection(nets_.begin(), nets_.end(),
  //                       other.nets_.begin(), other.nets_.end(),
  //                       std::back_inserter(intersections));
  // return !intersection.empty();
  for (const std::string &net : nets_) {
    if (other.Contains(net))
      return true;
  }
  return false;
}

bool EquivalentNets::Contains(const std::string &name) const {
  return nets_.find(name) != nets_.end();
}

bool EquivalentNets::Add(const EquivalentNets &other) {
  bool any = false;
  for (const std::string &net : other.nets_) {
    bool success = Add(net);
    any |= success;
  }
  return any;
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
