#include "ui_internal.hpp"
#include "runtime_window_scale_diagnostics_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_resize(const WindowResized& event) {
  const Result<void> result = resize_surface(event.size, event.scale);
  record_platform_diagnostic(window_scale_diagnostic(event, result.has_value()));
}

void WindowRuntime::handle_redraw() {
  auto result = try_draw_frame();
  if (!result) {
    fail_and_quit(result.error());
  }
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  auto result = try_resize_surface(size, scale);
  if (!result) {
    fail_and_quit(result.error());
  }
  return result;
}

} // namespace cgpui
