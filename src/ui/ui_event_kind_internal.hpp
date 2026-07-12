#pragma once

#include "cgpui/ui/runtime.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace cgpui {
namespace {
inline EventKind event_kind_for(const PlatformEvent& event) {
  if (std::holds_alternative<AccessibilityActionRequested>(event)) return EventKind::accessibility_action;
  if (std::holds_alternative<WindowActivated>(event)) {
    return EventKind::window_activated;
  }
  if (std::holds_alternative<WindowFocused>(event)) {
    return EventKind::window_focused;
  }
  if (std::holds_alternative<WindowMinimized>(event)) {
    return EventKind::window_minimized;
  }
  if (std::holds_alternative<WindowRestored>(event)) {
    return EventKind::window_restored;
  }
  if (std::holds_alternative<WindowResized>(event)) return EventKind::window_resized;
  if (std::holds_alternative<WindowMoved>(event)) {
    return EventKind::window_moved;
  }
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    return EventKind::window_close_requested;
  }
  if (std::holds_alternative<PointerMoved>(event)) {
    return EventKind::pointer_moved;
  }
  if (std::holds_alternative<PointerExited>(event)) return EventKind::pointer_exited;
  if (std::holds_alternative<PointerButton>(event)) {
    return EventKind::pointer_button;
  }
  if (std::holds_alternative<PointerScrolled>(event)) {
    return EventKind::pointer_scrolled;
  }
  if (std::holds_alternative<PointerCaptureChanged>(event)) {
    return EventKind::pointer_capture_changed;
  }
  if (std::holds_alternative<DragEntered>(event)) {
    return EventKind::drag_entered;
  }
  if (std::holds_alternative<DragUpdated>(event)) {
    return EventKind::drag_updated;
  }
  if (std::holds_alternative<DragDropped>(event)) {
    return EventKind::drag_dropped;
  }
  if (std::holds_alternative<DragExited>(event)) return EventKind::drag_exited;
  if (std::holds_alternative<NativeMenuCommand>(event)) return EventKind::native_menu_command;
  if (std::holds_alternative<KeyboardKey>(event)) {
    return EventKind::keyboard_key;
  }
  if (std::holds_alternative<TextInput>(event)) {
    return EventKind::text_input;
  }
  if (std::holds_alternative<ImeComposition>(event)) {
    return EventKind::ime_composition;
  }
  if (std::holds_alternative<ImeDeleteSurroundingText>(event)) {
    return EventKind::ime_delete_surrounding_text;
  }
  return EventKind::unknown;
}
inline std::size_t drag_drop_payload_value_count(
    const DragDropPayload& payload) {
  switch (payload.kind) {
    case DragDropPayloadKind::text:
      return payload.text.empty() ? 0 : 1;
    case DragDropPayloadKind::files:
      return payload.files.size();
    case DragDropPayloadKind::none:
      return 0;
  }
  return 0;
}

inline std::size_t drag_drop_payload_value_count(const PlatformEvent& event) {
  if (const auto* drag_entered = std::get_if<DragEntered>(&event);
      drag_entered != nullptr) {
    return drag_drop_payload_value_count(drag_entered->payload);
  }
  if (const auto* drag_updated = std::get_if<DragUpdated>(&event);
      drag_updated != nullptr) {
    return drag_drop_payload_value_count(drag_updated->payload);
  }
  if (const auto* drag_dropped = std::get_if<DragDropped>(&event);
      drag_dropped != nullptr) {
    return drag_drop_payload_value_count(drag_dropped->payload);
  }
  if (const auto* drag_exited = std::get_if<DragExited>(&event);
      drag_exited != nullptr) {
    return drag_drop_payload_value_count(drag_exited->payload);
  }
  return 0;
}

inline std::string drag_drop_operation_for(const PlatformEvent& event) {
  if (std::holds_alternative<DragEntered>(event)) {
    return "drag-entered";
  }
  if (std::holds_alternative<DragUpdated>(event)) {
    return "drag-updated";
  }
  if (std::holds_alternative<DragDropped>(event)) {
    return "drag-dropped";
  }
  if (std::holds_alternative<DragExited>(event)) {
    return "drag-exited";
  }
  return "drag-drop";
}

} // namespace
} // namespace cgpui
