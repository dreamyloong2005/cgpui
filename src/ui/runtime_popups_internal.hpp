#pragma once

#include "cgpui/ui/runtime_popups.hpp"

namespace cgpui {

// Private runtime access for the popup functions (WindowRuntime friend).
struct RuntimePopups {
  static bool supported(const WindowRuntime& runtime);
  static Result<AppOpenedWindow> open(
      WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
      std::unique_ptr<View> view, bool takes_keyboard);
  static bool close(WindowRuntime& runtime, WindowRuntimeId popup);
  // Sends root key and text events to the popup that takes the keyboard.
  static bool forward_keyboard(WindowRuntime& runtime, const PlatformEvent& event);
};

} // namespace cgpui
