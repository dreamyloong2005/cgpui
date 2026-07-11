#pragma once

#include "cgpui/core/geometry.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

[[nodiscard]] Point win32_logical_client_point(
    LPARAM lparam,
    DpiScale scale);
[[nodiscard]] Point win32_logical_client_point_from_screen(
    HWND hwnd,
    LPARAM lparam,
    DpiScale scale);
[[nodiscard]] Point win32_logical_client_point_from_screen(
    HWND hwnd,
    POINTL point,
    DpiScale scale);

} // namespace cgpui
