#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeAnimation {
  AnimationId id;
  AnimationOptions options;
  AnimationCallback callback;
  std::uint64_t started_ms = 0;
  std::uint64_t last_tick_ms = 0;
  std::uint64_t next_tick_ms = 0;
  std::uint64_t cancelled_elapsed_ms = 0;
  bool complete = false;
  bool cancelled = false;
  bool frame_pending = false;
  bool due_for_frame = false;
};

} // namespace cgpui
