#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <memory>

namespace cgpui {

// Popup windows (WindowKind::popup): frameless children of the root window
// that never take key focus and may extend outside it. A popup that takes
// the keyboard receives the root window's key and text events while it is
// open, because popups are never key windows.
[[nodiscard]] bool popup_windows_supported(const WindowRuntime& runtime);
[[nodiscard]] Result<AppOpenedWindow> open_popup(
    WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
    std::unique_ptr<View> view, bool takes_keyboard = false);
PlatformPopupPlacementResult place_popup(
    WindowRuntime& runtime, WindowRuntimeId popup, WindowPopupPlacement placement);
bool request_popup_redraw(WindowRuntime& runtime, WindowRuntimeId popup);
bool close_popup(WindowRuntime& runtime, WindowRuntimeId popup);

} // namespace cgpui
