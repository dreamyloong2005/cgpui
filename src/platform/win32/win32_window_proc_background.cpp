#include "win32_window_proc_background_internal.hpp"
#include "win32_window_backdrop_internal.hpp"

namespace cgpui {
namespace {

bool backdrop_extends_frame(HWND hwnd) {
  const auto* backdrop = static_cast<const Win32WindowBackdrop*>(
      GetPropW(hwnd, win32_window_backdrop_property));
  return backdrop != nullptr && backdrop->frame_extended();
}

} // namespace

bool win32_window_proc_handle_background(
    HWND hwnd, UINT message, WPARAM wparam, LRESULT& result) {
  if (message != WM_ERASEBKGND || !backdrop_extends_frame(hwnd)) {
    return false;
  }
  RECT client{};
  GetClientRect(hwnd, &client);
  FillRect(reinterpret_cast<HDC>(wparam), &client,
           static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
  result = 1;
  return true;
}

} // namespace cgpui
