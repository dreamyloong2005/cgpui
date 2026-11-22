#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

// Posted by show_context_menu after a choice (win32_window_context_menu.cpp).
inline constexpr UINT win32_context_menu_command_message = WM_APP + 2U;

class Win32WindowMessageTarget;

bool win32_window_proc_handle_command(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
