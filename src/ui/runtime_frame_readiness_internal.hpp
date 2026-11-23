#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <cstdint>
#include <vector>

namespace cgpui {

// No native handles, renderer pointers, workers or public scheduling API.
class WindowRuntime::RuntimeFrameReadiness {
 public:
  bool handle(WindowRuntime& runtime, WindowRuntimeId window, const Error& error);
  void clear(WindowRuntime& runtime, WindowRuntimeId window);
  void clear_all(WindowRuntime& runtime);

 private:
  struct Pending {
    WindowRuntimeId window;
    TimerId timer;
    std::uint64_t token = 0;
    std::uint64_t since_ms = 0;
    bool delivered = false;
  };
  Result<void> defer(WindowRuntime& runtime, WindowRuntimeId window);
  void deliver(const WindowRuntimeContext& context, std::uint64_t token);
  std::vector<Pending> pending_;
  // Never reset between run() calls: old copied closures cannot match a new run.
  std::uint64_t next_token_ = 0;
};

} // namespace cgpui
