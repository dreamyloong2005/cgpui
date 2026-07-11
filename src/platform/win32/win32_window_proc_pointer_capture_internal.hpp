#pragma once

#include "win32_window_message_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_pointer_capture(
    HWND hwnd,
    UINT message,
    Win32WindowMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
