#pragma once

#include "cgpui/core/event_pointer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

[[nodiscard]] const wchar_t* win32_system_cursor_id(CursorShape cursor_shape);
[[nodiscard]] HCURSOR load_win32_system_cursor(CursorShape cursor_shape);

} // namespace cgpui
