#include "win32_internal.hpp"

namespace cgpui {

UINT test_drag_enter_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragEnter");
  return message;
}

UINT test_drag_update_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragUpdate");
  return message;
}

UINT test_drag_drop_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragDrop");
  return message;
}

UINT test_drag_exit_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragExit");
  return message;
}

DragDropAction drag_action_from_drop_effect(DWORD drop_effect) {
  if ((drop_effect & DROPEFFECT_MOVE) != 0) {
    return DragDropAction::move;
  }
  if ((drop_effect & DROPEFFECT_COPY) != 0) {
    return DragDropAction::copy;
  }
  return DragDropAction::none;
}

DragDropAction drag_action_from_test_hook(
    const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return DragDropAction::none;
  }
  return drag_action_from_drop_effect(payload->drop_effect);
}

DWORD choose_ole_drop_effect(DWORD allowed_effect, DWORD key_state) {
  if ((key_state & MK_CONTROL) != 0 &&
      (allowed_effect & DROPEFFECT_COPY) != 0) {
    return DROPEFFECT_COPY;
  }
  if ((key_state & MK_SHIFT) != 0 &&
      (allowed_effect & DROPEFFECT_MOVE) != 0) {
    return DROPEFFECT_MOVE;
  }
  if ((allowed_effect & DROPEFFECT_COPY) != 0) {
    return DROPEFFECT_COPY;
  }
  if ((allowed_effect & DROPEFFECT_MOVE) != 0) {
    return DROPEFFECT_MOVE;
  }
  return DROPEFFECT_NONE;
}

} // namespace cgpui
