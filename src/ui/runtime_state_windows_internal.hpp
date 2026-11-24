#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_surface_resizes_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeWindowState {
  RuntimeWindowState();
  ~RuntimeWindowState();
  PlatformWindow* window_ = nullptr;
  Renderer* renderer_ = nullptr;
  Size framebuffer_size_{};
  Size viewport_size_{};
  DpiScale scale_{};
  std::vector<AppOpenedWindow> app_opened_windows_;
  WindowRuntimeId root_window_runtime_id_{1};
  WindowRuntimeId keyboard_popup_{};
  std::uint64_t next_window_runtime_id_ = 2;
  std::unique_ptr<RuntimeWindowStore> window_runtime_records_;
  RuntimeSurfaceResizes surface_resizes_;
  #include "runtime_window_ownership_state_internal.hpp"
};

} // namespace cgpui
