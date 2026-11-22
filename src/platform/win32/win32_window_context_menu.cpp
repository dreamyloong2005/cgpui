#include "win32_window_internal.hpp"
#include "win32_window_proc_command_internal.hpp"

#include <cmath>
#include <string>
#include <utility>

namespace cgpui {

// TrackPopupMenuEx runs a modal menu loop: it returns when the menu closes,
// and the app's own message loop waits until then. TPM_RETURNCMD returns
// the chosen id instead of posting WM_COMMAND, so the menu bar command map
// never sees it.
PlatformContextMenuResult Win32Window::show_context_menu(
    const NativeMenuModel& menu, Point position) {
  PlatformContextMenuResult result{.supported = true, .backend = "win32"};
  if (hwnd_ == nullptr || IsWindow(hwnd_) == FALSE) {
    result.reason = "window has no HWND";
    return result;
  }
  if (menu.items.empty()) {
    result.reason = "the menu has no items";
    return result;
  }
  const auto tree = Win32NativeMenuTree::build_popup(menu);
  if (!tree.has_value() || tree->root() == nullptr) {
    result.reason = "CreatePopupMenu or InsertMenuItemW failed";
    return result;
  }
  const auto commands = Win32NativeMenuCommandMap::build(menu);
  // Logical client pixels to physical screen pixels.
  const float scale = state_.scale.value > 0.0F ? state_.scale.value : 1.0F;
  POINT screen{static_cast<LONG>(std::lround(position.x * scale)),
               static_cast<LONG>(std::lround(position.y * scale))};
  const HWND hwnd = hwnd_;
  ClientToScreen(hwnd, &screen);
  // Documented TrackPopupMenu requirement: the owner is foreground while
  // the menu tracks (outside clicks close it), and WM_NULL follows it.
  SetForegroundWindow(hwnd);
  SetLastError(ERROR_SUCCESS);
  const UINT command = static_cast<UINT>(TrackPopupMenuEx(
      tree->root(), TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
      screen.x, screen.y, hwnd, nullptr));
  // 0 is a cancel or a failure. Handlers run inside the modal loop may set
  // the last error, so only the "another menu is open" failure is told apart.
  if (command == 0 && GetLastError() == ERROR_POPUP_ALREADY_ACTIVE) {
    result.reason = "another menu is already open";
    return result;
  }
  PostMessageW(hwnd, WM_NULL, 0, 0);
  result.shown = true;
  auto chosen = commands->resolve(command, NativeMenuCommandSource::context_menu);
  if (!chosen.has_value()) return result;
  result.action_name = chosen->action_name;
  // The modal loop dispatches the app's messages, so the app may have
  // destroyed this window meanwhile; then `this` must not be touched.
  if (IsWindow(hwnd) == FALSE) return result;
  // The caller is usually still handling the click that opened the menu;
  // the command event follows on the next message loop turn. A window
  // destroyed before then never gets the message: ~Win32Window clears
  // GWLP_USERDATA and destroys the HWND.
  pending_context_menu_commands_.push_back(std::move(*chosen));
  PostMessageW(hwnd, win32_context_menu_command_message, 0, 0);
  return result;
}

void Win32Window::context_menu_command() {
  if (pending_context_menu_commands_.empty()) return;
  NativeMenuCommand command = std::move(pending_context_menu_commands_.front());
  pending_context_menu_commands_.erase(pending_context_menu_commands_.begin());
  callback_(std::move(command));
}

} // namespace cgpui
