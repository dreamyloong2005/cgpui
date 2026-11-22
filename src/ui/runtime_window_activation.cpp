#include "ui_internal.hpp"
#include "runtime_popup_support_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

void WindowRuntime::activate_native_window_for_record(
    WindowRuntimeRecord& record) {
  record.native_window_error.reset();
  reset_accessibility_state_for_record(record);
  WindowRuntimeRecord* parent_record =
      find_window_runtime_record(record.parent_runtime_id);
  if (parent_record == nullptr) {
    record.native_window_error = Error{
        .code = ErrorCode::invalid_argument,
        .message = "Child window parent runtime record not found"};
    return;
  }
  if (!runtime_accept_popup_renderer(record, state_->core.application_, parent_record->renderer)) return;
  if (!parent_record->active || parent_record->window == nullptr) {
    return;
  }
  auto window_result = state_->core.application_.create_child_window(
      record.descriptor,
      *parent_record->window,
      [this, runtime_id = record.runtime_id](const PlatformEvent& event) {
        handle_native_additional_window_event(runtime_id, event);
      });
  if (!window_result) {
    record.native_window_error = window_result.error();
    return;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  const WindowState window_state = window->state();
  auto renderer_result = try_create_renderer(
      RenderSurfaceDescriptor{
          .native_surface = window->native_surface(),
          .framebuffer_size = window_state.framebuffer_size,
          .scale = window_state.scale,
          .transparent_background =
              runtime_wants_transparent_surface(record.descriptor)},
      "Renderer factory returned an empty child renderer");
  if (!renderer_result) {
    record.native_window_error = renderer_result.error();
    return;
  }
  if (!runtime_accept_popup_renderer(record, state_->core.application_, *renderer_result)) {
    state_->windows.window_lifetime_->release(*renderer_result); return;
  }

  record.window = window.get();
  record.renderer = *renderer_result;
  record.owns_renderer = state_->windows.window_lifetime_->owns(record.renderer);
  (void)runtime_apply_window_backdrop(*window, record.renderer, record.descriptor.backdrop);
  record.framebuffer_size = window_state.framebuffer_size;
  record.viewport_size = to_logical_pixels(
      window_state.framebuffer_size,
      window_state.scale);
  record.scale = window_state.scale;
  record.redraw_scheduled = false;
  record.input = {};
  reset_event_state_for_record(record);
  record.active = true;
  state_->windows.native_additional_windows_.push_back(std::move(window));
}

} // namespace cgpui
