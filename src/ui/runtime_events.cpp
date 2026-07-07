#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_event(const PlatformEvent& event) {
  if (handle_window_control_event(event)) {
    return;
  }

  if (window_ == nullptr || renderer_ == nullptr) {
    return;
  }

  update_input_state_for_event(event);
  current_event_route_ = EventRouter::route_to_root(event, root_view_id_);
  const std::optional<ElementId> hit_element_id =
      hit_test_target_for_event(event);
  update_hover_cursor_for_event(event, hit_element_id);
  resolve_event_route_target(event, hit_element_id);
  refresh_route_ancestry(*current_event_route_);
  update_active_state_for_event(event);
  refresh_disabled_interaction_state();
  apply_focus_activation_for_event(event);
  apply_keyboard_bindings_for_event(event);
  apply_text_input_for_event(event);
  finish_event_dispatch(dispatch_current_event_route(event));
}

} // namespace cgpui
