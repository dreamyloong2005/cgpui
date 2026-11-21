#include "macos_dpi_awareness_internal.hpp"

namespace cgpui {

PlatformDpiAwarenessState macos_dpi_awareness_state() {
  return PlatformDpiAwarenessState{
      .awareness = PlatformDpiAwareness::per_monitor_aware_v2,
      .declared_by_framework = true,
      .already_set_by_process = false,
      .backend = "macos",
  };
}

} // namespace cgpui
