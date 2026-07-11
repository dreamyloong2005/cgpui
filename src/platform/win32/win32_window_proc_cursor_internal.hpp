#pragma once

#include "win32_window_message_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_cursor(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
