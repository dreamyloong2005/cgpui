#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::activate_native_window_for_record(
    WindowRuntimeRecord& record) {
  record.native_window_error.reset();
  WindowRuntimeRecord* parent_record =
      find_window_runtime_record(record.parent_runtime_id);
  if (parent_record == nullptr) {
    record.native_window_error = Error{
        .code = ErrorCode::invalid_argument,
        .message = "Child window parent runtime record not found"};
    return;
  }
  if (!parent_record->active || parent_record->window == nullptr) {
    return;
  }
  auto window_result = application_.create_child_window(
      record.descriptor,
      *parent_record->window,
      [this, runtime_id = record.runtime_id](const PlatformEvent& event) {
        handle_native_additional_window_event(runtime_id, event);
      });
  if (!window_result) {
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
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
              record.descriptor.chrome.transparent_background},
      "Renderer factory returned an empty child renderer");
  if (!renderer_result) {
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
    record.native_window_error = renderer_result.error();
    return;
  }

  record.window = window.get();
  record.renderer = *renderer_result;
  record.framebuffer_size = window_state.framebuffer_size;
  record.viewport_size = to_logical_pixels(
      window_state.framebuffer_size,
      window_state.scale);
  record.scale = window_state.scale;
  record.redraw_scheduled = false;
  record.input = {};
  reset_event_state_for_record(record);
  record.active = true;
  native_additional_windows_.push_back(std::move(window));
}

} // namespace cgpui
