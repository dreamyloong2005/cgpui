#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_scheduling_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeScheduleState {
  std::vector<DeferredCallback> deferred_callbacks_;
  std::vector<RuntimeTimer> timers_;
  std::uint64_t next_timer_id_ = 1;
  std::uint64_t current_time_ms_ = 0, last_platform_time_ms_ = 0;
  bool firing_timers_ = false, platform_clock_initialized_ = false;
  bool handling_wakeup_ = false;
  int update_batch_depth_ = 0;
  InvalidationState invalidation_state_;
  bool draining_deferred_callbacks_ = false;
  bool redraw_scheduled_ = false;
  bool deferred_redraw_request_ = false;
  #include "runtime_frame_scheduling_internal.hpp"
};

} // namespace cgpui
