#ifndef EQUIVALENT_NETS_H_
#define EQUIVALENT_NETS_H_

#include <string>
#include <set>

namespace bfg {

// This is just a fancy wrapper for a set of net names (strings) which are all
// considered to be the same net. This is a convenience for the router to
// navigate the multiple nested names attached to ports and shapes in a layout
// when trying to find which are and which are not connectable.
//
// It is convenient to nominate one such alias as the primary name given to new
// objects belonging to the net. This is called the primary.
//
// The class guarantees that the primary (if not "") always exists in the set of
// aliases.
class EquivalentNets {
 public:
  EquivalentNets() {}
  EquivalentNets(const std::string &sole) {
    set_primary(sole);
  }
  EquivalentNets(const std::set<std::string> &names) {
    for (const std::string &name : names) {
      Add(name);
    }
  }
  EquivalentNets(const std::string &primary,
                 const std::set<std::string> &names)
      : nets_(names.begin(), names.end()) {
    primary_ = primary;
    // Might be a no-op.
    Add(primary);
  }

  bool Contains(const std::string &name) const;
  bool Add(const std::string &name);
  bool Delete(const std::string &name);
  bool Empty() const {
    return nets_.empty();
  }

  const std::string &primary() const { return primary_; }
  void set_primary(const std::string &primary) {
    primary_ = primary;
    Add(primary);
  }

 private:
  std::set<std::string> nets_;
  std::string primary_;
};

}  // namespace bfg

#endif  // EQUIVALENT_NETS_H_
