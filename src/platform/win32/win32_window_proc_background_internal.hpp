#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

// WM_ERASEBKGND while a backdrop extends the DWM frame into the client area:
// DWM composites the GDI redirection surface under the transparent
// swapchain, and an unpainted surface is opaque white. GDI black has alpha
// 0, so the client is filled black and the material shows through. Other
// windows fall through to DefWindowProc (null class brush: no painting).
bool win32_window_proc_handle_background(
    HWND hwnd, UINT message, WPARAM wparam, LRESULT& result);

} // namespace cgpui
