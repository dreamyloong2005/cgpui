#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimePlatformState {
  std::optional<PlatformAccessibilityTreeUpdate>
      last_platform_accessibility_update_;
  Theme app_theme_;
  std::unordered_map<std::uint64_t, Theme> window_themes_;
  #include "runtime_platform_service_state_internal.hpp"
  #include "runtime_system_appearance_internal.hpp"
};

} // namespace cgpui
