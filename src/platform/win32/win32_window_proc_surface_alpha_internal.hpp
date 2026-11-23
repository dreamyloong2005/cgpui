#pragma once

#include "../win32_surface_alpha_protocol_internal.hpp"

namespace cgpui {

[[nodiscard]] UINT_PTR win32_window_surface_alpha_instance(HWND hwnd);
bool win32_window_proc_handle_surface_alpha(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);
void win32_window_proc_after_surface_alpha(HWND hwnd, UINT message, UINT_PTR instance);

} // namespace cgpui
