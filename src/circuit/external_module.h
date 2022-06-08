#ifndef CIRCUIT_EXTERNAL_MODULE_H_
#define CIRCUIT_EXTERNAL_MODULE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "parameter.h"
#include "qualified_name.h"

namespace bfg {
namespace circuit {

class ExternalModule {
 public:
  Module() = default;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

 private:
  QualifiedName name_;
  std::string description_;

  std::vector<std::unique_ptr<Port>> ports_;

  std::unordered_map<std::string, Parameter> parameters_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_EXTERNAL_MODULE_H_
