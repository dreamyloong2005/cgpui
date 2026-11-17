#pragma once

#include "cgpui/core/geometry.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

// Physical client size for a logical client size at `dpi` (96 = 100%).
[[nodiscard]] SIZE win32_physical_client_size(Size logical, UINT dpi);

// Outer window size whose client area is `client`, given the window's
// current window and client rects. Their difference is the real non-client
// frame, so a client title bar (WM_NCCALCSIZE without a caption) and the
// DWM resize borders are accounted for without re-deriving them.
[[nodiscard]] SIZE win32_outer_size_for_client(
    SIZE client, RECT window_rect, RECT client_rect);

// Resizes `hwnd` so its client area is `logical` pixels at its current DPI.
bool win32_set_logical_client_size(HWND hwnd, Size logical);

} // namespace cgpui
