#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

#include "runtime_popups_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_event(const PlatformEvent& event) {
  const RuntimeWindowCallbackScope scope(*this, !std::holds_alternative<WindowWakeupRequested>(event));
  if (RuntimePopups::forward_keyboard(*this, event)) {
    return;
  }
  if (handle_accessibility_action_event(event, state_->views.root_view_id_)) {
    return;
  }
  if (handle_native_menu_command_event(event, state_->views.root_view_id_)) {
    return;
  }
  if (handle_window_control_event(event)) {
    return;
  }

  if (state_->windows.window_ == nullptr || state_->windows.renderer_ == nullptr) {
    return;
  }

  update_input_state_for_event(event);
  state_->input.current_event_route_ = EventRouter::route_to_root(event, state_->views.root_view_id_);
  const std::optional<ElementId> hit_element_id =
      hit_test_target_for_event(event);
  update_hover_cursor_for_event(event, hit_element_id);
  resolve_event_route_target(event, hit_element_id);
  refresh_route_ancestry(*state_->input.current_event_route_);
  update_active_state_for_event(event);
  refresh_disabled_interaction_state();
  apply_focus_activation_for_event(event);
  apply_keyboard_bindings_for_event(event);
  apply_text_input_for_event(event);
  finish_event_dispatch(dispatch_current_event_route(event));
}

} // namespace cgpui
