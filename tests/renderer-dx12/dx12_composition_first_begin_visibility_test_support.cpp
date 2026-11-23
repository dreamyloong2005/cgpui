#include "dx12_composition_first_begin_test_support.hpp"

namespace cgpui_test {
bool Dx12CompositionFirstBeginWindows::matches(bool visible) const {
  if (!valid()) return false;
  const LONG_PTR style = GetWindowLongPtrW(popup_, GWL_STYLE);
  const LONG_PTR extended = GetWindowLongPtrW(popup_, GWL_EXSTYLE);
  constexpr LONG_PTR required = WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  RECT client{};
  return (style & WS_POPUP) && !(style & (WS_CHILD | WS_CAPTION | WS_THICKFRAME)) &&
      (extended & required) == required &&
      !(extended & (WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT)) &&
      GetWindow(popup_, GW_OWNER) == owner_ &&
      ((IsWindowVisible(popup_) != FALSE) == visible) &&
      ((IsWindowVisible(owner_) != FALSE) == visible) &&
      ((style & WS_VISIBLE) != 0) == visible && !IsIconic(popup_) &&
      GetActiveWindow() == active_before_ && GetForegroundWindow() != popup_ &&
      GetClientRect(popup_, &client) && client.right - client.left == 64 &&
      client.bottom - client.top == 64;
}

bool Dx12CompositionFirstBeginWindows::show_explicitly() {
  if (!matches(false)) return false;
  ShowWindow(owner_, SW_SHOWNOACTIVATE);
  ShowWindow(popup_, SW_SHOWNOACTIVATE);
  return matches(true);
}
} // namespace cgpui_test
