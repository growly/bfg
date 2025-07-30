#ifndef UTILITY_H_
#define UTILITY_H_

#include <algorithm>
#include <optional>

namespace bfg {

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

}  // namespace bfg

#endif  // UTILITY_H_
