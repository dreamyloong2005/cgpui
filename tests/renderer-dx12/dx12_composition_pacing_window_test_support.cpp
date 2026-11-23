#include "dx12_composition_pacing_test_support.hpp"
#include "dx12_composition_first_begin_test_support.hpp"

namespace cgpui_test {
bool composition_pacing_hidden_extent(const Dx12CompositionFirstBeginWindows& windows, unsigned extent) {
  const auto descriptor = windows.descriptor();
  const auto* native = std::get_if<cgpui::Win32SurfaceHandle>(&descriptor.native_surface);
  if (!windows.valid() || !native || !native->composition) return false;
  const auto popup = static_cast<HWND>(native->hwnd);
  const auto owner = GetWindow(popup, GW_OWNER);
  const auto style = GetWindowLongPtrW(popup, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(popup, GWL_EXSTYLE);
  constexpr LONG_PTR required = WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  RECT client{};
  return owner && IsWindow(owner) && !IsWindowVisible(owner) && !IsWindowVisible(popup) &&
      !(style & WS_VISIBLE) && (style & WS_POPUP) && !(style & WS_CHILD) &&
      (extended & required) == required && !IsIconic(popup) &&
      GetClientRect(popup, &client) && client.right - client.left == static_cast<LONG>(extent) &&
      client.bottom - client.top == static_cast<LONG>(extent);
}
bool composition_pacing_resize_hidden_window(const Dx12CompositionFirstBeginWindows& windows,
                                             unsigned extent) {
  const auto descriptor = windows.descriptor();
  const auto* native = std::get_if<cgpui::Win32SurfaceHandle>(&descriptor.native_surface);
  if (!native || !native->hwnd) return false;
  return SetWindowPos(static_cast<HWND>(native->hwnd), nullptr, 0, 0,
      static_cast<int>(extent), static_cast<int>(extent), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE) &&
      composition_pacing_hidden_extent(windows, extent);
}
} // namespace cgpui_test
