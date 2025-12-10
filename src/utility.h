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

  // Does a kind of apriori stripmining, splitting 'length' into as many
  // sub-lengths of 'max' size as can fit, and any remaining length, where all
  // returned length are multiples of the given 'unit'. (The 'max' value is also
  // rounded down to the nearest multiple of 'unit'.)
  // 
  // e.g. StripInUnits(32, 9, 3) will result in [9, 9, 9, 3], with 2 leftover
  // of the original quantity not fitting a multiple of 3.
  // StripInUnits(33, 9, 3) will result in [9, 9, 9, 6].
  // StripInUnits(32, 8, 3) will result in [6, 6, 6, 6, 6].
  static std::vector<int64_t> StripInUnits(
      int64_t length, int64_t max, int64_t unit) {
    // Rely on truncating (floor) behaviour.
    int64_t real_max = (max / unit) * unit;

    std::vector<int64_t> lengths;

    // Stripmining!
    int64_t unallocated = length;
    while (unallocated >= unit) {
      int64_t remainder = unallocated - real_max;
      if (remainder >= 0) {
        lengths.push_back(real_max);
      } else {
        // Again we rely on truncating (floor) behaviour:
        lengths.push_back((unallocated / unit) * unit);
      }
      unallocated = remainder;
    }
    return lengths;
  }
};

}  // namespace bfg

#endif  // UTILITY_H_
