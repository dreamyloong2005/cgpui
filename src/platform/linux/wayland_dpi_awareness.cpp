#include "wayland_dpi_awareness_internal.hpp"

namespace cgpui {

PlatformDpiAwarenessState wayland_dpi_awareness_state(
    bool fractional_scale_available) {
  return PlatformDpiAwarenessState{
      .awareness = fractional_scale_available
          ? PlatformDpiAwareness::per_monitor_aware_v2
          : PlatformDpiAwareness::per_monitor_aware,
      .declared_by_framework = true,
      .already_set_by_process = false,
      .backend = "wayland",
  };
}

} // namespace cgpui
