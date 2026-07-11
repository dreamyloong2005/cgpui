#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

class Win32WindowMessageTarget;

bool win32_window_proc_handle_command(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
