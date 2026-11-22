#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

// Popup windows (win32_popup_window_internal.hpp): WM_MOUSEACTIVATE
// returns MA_NOACTIVATE, so a click never activates them. Any other window
// moves its popups along on WM_MOVE and lets the lifecycle handler report
// the move.
bool win32_window_proc_handle_popup(HWND hwnd, UINT message, LRESULT& result);

} // namespace cgpui
