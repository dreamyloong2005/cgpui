#pragma once

#include <string>

namespace cgpui {

enum class PlatformDpiAwareness {
  unsupported,
  unaware,
  system_aware,
  per_monitor_aware,
  per_monitor_aware_v2,
};

struct PlatformDpiAwarenessState {
  PlatformDpiAwareness awareness = PlatformDpiAwareness::unsupported;
  bool declared_by_framework = false;
  bool already_set_by_process = false;
  std::string backend;
};

} // namespace cgpui
