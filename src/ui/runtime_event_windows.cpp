#include "ui_internal.hpp"
#include "runtime_window_scale_diagnostics_internal.hpp"

namespace cgpui {

void WindowRuntime::dispatch_view_event_for_record(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr) {
    return;
  }

  update_input_state_for_record(record, event);

  begin_event_route_for_record(record, event);
  dispatching_view_event_ = true;
  EventResult result = view.handle_event(event, context_for_record(record));
  dispatching_view_event_ = false;
  finish_event_dispatch_for_record(record, result);
}

void WindowRuntime::handle_native_additional_window_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event) {
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr || !record->active) {
    return;
  }
  if (handle_native_menu_command_event(event, record->root_view_id)) {
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
    record->framebuffer_size = resized->size;
    record->viewport_size = to_logical_pixels(resized->size, resized->scale);
    record->scale = resized->scale;
    record->descriptor.size = to_logical_pixels(resized->size, resized->scale);
    const Result<void> resize_result =
        record->renderer->resize(resized->size, resized->scale);
    record_platform_diagnostic(
        window_scale_diagnostic(*resized, resize_result.has_value()));
    return;
  }
  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    record->redraw_scheduled = true;
    handle_redraw_for_record(*record, *view);
    return;
  }
  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (const auto* activated = std::get_if<WindowActivated>(&event);
        activated != nullptr) {
      record->input.focused = activated->active;
    } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
               minimized != nullptr) {
      record->input.focused = !minimized->minimized;
    } else {
      record->input.focused = false;
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
