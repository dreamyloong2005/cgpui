#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

[[nodiscard]] bool win32_pointer_captured(HWND hwnd);
void capture_win32_pointer(HWND hwnd);
[[nodiscard]] bool release_win32_pointer(HWND hwnd);

} // namespace cgpui
