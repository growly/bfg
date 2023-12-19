#ifndef ROUTING_TRACK_BLOCKAGE_H_
#define ROUTING_TRACK_BLOCKAGE_H_

#include <cstdint>
#include <glog/logging.h>

namespace bfg {

class RoutingTrackBlockage {
 public:
  RoutingTrackBlockage(int64_t start, int64_t end)
      : start_(start), end_(end) {
    LOG_IF(FATAL, end_ < start_)
        << "RoutingTrackBlockage start must be before end.";
  }

  ~RoutingTrackBlockage() {
    //LOG(INFO) << "Blockage (" << start_ << ", " << end_
    //          << ") is being destroyed";
  }

  bool Contains(int64_t position) const;
  bool IsAfter(int64_t position) const;
  bool IsBefore(int64_t position) const;

  bool Blocks(int64_t low, int64_t high) const;

  void set_start(int64_t start) { start_ = start; }
  void set_end(int64_t end) { end_ = end; }

  int64_t start() const { return start_; }
  int64_t end() const { return end_; }

 private:
  int64_t start_;
  int64_t end_;
};

}  // namespace bfg

#endif  // ROUTING_TRACK_BLOCKAGE_H_
