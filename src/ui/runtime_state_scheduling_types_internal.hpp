#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeTimer {
  TimerId id; WindowRuntimeId window_runtime_id{};
  std::uint64_t due_ms = 0;
  std::uint64_t interval_ms = 0;
  bool repeating = false;
  TimerCallback callback;
};

} // namespace cgpui
