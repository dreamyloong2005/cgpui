#pragma once

#include "cgpui/core/event_drag_drop.hpp"
#include "cgpui/core/event_keyboard.hpp"
#include "cgpui/core/event_native_menu.hpp"
#include "cgpui/core/event_pointer.hpp"
#include "cgpui/core/event_text.hpp"
#include "cgpui/core/event_window.hpp"

#include <variant>

namespace cgpui {

using PlatformEvent = std::variant<
    WindowCloseRequested,
    WindowRedrawRequested,
    WindowWakeupRequested,
    WindowResized,
    WindowMoved,
    WindowActivated,
    WindowFocused,
    WindowMinimized,
    WindowRestored,
    PointerMoved,
    PointerExited,
    PointerButton,
    PointerScrolled,
    PointerCaptureChanged,
    DragEntered,
    DragUpdated,
    DragDropped,
    DragExited,
    NativeMenuCommand,
    KeyboardKey,
    TextInput,
    ImeComposition,
    ImeDeleteSurroundingText>;

enum class EventKind {
  unknown,
  window_activated,
  window_focused,
  window_minimized,
  window_restored,
  window_resized,
  window_moved,
  window_close_requested,
  pointer_moved,
  pointer_exited,
  pointer_button,
  pointer_scrolled,
  pointer_capture_changed,
  drag_entered,
  drag_updated,
  drag_dropped,
  drag_exited,
  native_menu_command,
  keyboard_key,
  text_input,
  ime_composition,
  ime_delete_surrounding_text,
};

} // namespace cgpui
