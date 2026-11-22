#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_input_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeInputState {
  ViewInputState input_{};
  std::optional<PointerCaptureOwner> pointer_capture_owner_;
  std::optional<ViewId> keyboard_focus_owner_;
  std::optional<ElementId> keyboard_focus_element_owner_;
  std::optional<TextPointerSelectionDrag> text_pointer_selection_drag_;
  std::optional<ElementId> hovered_element_id_;
  std::optional<ElementId> active_element_id_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  CursorShape applied_cursor_shape_ = CursorShape::default_arrow;
  EventResult last_event_result_{};
  std::optional<EventRoute> current_event_route_;
  bool dispatching_view_event_ = false;
};

} // namespace cgpui
