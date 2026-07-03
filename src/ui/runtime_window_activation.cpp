#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::activate_native_window_for_record(
    WindowRuntimeRecord& record) {
  record.native_window_error.reset();
  auto window_result = application_.create_window(
      record.descriptor,
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
  auto renderer_result = renderer_factory_(RenderSurfaceDescriptor{
      .native_surface = window->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer_result || *renderer_result == nullptr) {
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
    record.native_window_error =
        renderer_result
            ? Error{
                  .code = ErrorCode::renderer_initialization_failed,
                  .message =
                      "Renderer factory returned an empty child renderer"}
            : renderer_result.error();
    return;
  }

  record.window = window.get();
  record.renderer = *renderer_result;
  record.active = true;
  native_additional_windows_.push_back(std::move(window));
}

} // namespace cgpui
