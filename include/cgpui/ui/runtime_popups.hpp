#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <memory>

namespace cgpui {

// Popup windows (WindowKind::popup): frameless children of the root window
// that never take key focus and may extend outside it. A popup that takes
// the keyboard receives the root window's key and text events while it is
// open, because popups are never key windows.
// Requires both native platform support and an active popup-capable renderer.
// Without them, widgets retain their in-window overlay; direct opens fail.
[[nodiscard]] bool popup_windows_supported(const WindowRuntime& runtime);
[[nodiscard]] Result<AppOpenedWindow> open_popup(
    WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
    std::unique_ptr<View> view, WindowPopupOptions options = {});
PlatformPopupPlacementResult place_popup(
    WindowRuntime& runtime, WindowRuntimeId popup, WindowPopupPlacement placement);
bool request_popup_redraw(WindowRuntime& runtime, WindowRuntimeId popup);
bool close_popup(WindowRuntime& runtime, WindowRuntimeId popup);

} // namespace cgpui
