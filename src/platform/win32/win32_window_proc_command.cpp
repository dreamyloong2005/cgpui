#include "win32_window_message_internal.hpp"
#include "win32_window_proc_command_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_command(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  if (message != WM_COMMAND || lparam != 0 || window == nullptr) return false;
  const WORD source = HIWORD(wparam);
  if (source > 1U) return false;
  window->native_menu_command(
      LOWORD(wparam),
      source == 1U ? NativeMenuCommandSource::accelerator
                   : NativeMenuCommandSource::menu);
  result = 0;
  return true;
}

} // namespace cgpui
