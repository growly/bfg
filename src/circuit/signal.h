#ifndef CIRCUIT_SIGNAL_H_
#define CIRCUIT_SIGNAL_H_

#include <string>

namespace bfg {
namespace circuit {

class Signal {
 public:
  Signal() = default;

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  int64_t width() const { return width_; }
  void set_width(const int64_t width) { width_ = width; }

 private:
  std::string name_;
  int64_t width_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_SIGNAL_H_
