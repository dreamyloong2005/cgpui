#pragma once

#include "cgpui/ui/runtime_popups.hpp"

namespace cgpui {

// Private runtime access for the popup functions (WindowRuntime friend).
struct RuntimePopups {
  static bool supported(const WindowRuntime& runtime);
  static Result<AppOpenedWindow> open(
      WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
      std::unique_ptr<View> view, WindowPopupOptions options);
  static bool close(WindowRuntime& runtime, WindowRuntimeId popup);
  // Sends root key and text events to the popup that takes the keyboard.
  static bool forward_keyboard(WindowRuntime& runtime, const PlatformEvent& event);
  // Handles `event` as an event of the root window, after the current
  // dispatch (a popup's pointer input for a widget of the root view).
  static void forward_to_root(WindowRuntime& runtime, PlatformEvent event);
};

} // namespace cgpui
