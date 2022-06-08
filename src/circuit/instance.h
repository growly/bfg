#ifndef CIRCUIT_INSTANCE_H_
#define CIRCUIT_INSTANCE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "parameter.h"
#include "qualified_name.h"

namespace bfg {
namespace circuit {

class Instance {
 public:
  Instance() = default;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

 private:
  std::string name_;

  // The "master", or "template", circuit object.
  Module *module_;

  std::unordered_map<std::string, Parameter> parameters_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_INSTANCE_H_
