#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
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
