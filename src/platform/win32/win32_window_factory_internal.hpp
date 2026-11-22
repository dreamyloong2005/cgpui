#pragma once

#include "win32_window_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<Win32Window>> create_win32_window(
    HINSTANCE instance,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    HWND owner);
// The HWND behind a Win32 PlatformWindow; nullptr for any other window.
[[nodiscard]] HWND win32_window_hwnd(const PlatformWindow& window);

} // namespace cgpui
