#include "win32_window_internal.hpp"

namespace cgpui {

void Win32Window::ole_drag_entered(
    IDataObject* data_object,
    POINTL point,
    DWORD key_state,
    DWORD* effect) {
  const DWORD selected_effect =
      choose_ole_drop_effect(
          effect != nullptr ? *effect : DROPEFFECT_NONE,
          key_state);
  if (effect != nullptr) {
    *effect = selected_effect;
  }
  last_ole_drag_position_ = client_position_from_screen(point);
  last_ole_drag_payload_ = drag_payload_from_ole_data_object(data_object);
  callback_(DragEntered{
      .position = last_ole_drag_position_,
      .payload = last_ole_drag_payload_,
      .action = drag_action_from_drop_effect(selected_effect)});
}

void Win32Window::ole_drag_updated(
    POINTL point,
    DWORD key_state,
    DWORD* effect) {
  const DWORD selected_effect =
      choose_ole_drop_effect(
          effect != nullptr ? *effect : DROPEFFECT_NONE,
          key_state);
  if (effect != nullptr) {
    *effect = selected_effect;
  }
  last_ole_drag_position_ = client_position_from_screen(point);
  callback_(DragUpdated{
      .position = last_ole_drag_position_,
      .payload = last_ole_drag_payload_,
      .action = drag_action_from_drop_effect(selected_effect)});
}

void Win32Window::ole_drag_exited() {
  callback_(DragExited{
      .position = last_ole_drag_position_,
      .payload = {},
      .action = DragDropAction::none});
  last_ole_drag_payload_ = {};
}

void Win32Window::ole_drag_dropped(
    IDataObject* data_object,
    POINTL point,
    DWORD key_state,
    DWORD* effect) {
  const DWORD selected_effect =
      choose_ole_drop_effect(
          effect != nullptr ? *effect : DROPEFFECT_NONE,
          key_state);
  if (effect != nullptr) {
    *effect = selected_effect;
  }
  last_ole_drag_position_ = client_position_from_screen(point);
  last_ole_drag_payload_ = drag_payload_from_ole_data_object(data_object);
  callback_(DragDropped{
      .position = last_ole_drag_position_,
      .payload = last_ole_drag_payload_,
      .action = drag_action_from_drop_effect(selected_effect)});
  last_ole_drag_payload_ = {};
}

void Win32Window::register_drop_target(HWND hwnd) {
  if (ole_drop_target_ == nullptr || hwnd == nullptr ||
      ole_drop_target_registration_.registered) {
    return;
  }
  ole_drop_target_registration_.hwnd = hwnd;
  ole_drop_target_registration_.last_registration_result =
      RegisterDragDrop(hwnd, ole_drop_target_.get());
  ole_drop_target_registration_.registered =
      SUCCEEDED(ole_drop_target_registration_.last_registration_result);
}

void Win32Window::revoke_drop_target() {
  if (!ole_drop_target_registration_.registered ||
      ole_drop_target_registration_.hwnd == nullptr) {
    return;
  }
  ole_drop_target_registration_.last_revocation_result =
      RevokeDragDrop(ole_drop_target_registration_.hwnd);
  ole_drop_target_registration_.registered = false;
  ole_drop_target_registration_.hwnd = nullptr;
}

Point Win32Window::client_position_from_screen(POINTL point) const {
  POINT screen_point{
      .x = static_cast<LONG>(point.x),
      .y = static_cast<LONG>(point.y),
  };
  if (hwnd_ != nullptr) {
    ScreenToClient(hwnd_, &screen_point);
  }
  return Point{
      .x = static_cast<float>(screen_point.x),
      .y = static_cast<float>(screen_point.y)};
}

} // namespace cgpui
