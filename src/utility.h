#ifndef UTILITY_H_
#define UTILITY_H_

#include <algorithm>
#include <optional>

namespace bfg {

class Utility {
 public:
  template<typename T>
  static void UpdateMax(const T &new_value, std::optional<T> *destination) {
    if (destination->has_value()) {
      (*destination) = std::max(new_value, destination->value());
    } else {
      (*destination) = new_value;
    }
  }

  template<typename T>
  static void UpdateMin(const T &new_value, std::optional<T> *destination) {
    if (destination->has_value()) {
      (*destination) = std::min(new_value, destination->value());
    } else {
      (*destination) = new_value;
    }
  }

  template<typename T>
  static T NextMultiple(const T &min, const T &multiple) {
    return ((min / multiple) + 1) * multiple;
  }
};

}  // namespace bfg

#endif  // UTILITY_H_
