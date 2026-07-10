#pragma once

#include "win32_window_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<Win32Window>> create_win32_window(
    HINSTANCE instance,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    HWND owner);

} // namespace cgpui
