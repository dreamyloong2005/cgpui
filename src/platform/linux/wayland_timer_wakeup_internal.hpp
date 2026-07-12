#pragma once

#include <cstdint>

namespace cgpui {

class WaylandTimerWakeup {
 public:
  WaylandTimerWakeup();
  ~WaylandTimerWakeup();

  WaylandTimerWakeup(const WaylandTimerWakeup&) = delete;
  WaylandTimerWakeup& operator=(const WaylandTimerWakeup&) = delete;

  [[nodiscard]] int descriptor() const { return descriptor_; }
  void request(std::uint64_t delay_ms);
  void cancel();
  void drain();

 private:
  int descriptor_ = -1;
};

} // namespace cgpui
