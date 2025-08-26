#ifndef UTILITY_H_
#define UTILITY_H_

#include <algorithm>
#include <fstream>
#include <glog/logging.h>
#include <google/protobuf/text_format.h>
#include <optional>
#include <string>

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

  // FIXME(aryap): These only work as written for integer types because they
  // rely on truncating divides. For floating point types an explicit std::floor
  // should be thrown in. Or maybe these should not be for all types!
  // 
  // NOTE(aryap): Use integer types only.

  // Returns the next multiple of `multiple` that is greater than OR EQUAL TO
  // `min`.
  template<typename T>
  static T NextMultiple(const T &min, const T &multiple) {
    if (multiple == 0) {
      return min;
    }
    if (min % multiple == 0) {
      return min;
    }
    // We want 'floor' behaviour.
    return ((min / multiple) + 1) * multiple;
  }

  // Returns the next multiple of `multiple` that is strictly greater than
  // `min`.
  template<typename T>
  static T NextGreaterMultiple(const T &min, const T &multiple) {
    if (multiple == 0) {
      return min;
    }
    return ((min / multiple) + 1) * multiple;
  }

  // Returns the greatest multiple of `multiple` that is less than OR EQUAL TO
  // `max`.
  template<typename T>
  static T LastMultiple(const T &max, const T &multiple) {
    if (multiple == 0) {
      return max;
    }
    if (max % multiple == 0) {
      return max;
    }
    // We want 'floor' behaviour.
    return (max / multiple) * multiple;
  }

  static bool ReadTextProtoOrDie(
      const std::string &path,
      google::protobuf::Message *message_pb) {
    std::ifstream input(path);
    LOG_IF(FATAL, !input.is_open())
        << "Could not open text proto: " << path;
    std::ostringstream ss;
    ss << input.rdbuf();
    return google::protobuf::TextFormat::ParseFromString(ss.str(), message_pb);
  }
};

}  // namespace bfg

#endif  // UTILITY_H_
