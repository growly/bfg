#include "circuit.h"

#include <sstream>

namespace bfg {

std::string Circuit::Describe() const {
  std::stringstream ss;

  ss << "circuit" << std::endl;

  return ss.str();
};

}  // namespace bfg
