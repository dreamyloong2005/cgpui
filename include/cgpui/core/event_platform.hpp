#pragma once

#include "cgpui/core/event_drag_drop.hpp"
#include "cgpui/core/event_keyboard.hpp"
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
    WindowActivated,
    WindowFocused,
    WindowMinimized,
    WindowRestored,
    PointerMoved,
    PointerButton,
    PointerScrolled,
    DragEntered,
    DragUpdated,
    DragDropped,
    DragExited,
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
  window_close_requested,
  pointer_moved,
  pointer_button,
  pointer_scrolled,
  drag_entered,
  drag_updated,
  drag_dropped,
  drag_exited,
  keyboard_key,
  text_input,
  ime_composition,
  ime_delete_surrounding_text,
};

} // namespace cgpui
