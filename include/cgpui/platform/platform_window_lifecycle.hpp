#pragma once

namespace cgpui {

enum class PlatformWindowDisplayState {
  normal,
  minimized,
  maximized,
  fullscreen,
};

struct PlatformWindowLifecycleState {
  bool native_window_created = false;
  bool initial_configure_complete = false;
  bool active = false;
  bool focused = false;
  bool close_requested = false;
  PlatformWindowDisplayState display_state =
      PlatformWindowDisplayState::normal;
};

} // namespace cgpui
