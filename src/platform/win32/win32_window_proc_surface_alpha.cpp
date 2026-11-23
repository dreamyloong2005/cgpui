#include "win32_window_proc_surface_alpha_internal.hpp"

#include "win32_native_menu_alpha_internal.hpp"
#include "win32_window_surface_alpha_internal.hpp"

namespace cgpui {

UINT_PTR win32_window_surface_alpha_instance(HWND hwnd) {
  const auto* alpha = win32_window_surface_alpha(hwnd);
  return alpha ? alpha->instance() : 0;
}

bool win32_window_proc_handle_surface_alpha(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) {
  auto* alpha = win32_window_surface_alpha(hwnd);
  if (!alpha) return false;
  if (message == win32_surface_alpha_instance_message) {
    result = wparam == 0 && lparam == 0 ? static_cast<LRESULT>(alpha->instance()) : 0;
    return true;
  }
  if (message >= win32_surface_alpha_query_message && message <= win32_surface_alpha_mark_opaque_message) {
    // Every wire payload is an integer token. Send/Post and nested dispatch use
    // exactly the same validation and never dereference a caller-owned address.
    result = alpha->request(message, static_cast<UINT_PTR>(wparam), static_cast<UINT_PTR>(lparam));
    return true;
  }
  if (message == WM_ERASEBKGND && alpha->active()) {
    RECT client{};
    GetClientRect(hwnd, &client);
    FillRect(reinterpret_cast<HDC>(wparam), &client,
             static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    result = 1;
    return true;
  }
  return false;
}

void win32_window_proc_after_surface_alpha(HWND hwnd, UINT message, UINT_PTR instance) {
  if (!instance || !win32_native_menu_alpha_event(message)) return;
  // Dispatch (especially the native menu loop) may destroy/recreate the window.
  // Do not retain a platform-object pointer across application/native callbacks.
  auto* alpha = win32_window_surface_alpha(hwnd);
  if (alpha && alpha->instance() == instance) alpha->repair_menu();
}

} // namespace cgpui
