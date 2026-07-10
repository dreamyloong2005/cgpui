#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::dispatch_view_event_for_record(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr) {
    return;
  }

  if (const auto* focused = std::get_if<WindowFocused>(&event);
      focused != nullptr) {
    input_.focused = focused->focused;
  } else if (const auto* moved = std::get_if<PointerMoved>(&event);
             moved != nullptr) {
    input_.pointer_position = moved->position;
  } else if (const auto* button = std::get_if<PointerButton>(&event);
             button != nullptr) {
    input_.pointer_position = button->position;
  } else if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
             scrolled != nullptr) {
    input_.pointer_position = scrolled->position;
  }

  current_event_route_ = EventRouter::route_to_root(event, record.root_view_id);
  refresh_route_ancestry(*current_event_route_);
  dispatching_view_event_ = true;
  EventResult result = view.handle_event(event, context_for_record(record));
  dispatching_view_event_ = false;
  last_event_result_ = result;
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context_for_record(record), *last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::handle_native_additional_window_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event) {
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr) {
    return;
  }
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    const bool close_policy_pending = record->window != nullptr &&
        record->window->close_request_state().pending;
    record_lifecycle_event_for_record(*record, event);
    if (close_policy_pending && record->window != nullptr) {
      if (record->window->close_request_state().pending) {
        (void)record->window->resolve_close_request(
            PlatformWindowCloseResolution::accept);
      }
      if (!record->window->close_request_state().accepted) {
        return;
      }
    }
    cleanup_closed_additional_window(*record);
    return;
  }
  View* view = find_view(record->root_view_id);
  if (view == nullptr) {
    return;
  }
  if (const auto* resized = std::get_if<WindowResized>(&event);
      resized != nullptr) {
    record->descriptor.size = to_logical_pixels(resized->size, resized->scale);
    if (record->renderer != nullptr) {
      (void)record->renderer->resize(resized->size, resized->scale);
    }
    return;
  }
  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    handle_redraw_for_record(*record, *view);
    return;
  }
  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (const auto* activated = std::get_if<WindowActivated>(&event);
        activated != nullptr) {
      input_.focused = activated->active;
    } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
               minimized != nullptr) {
      input_.focused = !minimized->minimized;
    } else {
      input_.focused = false;
    }
    record_lifecycle_event_for_record(*record, event);
    return;
  }

  if (std::holds_alternative<WindowMoved>(event)) {
    record_lifecycle_event_for_record(*record, event);
    return;
  }
  dispatch_view_event_for_record(*record, *view, event);
}

} // namespace cgpui
