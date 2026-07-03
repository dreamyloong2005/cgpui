#include "win32_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_test_drag(
    Win32WindowMessageTarget* window,
    UINT message,
    LPARAM lparam,
    LRESULT& result) {
  const auto* drag_payload =
      reinterpret_cast<const Win32TestDragDropPayload*>(lparam);

  if (message == test_drag_enter_message()) {
    if (window != nullptr) {
      window->drag_entered(drag_payload);
    }
    result = 0;
    return true;
  }
  if (message == test_drag_update_message()) {
    if (window != nullptr) {
      window->drag_updated(drag_payload);
    }
    result = 0;
    return true;
  }
  if (message == test_drag_drop_message()) {
    if (window != nullptr) {
      window->drag_dropped(drag_payload);
    }
    result = 0;
    return true;
  }
  if (message == test_drag_exit_message()) {
    if (window != nullptr) {
      window->drag_exited(drag_payload);
    }
    result = 0;
    return true;
  }

  return false;
}

} // namespace cgpui
