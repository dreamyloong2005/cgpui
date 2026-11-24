#pragma once

#include "../../core/native_window_work_internal.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui {
inline constexpr UINT win32_native_window_work_message = WM_APP + 9;
std::uintptr_t win32_native_window_work_open(HWND) noexcept;
void win32_native_window_work_close(HWND, std::uintptr_t& instance) noexcept;
bool win32_window_proc_handle_native_work(
    HWND, UINT, WPARAM, LPARAM, LRESULT&) noexcept;
} // namespace cgpui
