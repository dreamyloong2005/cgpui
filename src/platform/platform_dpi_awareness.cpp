#include "cgpui/platform/platform_application.hpp"

namespace cgpui {

// Platforms without a process-wide DPI awareness model report unsupported.
PlatformDpiAwarenessState PlatformApplication::dpi_awareness() const {
  return PlatformDpiAwarenessState{
      .awareness = PlatformDpiAwareness::unsupported,
      .backend = "unsupported",
  };
}

} // namespace cgpui
